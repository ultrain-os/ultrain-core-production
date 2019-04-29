import { NAME } from "ultrain-ts-lib/src/account";
import { intToString } from "ultrain-ts-lib/src/utils";
import { SHA256 } from "ultrain-ts-lib/src/crypto";
import { Block } from "ultrain-ts-lib/src/block";
import { Asset } from "ultrain-ts-lib/src/asset";
import { queryBalance } from "../../node_modules/ultrain-ts-lib/lib/balance";

//table + scope for external query
export const VOTER_TABLE = "voter";
export const WAITER_TABLE = "waiter";
export const RAND_TABLE = "rand";
export const VOTE_HST_TABLE ="history";
export const CONFIG_TABLE = "config";

export const CONT_NAME = "utrio.rand";

export const EPOCH: u64 = 3; // period for rand generation
export let RAND_KEY = NAME("rand");
export let MAIN_COUNT_KEY = NAME("mainnum");
export let MAIN_VOTES_NUM_KEY = NAME("votesnum");

// committee
const ADMIN_NAME = "ultrainio";
const COMMITTEE_TABLE = "producers";

const CACHED_RAND_COUNT: u64 = 999;
const CACHED_HST_COUNT: u64 = 10;
const WAITER_BONUS: u64 = 1; // bonus for waiter voters

const DEPOSIT_KEY:u64 = 0;

let sha256 = new SHA256();

export class Config implements Serializable {
  @primaryid
  id: u64;
  val: u64;
}

export class VoteHistory implements Serializable {
  @primaryid
  bcknum: u64;
  votedUser: Array<account_name>;

  constructor() {
    this.votedUser = new Array<account_name>();
  }
}

export class Voter implements Serializable {
  @primaryid
  name: account_name = 0 // The account name
  belongBckNums: Array<u64>; // The vote belong the random block number
  voteVals: Array<u64>; // The voter voted value
  deposit: u64; // The deposit money, the accurate value is new Asset(deposit)
  regisBckNum: u64; // The block number of the voter registered, used to freeze the deposit for some time.
  unregisBckNum: u64; // The block number of the voter unregistered, used to freeze the deposit for some time.
  role: i32 = 0; // The user role, 1 represent main voter, 2 represent waiter voter
  bonus: i32 = 0; // 

  constructor() {
    this.belongBckNums = new Array<u64>(<i32>EPOCH);
    this.voteVals = new Array<u64>(<i32>EPOCH);
  }

  //In case of user calculated the random privately, 
  //so user can't vote immediately after registerd.
  votable(): boolean {
    return (Block.number > this.regisBckNum + 3 && this.regisBckNum != 0);
  }

  isUnregister(): boolean {
    return this.unregisBckNum != 0;
  }

  //In case of user redeem the deposit money immediately,
  //because of user making evil can't be found immediately.
  redeemable(): bool {
    return (Block.number > this.unregisBckNum + 10) && this.unregisBckNum != 0;
  }

  setWaiterVoter(): void {
    this.role = 2;
  }

  isWaiterVoter(): bool {
    return (this.role == 2);
  }
}

export class Waiter implements Serializable {
  waiters: Array<account_name> = new Array<account_name>();
  primaryKey(): id_type {
    return 0;
  }
}

/**
 *  The producer class
 */
export class Producer implements Serializable {
  owner: account_name;
  primaryKey(): account_name {
    return this.owner
  }
}

export class RandRecord implements Serializable, Returnable {
  blockNum: u64 = 0; // The block number
  val: u64 = 0; // The random value
  code: i32 = 0; // The random record status, the calculate way refer method calcCode

  primaryKey(): id_type {
    return this.blockNum;
  }
  
  setFields(blockNum: u64, val: u64, code: i32): void {
    this.blockNum = blockNum;
    this.val = val;
    this.code = code;
  }

  /**
   * The algorithm to calculate the code
   * The code value explain:
   *  code 0: absolutely random that user voting the data
   *  code 1: the random number calculated by the previos data
   * 
   * The code value format 0b111.
   * @param preRandCode The previous rand code, 7 - the previous rand is absolutely calculated
   * @param mainVoteCode The main voter voting code, 0 - using the main voter voting
   * @param waiterVoteCode The waiter voter voting code, 0 - using the waiter voting value
   */
  static calcCode(preRandCode: i32, mainVoteCode: i32, waiterVoteCode: i32): i32 {
    var code = 0;
    if (preRandCode == 7) code += 4;
    if (mainVoteCode > 0) code += 2;
    if (waiterVoteCode > 0) code += 1;
    return code;
  }

  // TODO format this.val to be hex
  toString(): string {
    return intToString(this.blockNum) + "," + intToString(this.val) + "," + this.code.toString();
  }
}

export class Random {

