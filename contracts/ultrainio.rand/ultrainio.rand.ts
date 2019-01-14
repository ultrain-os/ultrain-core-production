import { Contract } from "ultrain-ts-lib/src/contract";
import { RNAME, NAME, Account } from "ultrain-ts-lib/src/account";
import { Action } from "ultrain-ts-lib/src/action";
import { Block } from "ultrain-ts-lib/src/block";
import { Return } from "ultrain-ts-lib/src/return";
import { intToString } from "ultrain-ts-lib/src/utils";
import { Asset } from "ultrain-ts-lib/src/asset";
import { verify_with_pk, SHA256 } from "ultrain-ts-lib/src/crypto";
import { Log } from "ultrain-ts-lib/src/log";

class Candidate implements Serializable {
    @primaryid
    name: account_name = 0;
    vrf_pk: string = "";
    age: u32 = 0;
}

class Vote implements Serializable {
    @primaryid
    name: account_name = 0;
    val: u64 = 0;
}

//table + scope for external query
const canditable = "candidate";
const candiscope = "s.candidate";

const votetable = "vote";
const votescope = "s.vote";

// parameters
let DEPOSIT_AMOUNT :Asset = new Asset(20000);
const EPOCH :u64 =  3; // peorids for rand generation
const BONUS :u64 = 100; //bonus for voters
const MIN_AGE :u64=2; //minimum age to be candidate

// number of tables used in this class, not scopes
// two tables for two different data types
@database(Vote, "vote")
@database(Candidate, "candidate")
class rand extends Contract {

    candidateDB: DBManager<Candidate>;
    votedDB: DBManager<Vote>;

    constructor(code: u64) { // run every time any function is called
        super(code);
        // cadidateDB for internal use, with external interface defined
        // data type contains primary_key
        this.candidateDB = new DBManager<Candidate>(NAME(canditable), this.receiver, NAME(candiscope));
        this.votedDB = new DBManager<Vote>(NAME(votetable), this.receiver, NAME(votescope));

        // create permanent variable if first time
        if (!this.votedDB.exists(NAME("blocknum"))) { //first time
            let c = new Vote();
            c.name = NAME("blocknum");
            c.val = Block.number;
            this.votedDB.emplace(Action.sender, c);
            c.name = NAME("seed");
            c.val = 2346;
            this.votedDB.emplace(Action.sender, c);
            c.name = NAME("rand");
            c.val = 0;
            this.votedDB.emplace(Action.sender, c);
        }
        //TODO: auto-elimination of late signers
    }

    //clear historical data
    @action
    clearDB(): void {
        ultrain_assert(Action.sender == this.receiver, "only contract owner can clear DBs.");
        this.candidateDB.dropAll();
        this.votedDB.dropAll();
    }

    // all candidate can register with deposit and pk
    @action
    transfer(from: account_name, to: account_name, value: Asset, memo: string): void {
        Log.s("addCandidate: ").s(RNAME(from)).flush();
        if (from == this.receiver) return; // yourself, omit it.
        if (memo != "as candidate") return; // not transfer to join the candidate list
        ultrain_assert(!this.candidateDB.exists(from), "cannot add existing candidate.");
        ultrain_assert(value.eq(DEPOSIT_AMOUNT), "deposit money is not accurate, require deposit: " + DEPOSIT_AMOUNT.toString());

        let c = new Candidate();
        c.name = from;
        c.age = Block.number;
        // c.vrf_pk = "";
        this.candidateDB.emplace(this.receiver, c);
    }

    //TODO: punish candidate

    //remove Candidate
    @action
    removeCandidate(): void {
        ultrain_assert(this.candidateDB.exists(Action.sender), "cannot remove non-existing candidate.");
        this.candidateDB.erase(Action.sender);
        if (this.votedDB.exists(Action.sender)) {
            this.votedDB.erase(Action.sender);
        }
        Asset.transfer(this.receiver, Action.sender, DEPOSIT_AMOUNT, "return deposited money"); //return deposit
    }

