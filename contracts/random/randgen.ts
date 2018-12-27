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
    vrf_pk: string ="";
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
const EPOCH :u64 =  3; // peorids for rand generation
const DEPOSIT_AMOUNT :u64 = 20000;
const BONUS :u64 = 100; //bonus for voters
const MIN_AGE :u64=2; //minimum age to be candidate

// number of tables used in this class, not scopes
// two tables for two different data types
@database(Vote, "vote")
@database(Candidate, "candidate")
class randgen extends Contract {

    candidateDB: DBManager<Candidate>;
    votedDB: DBManager<Vote>;

    constructor(code: u64) { // run every time any function is called
        super(code);
        // cadidateDB for internal use, with external interface defined
        // data type contains primary_key
        this.candidateDB = new DBManager<Candidate>(NAME(canditable), this.receiver, NAME(candiscope));
        // this.candidateDB2 = new DBManager<Candidate>(NAME(canditable), this.receiver, NAME(candiscope2)); //define same DB, same table with another scope
        this.votedDB = new DBManager<Vote>(NAME(votetable), this.receiver, NAME(votescope));  

        // create permanent variable if first time
        if (!this.votedDB.exists(NAME("start_blocknum"))) { //first time
                let c = new Vote();
                c.name = NAME("start_blocknum");
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
    addCandidate(): void {
        Log.s("start to remove: ").flush();
        //ultrain_assert(!this.candidateDB.exists(Action.sender), "cannot add existing candidate.");

        let c = new Candidate();
        Log.s("start to remove: ").flush();
        c.name = Action.sender;
        c.age = Block.number;
        // c.vrf_pk = "";
        Log.s("start to remove: ").flush();
        this.candidateDB.emplace(this.receiver, c);
        Log.s("start to remove: ").flush();
        Asset.transfer(Action.sender, this.receiver, new Asset(DEPOSIT_AMOUNT), "deposit money"); //return deposit
        Log.s("start to remove: ").flush();
    }

    //TODO: punish candidate

    //remove Candidate
    @action
    removeCandidate(): void {
        ultrain_assert(this.candidateDB.exists(Action.sender), "cannot remove non-existing candidate.");
        this.candidateDB.erase(Action.sender);
        if (this.votedDB.exists(Action.sender))
            this.votedDB.erase(Action.sender);
        Asset.transfer(this.receiver, Action.sender, new Asset(DEPOSIT_AMOUNT), "return deposited money"); //return deposit
    }

    //TODO: punish unvoted ones?
    @action
    vote(pk_proof: string): void {
        ultrain_assert(this.candidateDB.exists(Action.sender), "you should be a candidate firstly.");

        let c = new Vote();
        this.votedDB.get(NAME("start_blocknum"), c);
        //trigger a new epoch
        if ((Block.number - c.val) >=EPOCH) {
            let new_start_blockNum=c.val;
            while (new_start_blockNum < (Block.number-EPOCH)) // catch up to the recent start block, in case no vote tx
                 new_start_blockNum = new_start_blockNum + EPOCH;

            //seed = rand
            this.votedDB.get(NAME("rand"), c);
            let new_seed = c.val;

            //clear votedDB
            this.votedDB.dropAll(); //return -1 if not existing
            // this.voters = []; // clear voted array

            //refill data
            c.name = NAME("start_blocknum");
            c.val = new_start_blockNum;
            this.votedDB.emplace(Action.sender, c);

            c.name = NAME("seed");
            c.val = new_seed;
            this.votedDB.emplace(Action.sender, c);

            c.name = NAME("rand");
            c.val = 0;
            this.votedDB.emplace(Action.sender, c);
        }

        ultrain_assert(!this.votedDB.exists(Action.sender), "you has already voted.");

        //query and verify with pk
        let cdd = new Candidate();

        // need to be old enough
        // if (Block.number - c.age < EPOCH)
        //  ultrain_assert(false, "this account need to wait for the next round.");

        this.candidateDB.get(Action.sender, cdd);

        //query compressed pk point with account name, then verify proof
	let ZERO = "0";
        this.votedDB.get(NAME("seed"), c);
        let cStr = intToString(c.val);
        ZERO=ZERO.repeat(64-cStr.length);

        let pkStr=Account.publicKeyOf(Action.sender, 'hex');
        ultrain_assert(verify_with_pk(pkStr, pk_proof, cStr.concat(ZERO)), "please provide a valid VRF proof."+ pkStr + " " +pk_proof +" " +cStr.concat(ZERO));

        // aggregate rand
        //use hash proof
        let sha256 = new SHA256();
        let hash = sha256.hash(pk_proof.substring(0,66)); // proof = (r_33byte, c_16_byte, s_32byte)
        let vrf:u64 = <u64> parseInt(hash.substring(0,14), 16); // convert hex_6digits string to u64

        this.votedDB.get(NAME("rand"), c);
        c.val = c.val ^ vrf; 
        let rand = c.val;
        this.votedDB.modify(this.receiver, c);

        c.name = Action.sender;
        c.val = vrf;
        this.votedDB.emplace(Action.sender, c);
        Asset.transfer(this.receiver, Action.sender, new Asset(BONUS), "bonus money"); //give bonus

        Return(" Rand: "+intToString(rand)+" Block.number: "+intToString(Block.number));
    }

    //for external users, cross-contract calling
    // TODO: require fees or deposits
    // TODO: provide customized rand, such as hash(rand || query_account_name)
    @action
    query(): void {
        let c = new Vote();
        this.votedDB.get(NAME("start_blocknum"), c);
        //trigger a new epoch
        if ((Block.number - c.val) >=EPOCH) 
            this.votedDB.get(NAME("rand"), c);
        else
            this.votedDB.get(NAME("seed"), c);
	Return(" Rand: "+intToString(c.val)+" Block.number: "+intToString(Block.number));
    }
}
