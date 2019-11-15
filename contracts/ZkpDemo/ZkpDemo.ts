import { Contract } from "../../../src/contract";
import { verify_zero_knowledge_proof } from "../../../src/crypto";
import { NAME } from "../../../src/account";

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


const TABLE_RESULT: string = "result";
const TABLE_KEY: string = "key";
const VERIFY_KEY_ID: u64 = 1;

@database(Record, TABLE_RESULT)
@database(Vk, TABLE_KEY)
export　class ZkpDemo extends Contract {
    
    private resultManager: DBManager<Record>;
    private vkManager: DBManager<Vk>;

    constructor(receiver: account_name) {
        super(receiver);
        this.resultManager = new DBManager<Record>(NAME(TABLE_RESULT), NAME(TABLE_RESULT));
        this.vkManager = new DBManager<Vk>(NAME(TABLE_KEY), NAME(TABLE_KEY));
    }

    @action
    setVerifyKey(key: string): void {
        var id = VERIFY_KEY_ID;
        var vk = new Vk();
        vk.id =  id;
        vk.key = key;
        if (!this.vkManager.exists(id)) {
            this.vkManager.emplace(vk);
        } else {
            this.vkManager.modify(vk)
        }
    }


    @action
    testZkp(pk_input: string, proof: string, expect: i32): void {
        var vk = this.getVerifyKey();
        var result = verify_zero_knowledge_proof(vk,pk_input,proof);
        var pk = this.getLatestPk();
        var record = new Record();
        record.id = pk;
        record.val = result ? 1 : 0;
        // Log.s("record.id: ").i(record.id).flush();
        if (!this.resultManager.exists(record.id)) {
            this.resultManager.emplace(record);
        }
        ultrain_assert(record.val == expect, "The zkp test case result is not expect.")
    }

    private getVerifyKey(): string {
        if (this.vkManager.exists(VERIFY_KEY_ID)) {
            let vk = new Vk();
            this.vkManager.get(VERIFY_KEY_ID, vk);
            return vk.key;
        }
        ultrain_assert(false, "Please set the verify key.");
        return "";
    }

    private getLatestPk(): u64 {
        var const_pk = 1; 
        var record = new Record();
        if (this.resultManager.exists(const_pk)) {
            this.resultManager.get(const_pk, record);
            record.val  = record.val + 1;
            this.resultManager.modify(record);
            return record.val;
        } else {
            let startPk = 1122;
            record.id = const_pk;
            record.val = startPk;
            this.resultManager.emplace(record);
            return record.val;
        }
    }
}