    //TODO: punish unvoted ones?
    @action
    vote(pk_proof: string): void {
        ultrain_assert(this.candidateDB.exists(Action.sender), "you should be a candidate firstly.");

        let old_blocknum = new Vote();
        this.votedDB.get(NAME("blocknum"), old_blocknum);
        let old_blocknum_val = old_blocknum.val;
        //trigger a new epoch
        if ((Block.number - old_blocknum_val) >= EPOCH) {
            let new_blockNum_val = old_blocknum_val;
            while (new_blockNum_val + EPOCH <= Block.number) { // catch up to the recent start block, in case no vote tx
                new_blockNum_val += EPOCH;
            }
            //seed = rand
            let old_rand = new Vote();
            this.votedDB.get(NAME("rand"), old_rand);
            let new_seed_val = old_rand.val;

            this.votedDB.dropAll();

            let new_blocknum = new Vote();
            new_blocknum.name = NAME("blocknum");
            new_blocknum.val = new_blockNum_val;
            this.votedDB.emplace(Action.sender, new_blocknum);

            let new_seed = new Vote();
            new_seed.name = NAME("seed");
            new_seed.val = new_seed_val;
            this.votedDB.emplace(Action.sender, new_seed);

            let new_rand = new Vote();
            new_rand.name = NAME("rand");
            new_rand.val = 0;
            this.votedDB.emplace(Action.sender, new_rand);
        }

        ultrain_assert(!this.votedDB.exists(Action.sender), "you has already voted."); // vote once only in each epoch

        //query and verify with pk
        let cdd = new Candidate();
        this.candidateDB.get(Action.sender, cdd);
        if (Block.number - cdd.age < EPOCH) {
            ultrain_assert(false, "need to wait for the next round");
        }

        //query compressed pk point with account name, then verify proof
        let ZERO = "0";
        let seed = new Vote();
        this.votedDB.get(NAME("seed"), seed);
        let m = intToString(seed.val);
        ZERO = ZERO.repeat(64 - m.length);
        m = m.concat(ZERO);

        let pkStr = Account.publicKeyOf(Action.sender, 'hex');
        Log.s("message1 : ").s(m).flush();
        ultrain_assert(verify_with_pk(pkStr, pk_proof, m), "please provide a valid VRF proof." + pkStr + " " + pk_proof + " " + m);

        // aggregate rand
        //use hash proof
        let sha256 = new SHA256();
        let hash = sha256.hash(pk_proof.substring(0, 66)); // proof = (r_33byte, c_16_byte, s_32byte)
        let vrf:u64 = <u64> parseInt(hash.substring(0, 14), 16); // convert hex_6digits string to u64

        let rand = new Vote();
        this.votedDB.get(NAME("rand"), rand);
        rand.val = rand.val ^ vrf;
        this.votedDB.modify(this.receiver, rand);

        Log.s("message2 : ").s(m).flush();
        let sender = new Vote();
        sender.name = Action.sender;
        sender.val = vrf;
        this.votedDB.emplace(Action.sender, sender);
        Asset.transfer(this.receiver, Action.sender, new Asset(BONUS), "bonus money"); //give bonus

        Return(" Rand: " + intToString(rand.val) + " Block.number: " + intToString(Block.number));
    }

    public filterAction(originalReceiver: u64): boolean {
        return Contract.filterAcceptTransferTokenAction(this.receiver, originalReceiver, this.action);
    }

    //for external users, cross-contract calling
    // TODO: require fees or deposits
    // TODO: provide customized rand, such as hash(rand || query_account_name)
    @action
    query(): void {
        Log.s(RNAME(Action.sender)).s(" query").flush();
        let blocknum = new Vote();
        this.votedDB.get(NAME("blocknum"), blocknum);
        //trigger a new epoch
        if ((Block.number - blocknum.val) >= EPOCH) {
            let rand = new Vote();
            this.votedDB.get(NAME("rand"), rand);
            Return(" Rand: " + intToString(rand.val) + " Block.number: " + intToString(Block.number));
        } else {
            let seed = new Vote();
            this.votedDB.get(NAME("seed"), seed);
            Return(" Rand: " + intToString(seed.val) + " Block.number: " + intToString(Block.number));
        }
        Log.s("end").flush();
    }
}