import { Contract } from "ultrain-ts-lib/src/contract";
import { verify_zero_knowledge_proof } from "ultrain-ts-lib/src/crypto";
import { SHA256 } from "ultrain-ts-lib/src/crypto";
import { NAME } from "ultrain-ts-lib/src/account";
import { Log } from "ultrain-ts-lib/src/log";
import { intToString } from "ultrain-ts-lib/src/utils";

class Record implements Serializable {
    id: u64;
    val: u64;
    primaryKey(): id_type {
        return this.id;
    }
}

class Vk implements Serializable {
    id: u64;
    key: string;

    primaryKey(): id_type {
        return this.id;
    } 
}

class PmInput implements Serializable {
    hash: u64;

    primaryKey(): id_type {
        return this.hash;
    }
}

const TABLE_RESULT: string = "result";
const TABLE_KEY: string = "key";
const TABLE_PRIMARY_INPUT: string = "pminput";

@database(Record, TABLE_RESULT)
@database(Vk, TABLE_KEY)
@database(PmInput, TABLE_PRIMARY_INPUT)
export　class ZkpDemo extends Contract {
    
    private resultManager: DBManager<Record>;
    private vkManager: DBManager<Vk>;
    private pmManager: DBManager<PmInput>;

    constructor(receiver: account_name) {
        super(receiver);
        this.resultManager = new DBManager<Record>(NAME(TABLE_RESULT), NAME(TABLE_RESULT));
        this.vkManager = new DBManager<Vk>(NAME(TABLE_KEY), NAME(TABLE_KEY));
        this.pmManager = new DBManager<PmInput>(NAME(TABLE_PRIMARY_INPUT), NAME(TABLE_PRIMARY_INPUT));
    }

    @action
    setVerifyKey(bid: u64, key: string): void {
        var vk = new Vk();
        vk.id = bid;
        vk.key = key;
        if (!this.vkManager.exists(bid)) {
            this.vkManager.emplace(vk);
        } else {
            this.vkManager.modify(vk)
        }
    }


    @action
    testZkp(caseid: u64, bid: u64, pk_input: string, proof: string, expect: i32): void {
        let pm = intToString(bid);
        let arr: Array<u8> = new Array<u8>();
        for (let i = 0; i < pk_input.length; i++) {
            let code = <u8>pk_input.charCodeAt(i);
            if ( code >= 0x30 && code <= 0x39) {
                arr.push(code);
            }
        }
        pm = pm + String.UTF8.decode(arr.buffer);
	//Log.s("new pm: ").s(pm).flush();
        var sha = new SHA256();
        sha.hash(pm);
        let ds = new DataStream(sha.buffer, 8);
        var pm_record = new PmInput();
	pm_record.hash = ds.read<u64>();

        ultrain_assert(!this.pmManager.exists(pm_record.hash), "the primary input already exists");
        this.pmManager.emplace(pm_record);

        var vk = this.getVerifyKey(bid);
        var result = verify_zero_knowledge_proof(vk,pk_input,proof);
        var record = new Record();
        record.id = caseid;
        record.val = result ? 1 : 0;
        ultrain_assert(record.val == expect, "The zkp test case result is not expect.")
        if (this.resultManager.exists(record.id)) {
            ultrain_assert(false, "The test case already exists");
        } else {
            this.resultManager.emplace(record);
        }
    }

    private getVerifyKey(id: u64): string {
        if (this.vkManager.exists(id)) {
            let vk = new Vk();
            this.vkManager.get(id, vk);
            return vk.key;
        }
        ultrain_assert(false, "Please set the verify key.");
        return "";
    }
}