  voteDB: DBManager<Voter>;
  waiterDB: DBManager<Waiter>;
  randDB: DBManager<RandRecord>;
  committeeDB: DBManager<Producer>;
  configDB: DBManager<Config>;
  voteHstDB: DBManager<VoteHistory>;

  constructor() {
    this.voteDB = DBManager.newInstance<Voter>(NAME(VOTER_TABLE), NAME(CONT_NAME), NAME(VOTER_TABLE));
    this.waiterDB = DBManager.newInstance<Waiter>(NAME(WAITER_TABLE), NAME(CONT_NAME), NAME(WAITER_TABLE));
    this.randDB = DBManager.newInstance<RandRecord>(NAME(RAND_TABLE), NAME(CONT_NAME), NAME(RAND_TABLE));
    this.voteHstDB = DBManager.newInstance<VoteHistory>(NAME(VOTE_HST_TABLE), NAME(CONT_NAME), NAME(VOTE_HST_TABLE));
    this.configDB = DBManager.newInstance<Config>(NAME(CONFIG_TABLE), NAME(CONT_NAME), NAME(CONFIG_TABLE));
    
    this.committeeDB = DBManager.newInstance<Producer>(NAME(COMMITTEE_TABLE), NAME(ADMIN_NAME), NAME(ADMIN_NAME));
  }

  public queryLatest(): RandRecord {
    return this.generateRand(Block.number, false);
  }

  public query(bckNum: u64): RandRecord {
    var minBckNum = this.getMinRandBckNum();
    var maxBckNum = Block.number;
    ultrain_assert(minBckNum <= bckNum && bckNum <= maxBckNum, "Currently the block number of the rand should be between " + intToString(minBckNum) + " and " + intToString(maxBckNum));
    return this.generateRand(bckNum, false);
  } 

  @inline
  private indexOf(bckNum: u64): i32 {
    return <i32>(bckNum % EPOCH);
  }

  public hash(seed: u64): u64 {
    var hash = sha256.hash(intToString(seed));
    return <u64>parseInt(hash.substring(0, 14), 16);
  }

  /**
   * Get the main voter voted value.
   * @param headBckNum the header block number
   * @param preRand the previous block random number
   */
  private getMainVoteVal(headBckNum: u64, preRand: u64): RandRecord {
    var randInfo = new Voter();
    this.voteDB.get(RAND_KEY, randInfo);
    var index = this.indexOf(headBckNum);
    var rand = new RandRecord();
    if (randInfo.belongBckNums[index] == headBckNum) {
      rand.val = randInfo.voteVals[index];
      rand.code = 0;
    } else {
      rand.val = this.hash(preRand);
      rand.code = 1;
    }
    rand.blockNum = headBckNum;
    return rand;
  }

  private getConfig(id: u64): u64 {
    var config = new Config();
    if (this.configDB.exists(id)) {
      this.configDB.get(id, config);
      return config.val;
    }
    return 0;
  }

  private setConfigValue(id: u64, val: u64): void {
    var config = new Config();
    config.val = val;
    config.id = id;
    if (this.configDB.exists(id)) {
      this.configDB.modify(config);
    } else {
      this.configDB.emplace(config);
    }
  }

  public increaseDeposit(val: u64): void {
    var original = this.getConfig(DEPOSIT_KEY);
    this.setConfigValue(DEPOSIT_KEY, original + val);
  }

  public decreaseDeposit(val: u64): void {
    var original = this.getConfig(DEPOSIT_KEY);
    this.setConfigValue(DEPOSIT_KEY, original - val);
  }

  private canSendBonus(): bool {
    var balance = queryBalance(NAME(CONT_NAME));
    var deposit = this.getConfig(DEPOSIT_KEY);
    return balance.getAmount() >= (deposit + WAITER_BONUS);
  }

  /**
   * The waiter vote value generated way:
   * If using the waiter vote, the vote value is waiter.voteval
   * if not using the waiter vote, the vote value is hash(randNum)
   */
  private getWaiterVoteVal(headBckNum: u64, randNum: u64, isUpt: boolean): RandRecord {
    
    var waiterseq = new Waiter();
    this.waiterDB.get(0, waiterseq);
    let waiters = waiterseq.waiters;
    var waiterLen = waiters.length;

    // Maybe a hidden problem, the recursive randTemp value.
    var recursive: i32 = 0;
    var recursiveDepth: i32 = 10; // *tips*: this recursive depth maybe not the best value

    var rand = new RandRecord();
    rand.blockNum = headBckNum;
    rand.val = this.hash(randNum);
    rand.code = 1;

    if (waiterLen == 0) return rand;

    var waiterVote = new Voter();
    var index = this.indexOf(headBckNum);
    var oldRandNum = randNum;
    do {
      let slotIndex = <i32>(oldRandNum % waiterLen);
      let user = waiterseq.waiters[slotIndex];
      this.voteDB.get(user, waiterVote);
      let seedBckNum = waiterVote.belongBckNums[index];
      if (headBckNum != seedBckNum) { // This round, the user don't submit the vote.
        oldRandNum = this.hash(oldRandNum);
      } else {
        // TODO Using the last random period waiter vote value maybe better
        rand.val = waiterVote.voteVals[index];
        rand.code = 0;
        if (isUpt && this.canSendBonus()){
          Asset.transfer(NAME(CONT_NAME), user, new Asset(WAITER_BONUS), "bonus money"); //give bonus
        }
        break;
      }
      recursive++;
    } while (recursive < recursiveDepth);
    return rand;
  }

