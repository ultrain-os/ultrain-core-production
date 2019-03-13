import { Contract } from "ultrain-ts-lib/src/contract";
import { RNAME, NAME, Account } from "ultrain-ts-lib/src/account";
import { Action } from "ultrain-ts-lib/src/action";
import { Block } from "ultrain-ts-lib/src/block";
import { Return } from "ultrain-ts-lib/src/return";
import { intToString } from "ultrain-ts-lib/src/utils";
import { Asset } from "ultrain-ts-lib/src/asset";
import { verify_with_pk, SHA256 } from "ultrain-ts-lib/src/crypto";
import { Log } from "ultrain-ts-lib/src/log";
import {
  Voter,
  Waiter,
  RandRecord,
  VOTER_TABLE,
  WAITER_TABLE,
  RAND_TABLE,
  EPOCH,
  Random,
  RAND_KEY
} from "./lib/random.lib";

let MAIN_COUNT_KEY = NAME("mainnum");

// parameters
let MAIN_DEPOSIT_AMOUNT: Asset = new Asset(20000);
let WAITER_DEPOSIT_AMOUNT: Asset = new Asset(2000);

const MAIN_BONUS: u64 = 100; //bonus for main voters
const WAITER_BONUS: u64 = 8; // bonus for waiter voters
const WAITER_NUM: u64 = 100;
const MAIN_NUM: u64 = 20;

@database(Voter, VOTER_TABLE)
@database(Waiter, WAITER_TABLE)
@database(RandRecord, RAND_TABLE)
class RandContract extends Contract {
  voteDB: DBManager<Voter>;
  waiterDB: DBManager<Waiter>;
  randDB: DBManager<RandRecord>;
  random: Random;

  constructor(code: u64) { // Running every time any function is called
    super(code);
    this.random = new Random();
    this.voteDB = this.random.voteDB;
    this.waiterDB = this.random.waiterDB;
    this.randDB = this.random.randDB;
    
    // Initial the parameters
    if (!this.voteDB.exists(RAND_KEY)) { //first time
      let vote = new Voter();
      vote.name = RAND_KEY;
      this.voteDB.emplace(Action.sender, vote);
    }

    if (!this.voteDB.exists(MAIN_COUNT_KEY)) {
      let vote = new Voter();
      vote.name = MAIN_COUNT_KEY;
      this.voteDB.emplace(Action.sender, vote);
    }

    if (!this.waiterDB.exists(0)) {
      let waitorseq = new Waiter();
      this.waiterDB.emplace(Action.sender, waitorseq);
    }

    // Initial the mininum block number and maxinum block num and add default random number.
    // When first call the contract, initialize the default the random by the block number
    if (!this.randDB.exists(0)) {
      let rand = new RandRecord();
      let initBckNum = (Block.number < 2) ? 2 : Block.number;
      rand.blockNum = 0; // The blocknum 0, saving the special data that the minimum block number of rand.
      rand.val = initBckNum; // The minimum rand block number, maybe not exist. The default value is one.
      this.randDB.emplace(Action.sender, rand);

      rand.blockNum = 1; // The blocknum 1, saving the lastest random number
      rand.val = initBckNum;
      this.randDB.emplace(Action.sender, rand);

      rand.blockNum = initBckNum; // The blocknum 2, saving the first random number.
      rand.val = this.hash(initBckNum);
      rand.code = 7;
      this.randDB.emplace(Action.sender, rand);
    }
  }

  /**
   * This method triggered by the ultraio.token `transfer` action
   * TODO Now here should add action anotation, enhance it.
   * @param from from account
   * @param to to account
   * @param value the asset that transfer
   * @param memo the commment
   */
  @action
  transfer(from: account_name, to: account_name, value: Asset, memo: string): void {
    Log.s("A candidate ").s(RNAME(from)).s(" register to: ").s(RNAME(to)).flush();
    this.register(from, value, memo);
  }

