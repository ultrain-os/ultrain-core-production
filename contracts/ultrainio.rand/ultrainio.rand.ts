import { Contract } from "ultrain-ts-lib/src/contract";
import { RNAME, Account } from "ultrain-ts-lib/src/account";
import { Action } from "ultrain-ts-lib/src/action";
import { Block } from "ultrain-ts-lib/src/block";
import { intToString } from "ultrain-ts-lib/src/utils";
import { Asset } from "ultrain-ts-lib/src/asset";
import { verify_with_pk, SHA256 } from "ultrain-ts-lib/src/crypto";
import {
  Voter,
  Waiter,
  RandRecord,
  VoteHistory,
  VOTER_TABLE,
  WAITER_TABLE,
  RAND_TABLE,
  VOTE_HST_TABLE,
  EPOCH,
  Random,
  RAND_KEY
} from "./lib/random.lib";

// parameters
let WAITER_DEPOSIT_AMOUNT: Asset = new Asset(10000000);

const WAITER_NUM: u64 = 100;
// Version format, main.major.patch
const VERSION: string = "0.0.3";
// Fix not clear voting history.

@database(Voter, VOTER_TABLE)
@database(Waiter, WAITER_TABLE)
@database(RandRecord, RAND_TABLE)
@database(VoteHistory, VOTE_HST_TABLE)
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
      this.voteDB.emplace(vote);
    }

    if (!this.waiterDB.exists(0)) {
      let waitorseq = new Waiter();
      this.waiterDB.emplace(waitorseq);
    }
    this.random.initVoteHstMinBckNum();
    // Initial the mininum block number and maxinum block num and add default random number.
    // When first call the contract, initialize the default the random by the block number
    if (!this.randDB.exists(0)) {
      let randBeginNum: u64 = 2;
      let rand = new RandRecord();
      let initBckNum = (Block.number < randBeginNum) ? randBeginNum : Block.number;

      rand.blockNum = 0; // The blocknum 0, saving the special data that the minimum block number of rand.
      rand.val = initBckNum; // The minimum rand block number, maybe not exist. The default value is one.
      this.randDB.emplace(rand);

      rand.blockNum = 1; // The blocknum 1, saving the lastest random number
      rand.val = initBckNum;
      this.randDB.emplace(rand);

      rand.blockNum = initBckNum; // The blocknum 2, saving the first random number.
      rand.val = this.random.hash(initBckNum);
      rand.code = 7; // 7 represent 111.
      this.randDB.emplace(rand);
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
    var instruct: string = "in0x1WaiterRegister";
    if (memo != instruct) return; // not transfer to join the candidate list
    ultrain_assert(!this.voteDB.exists(user), "cannot add existing candidate.");
        
    if (!value.eq(WAITER_DEPOSIT_AMOUNT)) {
      ultrain_assert(false, "deposit money is not accurate, require deposit 1000 UGAS");
    }

    var voter = new Voter();
    voter.name = user;
    voter.setWaiterVoter();
    let waitorseq = new Waiter();
    this.waiterDB.get(0, waitorseq);
    ultrain_assert(waitorseq.waiters.length <= <i32>WAITER_NUM, "The total of waiters should be less than " + intToString(WAITER_NUM));
    waitorseq.waiters.push(user);
    this.waiterDB.modify(waitorseq);

    voter.deposit = value.getAmount();
    voter.regisBckNum = this.curtBckNum();
    voter.unregisBckNum = 0;
    this.voteDB.emplace(voter);

    this.random.increaseDeposit(WAITER_DEPOSIT_AMOUNT.getAmount());
  }

  private curtBckNum(): u64 {
    return Block.number + 1;
  }

  /**
   * The user unregister to be voter.
   * If users want to redeem the deposit, users should call action "redeem".
   */
  @action
  unregister(): void {
    ultrain_assert(this.voteDB.exists(Action.sender), "cannot unregister non-existing voter.");
    var voter = new Voter();
    this.voteDB.get(Action.sender, voter);
    ultrain_assert(!voter.isUnregister(), "cannot repeat to unregsiter voter");
    if (voter.isUnregister()) {
      Asset.transfer(this.receiver, Action.sender, WAITER_DEPOSIT_AMOUNT, "return deposited money");
      this.random.decreaseDeposit(WAITER_DEPOSIT_AMOUNT.getAmount());
      let waiterseq = new Waiter();
      this.waiterDB.get(0, waiterseq);
      let waiters =  waiterseq.waiters.filter((value: account_name, index: i32, array: Array<account_name>): bool => value != Action.sender);
      waiterseq.waiters = waiters;
      this.waiterDB.modify(waiterseq);
    }
    voter.regisBckNum = 0;
    voter.unregisBckNum = this.curtBckNum();
    this.voteDB.modify(voter);
  }

  /**
   * The user redeem the deposit money.
   */
  @action
  redeem(): void {
    ultrain_assert(this.voteDB.exists(Action.sender), "cannot redeem non-existing voter.");
    var voter = new Voter();
    this.voteDB.get(Action.sender, voter);
    if (voter.redeemable()) {
      Asset.transfer(this.receiver, Action.sender, new Asset(voter.deposit), "return deposited money");
      this.voteDB.erase(Action.sender);
    } else{
      ultrain_assert(false, "cannot redeem deposit, you must wait for some blocks generated or unregister first.");
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
    // To check the voting whether expried.
    var curtBckNum = Block.number + 1;
    ultrain_assert(this.belongRandNum(blockNum) >= curtBckNum, "The current block number is " 
      + intToString(curtBckNum) + ". The vote seed block number: " + intToString(blockNum) + " has expired.");

    this.triggerRandGenerate();
    if (this.random.isMainVoter(sender, blockNum)) {
      this.mainVoterVote(sender, pk_proof, blockNum);
    } else {
      ultrain_assert(this.voteDB.exists(sender), "You " + RNAME(sender) + " should be a candidate first.");
      var voter = new Voter();
      this.voteDB.get(sender, voter);
      ultrain_assert(voter.votable(), "The voter " + RNAME(sender) + " status is not votable.");
      this.waiterVoterVote(sender, pk_proof, blockNum);
    }
  }

  private saveVoterVoteStatus(name: account_name, seedBckNum: u64, vrf: u64): void {
    var index = this.indexOf(seedBckNum);
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
        this.voteDB.modify(voteInfo);
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
    // Log.s("seedStr : ").s(seedStr).flush();
    // Log.s("pkstr: ").s(pkStr).flush();
    // Log.s("blockNum: ").i(blockNum).flush();

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
    if (this.random.isMainVoted(name, blockNum)){
      ultrain_assert(false, RNAME(name) + " had already submitted the vote, please wait for the next round.");
    }
    var vrf = this.getVrfVal(pk_proof, blockNum);
    var index = this.indexOf(blockNum);
    var randVoteInfo = new Voter();
    this.voteDB.get(RAND_KEY, randVoteInfo);
    var oldBckNum = randVoteInfo.belongBckNums[index];
    if (this.belongRandNum(blockNum) != oldBckNum) {
      // using the random number as default value
      randVoteInfo.voteVals[index] = this.getExistRand(blockNum);
      randVoteInfo.belongBckNums[index] = this.belongRandNum(blockNum);
    }
    randVoteInfo.voteVals[index] = randVoteInfo.voteVals[index] ^ vrf;
    this.voteDB.modify(randVoteInfo);
  }

  /**
   * The waiter voter vote.
   * The waiter voter should vote the next two block period after the random block number.
   * @param name the account name
   * @param pk_proof the privat proof
   * @param blockNum the block number
   */
  private waiterVoterVote(name: account_name, pk_proof: string, blockNum: u64): void {
    if (this.curtBckNum() - blockNum > 2) {
      ultrain_assert(false, "The waiter should vote the next two block period after the blockNum");
    }
    var vrf = this.getVrfVal(pk_proof, blockNum);
    this.saveVoterVoteStatus(name, blockNum, vrf);
  }

  public filterAction(originalReceiver: u64): boolean {
    return Contract.filterAcceptTransferTokenAction(this.receiver, originalReceiver, this.action);
  }

  @action("pureview")
  getMainVoteInfo(): RandRecord {
    if(this.random.isMainVoter(Action.sender, Block.number)){
      return this.random.query(Block.number);
    }
    var rand = new RandRecord();
    rand.code = -1;
    return rand;
  }

  @action("pureview")
  query(): RandRecord {
    return this.random.queryLatest();
  }

  @action("pureview")
  queryBck(bckNum: u64): RandRecord {
    return this.random.query(bckNum);
  }

  @action("pureview")
  version(): string {
    return VERSION;
  }
}