  getWaiterVote(headBckNum: u64, randNum: u64): Voter {
    var waiterseq = new Waiter();
    this.waiterDB.get(0, waiterseq);
    let waiters = waiterseq.waiters;
    var waiterLen = waiters.length;

    // Maybe a hidden problem, the recursive randTemp value.
    var recursive: i32 = 0;
    var recursiveDepth: i32 = 10; // *tips*: this recursive depth maybe not the best value

    var tempWaiter = new Voter();
    if (waiterLen == 0) return tempWaiter;

    var waiterVote = new Voter();
    var index = this.indexOf(headBckNum);
    var oldRandNum = randNum;
    do {
      let slotIndex = <i32>(oldRandNum % waiterLen);
      let user = waiterseq.waiters[slotIndex];
      this.voteDB.get(user, tempWaiter);
      let seedBckNum = tempWaiter.belongBckNums[index];
      if (headBckNum != seedBckNum) { // This round, the user don't submit the vote.
        oldRandNum = this.hash(oldRandNum);
      } else {
        // TODO Using the last random period waiter vote value maybe better 
        waiterVote = tempWaiter;
        break;
      }
      recursive++;
    } while (recursive < recursiveDepth);
    return waiterVote;
  }

  isMainVoted(sender: account_name, bckNum: u64): bool {
    var voteHst = new VoteHistory();
    voteHst.bcknum = bckNum;
    if (!this.voteHstDB.exists(bckNum)) {
      voteHst.votedUser.push(sender);
      this.voteHstDB.emplace(voteHst);
      return false;
    } else {
      this.voteHstDB.get(bckNum, voteHst);
      let voteUsers = voteHst.votedUser;
      for (let i = 0; i < voteUsers.length; i++) {
        if (voteUsers[i] == sender) {
          return true;
        }
      }
      voteUsers.push(sender);
      voteHst.votedUser = voteUsers;
      this.voteHstDB.modify(voteHst);
      return false;
    }
  }

  isMainVoter(sender: account_name, bckNum: u64): bool {
    var isCommittee = this.committeeDB.exists(sender);
    if (!isCommittee) {
      return false;
    }
    const votingNums: u64 = 4;
    var count = this.committeeDB.cursor().count;
    var hashSender = this.hash(changetype<u64>(sender));
    var modVal = (hashSender + bckNum) % count;
    return modVal < votingNums;
  }

  initVoteHstMinBckNum(): void {
    if (!this.voteHstDB.exists(0)) {
      var voteHst = new VoteHistory();
      voteHst.bcknum = 0;
      voteHst.votedUser.push(Block.number);
      this.voteHstDB.emplace(voteHst);
    }
  }

  private getVoteHstMinBckNum() : u64 {
    var voteHst = new VoteHistory();
    this.voteHstDB.get(0, voteHst);
    return changetype<u64>(voteHst.votedUser[0]);
  }

  private clearMainVoteHst(blockNum: u64): void {
    var oldMinNum = this.getVoteHstMinBckNum();
    var voteHst = new VoteHistory();
    // Here should use the expression, (blockNum >= CACHED_RAND_COUNT + minBckNum)
    // cannot use (blockNum - minBckNum > CACHED_RAND_COUNT)
    // because if (blockNum < minBckNum), the value of (blockNum - minBckNum) maybe overflow.
    if (blockNum > CACHED_HST_COUNT + oldMinNum) {
      const DELETED_MOST: u64 = 1000;
      let range = blockNum - CACHED_HST_COUNT - oldMinNum;
      let delCount = range >= DELETED_MOST ? DELETED_MOST : range;
      for (let i = 0; i <= <i32>delCount; i ++) {
        if (this.voteHstDB.exists(oldMinNum + i)) {
          this.voteHstDB.erase(oldMinNum + i);
        }
      }
      voteHst.bcknum = 0;
      voteHst.votedUser.push(oldMinNum + delCount + 1);
      this.voteHstDB.modify(voteHst);
    }
  }