  /**
   * Register to be a candidate
   * @param user The user who register
   * @param value The deposit money
   * @param memo The memo
   */
  private register(user: account_name, value: Asset, memo: string): void {
    if (user == this.receiver) return; // yourself, omit it.
    if (memo != "as candidate") return; // not transfer to join the candidate list
    ultrain_assert(!this.voteDB.exists(user), "cannot add existing candidate.");

    // Log.s("User: ").s(RNAME(user)).s(" begin to register.").flush();

    var voter = new Voter();
    voter.name = user;
    if (value.eq(MAIN_DEPOSIT_AMOUNT)) {
      voter.setMainVoter();
      ultrain_assert(this.countMainVoter() <= MAIN_NUM, "The total of main should be less than " + intToString(MAIN_NUM));
      this.updateCountMainVoter(1);
    } else if (value.eq(WAITER_DEPOSIT_AMOUNT)) {
      voter.setWaiterVoter();
      let waitorseq = new Waiter();
      this.waiterDB.get(0, waitorseq);
      ultrain_assert(waitorseq.waiters.length <= <i32>WAITER_NUM, "The total of waiters should be less than " + intToString(WAITER_NUM));
      waitorseq.waiters.push(user);
      this.waiterDB.modify(user, waitorseq);
    } else {
      Log.s("User: ").s(RNAME(user)).s(" deposited money accurate.").flush();
      ultrain_assert(false, "deposit money is not accurate, require deposit 2.0000 UGAS or 0.2000 UGAS");
    }

    voter.deposit = value.getAmount();
    voter.regisBckNum = this.curtBckNum();
    voter.unregisBckNum = 0;
    this.voteDB.emplace(this.receiver, voter);
    // Log.s("User: ").s(RNAME(user)).s(" register successfully.").flush();
  }

  private curtBckNum(): u64 {
    return Block.number + 1;
  }

  private countMainVoter(): u64 {
    var voter = new Voter();
    this.voteDB.get(MAIN_COUNT_KEY, voter);
    return voter.voteVals[0];
  }

  private updateCountMainVoter(changeQty: u64): void {
    var voter = new Voter();
    this.voteDB.get(MAIN_COUNT_KEY, voter);
    voter.voteVals[0] += changeQty;
    this.voteDB.modify(Action.sender, voter);
  }

  /**
   * The user unregister to be voter.
   * If users want to redeem the deposit, users should call action "redeem".
   */
  @action
  unregister(): void {
    ultrain_assert(this.voteDB.exists(Action.sender), "cannot remove non-existing candidate.");
    var voter = new Voter();
    this.voteDB.get(Action.sender, voter);
    if (voter.isMainVoter()) {
      Asset.transfer(this.receiver, Action.sender, MAIN_DEPOSIT_AMOUNT, "return deposited money");
      this.updateCountMainVoter(-1);
    } else {
      Asset.transfer(this.receiver, Action.sender, WAITER_DEPOSIT_AMOUNT, "return deposited money");
      let waiterseq = new Waiter();
      this.waiterDB.get(0, waiterseq);
      let waiters =  waiterseq.waiters.filter((value: i32, index: i32, array: Array<account_name>): bool => value != Action.sender);
      waiterseq.waiters = waiters;
      this.waiterDB.modify(Action.sender, waiterseq);
    }
    voter.regisBckNum = 0;
    voter.unregisBckNum = this.curtBckNum();
    this.voteDB.modify(Action.sender, voter);
  }

  /**
   * The user redeem the deposit money.
   */
  @action
  redeem(): void {
    ultrain_assert(this.voteDB.exists(Action.sender), "cannot remove non-existing candidate.");
    var voter = new Voter();
    this.voteDB.get(Action.sender, voter);
    if (voter.redeemable()) {
      Asset.transfer(this.receiver, Action.sender, new Asset(voter.deposit), "return deposited money");
      this.voteDB.erase(Action.sender);
    } else{
      ultrain_assert(false, "cannot redeem deposit, you must wait for 10 block generated.");
    }
  }

  private getExistRand(blockNum: u64): u64 {
    var rand = new RandRecord();
    ultrain_assert(this.randDB.exists(blockNum), "The random of block height has not generated.");
    this.randDB.get(blockNum, rand);
    return rand.val;
  }

  /**
   * Return the random number which the voting by the seed block number
   * @param seedBckNum The block number which the seed used.
   */
  private belongRandNum(seedBckNum: u64): u64 {
    return seedBckNum + EPOCH;
  }

  /**
   * 
   * @param blockNum The block number
   */
  public triggerRandGenerate(): void {
    var blockNum = Block.number;
    if (!this.randDB.exists(blockNum)) {
      this.random.generateRand(blockNum, true);
    }
  }

  /**
   * Voting the proof
   * 1. Check whether the seed and block number are legal.
   * 2. Trigger the random number generated
   * 3. Saving the vote status
   * @param pk_proof The pk proof
   * @param blockNum The block number which the seed generated.
   */
  @action
  vote(pk_proof: string, blockNum: u64): void {
    var sender = Action.sender;
    ultrain_assert(this.voteDB.exists(sender), "You should be a candidate first.");
    var index = this.indexOf(blockNum);
    // Log.s("User: ").s(RNAME(sender)).s(" begins to vote. The index: ").i(index).s("bck num:").i(blockNum).flush();

    // To check the voting whether expried.
    var curtBckNum = Block.number + 1;
    ultrain_assert(this.belongRandNum(blockNum) >= curtBckNum, "The current block number is " 
      + intToString(curtBckNum) + ". The vote seed block number: " + intToString(blockNum) + " has expired.");

    this.triggerRandGenerate();
    var voter = new Voter();
    this.voteDB.get(sender, voter);
    if (voter.isMainVoter()) {
      this.mainVoterVote(sender, pk_proof, blockNum);
    } else if (voter.isWaiterVoter()) {
      this.waiterVoterVote(sender, pk_proof, blockNum);
    }
  }

  private saveVoterVoteStatus(name: account_name, seedBckNum: u64, vrf: u64): void {
    var index = this.indexOf(seedBckNum);
    ultrain_assert(0 <= index && index<= 2, "The index should be between 0 and 2");
    var curtBlockNum = Block.number + 1;
    var voteInfo = new Voter();
    voteInfo.name = name;
    if (this.voteDB.exists(name)) {
      this.voteDB.get(name, voteInfo);
      if (this.belongRandNum(seedBckNum) == voteInfo.belongBckNums[index]) {
        ultrain_assert(false, "You had already submitted the vote, you should wait for the next round.");
      }
      if (this.belongRandNum(seedBckNum) >= curtBlockNum || voteInfo.belongBckNums[index] == 0) {
        voteInfo.belongBckNums[index] = this.belongRandNum(seedBckNum);
        voteInfo.voteVals[index] = vrf;
        this.voteDB.modify(name, voteInfo);
      } else {
        ultrain_assert(false, "Your vote was expired.");
      }
    } 
  }

  private getVrfVal(pk_proof: string, blockNum: u64): u64 {
    var seed = this.getExistRand(blockNum);
    var seedStr = intToString(seed);
    seedStr = seedStr.concat("0".repeat(64 - seedStr.length));

    var pkStr = Account.publicKeyOf(Action.sender, 'hex');
    // Log.s("m : ").s(seedStr).flush();
    // Log.s("pkstr: ").s(pkStr).flush();
    ultrain_assert(verify_with_pk(pkStr, pk_proof, seedStr), "please provide a valid VRF proof." + pkStr + " " + pk_proof + " " + seedStr);
    var sha256 = new SHA256();
    var hash = sha256.hash(pk_proof.substring(0, 66)); // proof = (r_33byte, c_16_byte, s_32byte)
    var vrf: u64 = <u64>parseInt(hash.substring(0, 14), 16); // convert hex_6digits string to u64
    return vrf;
  }

  private indexOf(bckNum: u64): i32 {
    return <i32>(bckNum % EPOCH);
  }

  private mainVoterVote(name: account_name, pk_proof: string, blockNum: u64): void {
    var vrf = this.getVrfVal(pk_proof, blockNum);
    this.saveVoterVoteStatus(name, blockNum, vrf);
    
    var index = this.indexOf(blockNum);
    // To update the main random number
    var randVoteInfo = new Voter();
    this.voteDB.get(RAND_KEY, randVoteInfo);
    var oldBckNum = randVoteInfo.belongBckNums[index];
    if (this.belongRandNum(blockNum) != oldBckNum) {
      randVoteInfo.voteVals[index] = this.getExistRand(blockNum);
      randVoteInfo.belongBckNums[index] = this.belongRandNum(blockNum);
    }
    randVoteInfo.voteVals[index] = randVoteInfo.voteVals[index] ^ vrf;
    this.voteDB.modify(name, randVoteInfo);

    // Issue the bonus for the voter
    Asset.transfer(this.receiver, Action.sender, new Asset(MAIN_BONUS), "bonus money"); //give bonus
  }

  /**
   * The waiter voter vote.
   * The waiter voter should vote the next two block period after the random block number.
   * @param name 
   * @param pk_proof 
   * @param blockNum 
   */
  private waiterVoterVote(name: account_name, pk_proof: string, blockNum: u64): void {
    if (this.curtBckNum() - blockNum > 2) {
      ultrain_assert(false, "The waiter should vote the next two block period after the blockNum");
    }
    var vrf = this.getVrfVal(pk_proof, blockNum);
    this.saveVoterVoteStatus(name, blockNum, vrf);
    Asset.transfer(this.receiver, Action.sender, new Asset(WAITER_BONUS), "bonus money"); //give bonus
  }

  public filterAction(originalReceiver: u64): boolean {
    return Contract.filterAcceptTransferTokenAction(this.receiver, originalReceiver, this.action);
  }

  @action("pureview")
  query(): void {
    var rand = this.random.queryLatest();
    Return(rand.toString());
  }

  @action("pureview")
  queryBck(bckNum: u64): void {
    var rand = this.random.query(bckNum);
    Return(rand.toString());
  }
  
  private hash(seed: u64): u64 {
    var sha256 = new SHA256();
    var hash = sha256.hash(intToString(seed));
    return <u64>parseInt(hash.substring(0, 14), 16);
  }

}