  /**
   * The algorithm of the random expression:
   * rand = preRand ^ mainVoterValue ^ waiterVoterValue
   * 
   * The algorithm of the mainVoterValue expression:
   * mainVoterValue = (m1VoterValue ^ m2VoterValue ^ ... ^ mVoterValue)
   * If no main voter voted,
   * mainVoterValue = hash(preRand)
   *  
   * The algorithm of the waiterVoterValue:
   * 1. Sorting the waiter sequence by the register sequence 
   * 2. Getting the random value,
   *    random = mainVoterValue, the first time
   *    random = hash(random), not the first time
   * 3. Get the index, the expression of index: random % waiterCount
   * 4. So the waiterVoterValue is the index waiter voter value.
   * 5. If the index waiter don't vote, redo the sequence 2 ~ 4 n times.
   * 6. If there is still no voter value, using the hash(mainVoterValue)
   */
  public generateRand(headBckNum: u64, isUpt: boolean): RandRecord {
    // To check the rand whether existing, if existing return.
    var rand = new RandRecord();
    if (this.randDB.exists(headBckNum)) {
      this.randDB.get(headBckNum, rand);
      return rand;
    }
    var lastRand = this.getLastRand();
    var lastBckNum = lastRand.blockNum;

    const LAST_ROUND: u64 = 13;
    if (headBckNum > lastBckNum + LAST_ROUND) {
      let preRand = lastRand.val;
      let mainVoterValue = this.hash(headBckNum);
      let waitorVoterValue = this.hash(mainVoterValue);
      let randVal = preRand ^ mainVoterValue ^ waitorVoterValue;
      lastRand.setFields(headBckNum, randVal, 7);
      if (isUpt) {
        this.saveAndClearPartRands(headBckNum, randVal, 7);
      }
      return lastRand;
    } 

    var waiterVote = new RandRecord();
    for (let i = lastBckNum + 1; i <= headBckNum; i++) {
      let mainVote = this.getMainVoteVal(i, lastRand.val);
      if ( i >= lastBckNum + EPOCH) { // In this case, there is no waiter voting
        waiterVote.setFields(i, this.hash(mainVote.val), 1); 
      } else {
        waiterVote = this.getWaiterVoteVal(i, mainVote.val, isUpt);
      }
      let code = RandRecord.calcCode(lastRand.code, mainVote.code, waiterVote.code); 
      let randVal = lastRand.val ^ mainVote.val ^ waiterVote.val;
      rand.setFields(i, randVal, code);
      if (isUpt) {
        this.saveAndClearPartRands(i, randVal, code);
      }
      lastRand = rand;
    }
    return lastRand;
  }


  /**
   * Saving the random number and clear the cached random number partly.
   * @param bckNum The block number
   * @param randNum The random number
   * @param code The code
   */
  private saveAndClearPartRands(bckNum: u64, randNum: u64, code: i32): void {
    var rand = new RandRecord();
    rand.setFields(bckNum, randNum, code);
    if (!this.randDB.exists(bckNum)) {
      this.randDB.emplace(rand); // To do the payer user should to be discuss
    }
    // Saving the maxinum blocknum
    this.saveRandMaxBckNum(bckNum);
    this.clearPartRands(bckNum);
    this.clearMainVoteHst(bckNum);
  }

  private getMinRandBckNum(): u64 {
    var rand = new RandRecord();
    this.randDB.get(0, rand);
    return rand.val;
  }

  /**
   * Get the last generated random number that saved in database
   */
  private getLastRand(): RandRecord {
    var rand = new RandRecord();
    this.randDB.get(1, rand);

    var lastRand = new RandRecord();
    this.randDB.get(rand.val, lastRand);
    return lastRand;
  }

  /**
   * Savint the max block number
   * @param bckNum The latest block number
   */
  private saveRandMaxBckNum(bckNum: u64): u64 {
    var rand = new RandRecord();
    this.randDB.get(1, rand);
    if (bckNum > rand.val) {
      rand.val = bckNum;
      this.randDB.modify(rand);
    }
    return rand.val;
  }

  private clearPartRands(blockNum: u64): void {
    var minBckNum  = this.getMinRandBckNum();
    var rand = new RandRecord();
    // Here should use the expression, (blockNum >= CACHED_RAND_COUNT + minBckNum)
    // cannot use (blockNum - minBckNum > CACHED_RAND_COUNT)
    // because if (blockNum < minBckNum), the value of (blockNum - minBckNum) maybe overflow.
    if (blockNum >= CACHED_RAND_COUNT + minBckNum) {
      if (this.randDB.exists(minBckNum)) {
        this.randDB.erase(minBckNum);
      }
      rand.val = minBckNum + 1;
      if (blockNum > CACHED_RAND_COUNT + minBckNum) {
        if (this.randDB.exists(minBckNum + 1)) {
          this.randDB.erase(minBckNum + 1);
        }
        rand.val = minBckNum + 2;
      }
      this.randDB.modify(rand);
    }
  }
}
