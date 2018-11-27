// import {getActionsByAccount} from "u3.js/src";

const { U3 } = require('u3.js');

path = require('path');
const fs = require('fs');
const ini = require('ini');

const { createU3, format } = U3;

let rpcUrl = "http://127.0.0.1:8888/v1/chain/get_subchain_committee";

let config = {
    // httpEndpoint:'http://172.16.10.4:8888',
    // httpEndpoint_history: 'http://172.16.10.4:3000',
    // keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..


    // httpEndpoint: 'http://40.117.73.83:8888',
    // httpEndpoint_history: 'http://40.117.73.83:3000',
    // keyProvider: ['5KATDC5YqmSTfy99BWqRugriDmeaTAqpwZxXV8jQafdwJqTaqF4'], // WIF string or array of keys..
    httpEndpoint: 'http://172.16.10.4:8877',
    httpEndpoint_history: 'http://172.16.10.4:3000',
    keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..

    chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: true,
    verbose: false,
    logger: {
        log: console.log,
        error: console.error,
        debug: console.log
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};

let configSub = {
    // httpEndpoint:'http://172.16.10.4:8888',
    // httpEndpoint_history: 'http://172.16.10.4:3000',
    // keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..


    // httpEndpoint: 'http://40.117.73.83:8888',
    // httpEndpoint_history: 'http://40.117.73.83:3000',
    // keyProvider: ['5KATDC5YqmSTfy99BWqRugriDmeaTAqpwZxXV8jQafdwJqTaqF4'], // WIF string or array of keys..
    httpEndpoint: 'http://172.16.10.8:8877',
    httpEndpoint_history: 'http://172.16.10.4:3000',
    keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..

    chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: true,
    verbose: false,
    logger: {
        log: console.log,
        error: console.error,
        debug: console.log
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};

let u3 = createU3(config);
let u3Sub = createU3(configSub);

// // 异步打开文件
// console.log("准备打开文件！");
// fs.readFile('CommandRun.js', {flag: 'r+', encoding: 'utf8'}, function(err, fd) {
//     if (err) {
//         return console.error(err);
//     }
//     console.log("文件打开成功！"+ fd.toString());
//
// });

var path = '/root/.local/share/ultrainio/nodultrain/config.ini';

var configIni = ini.parse(fs.readFileSync(path,'utf-8'));

configIni.name = 'helloworld';
configIni.version = '2,0.0';

console.log(configIni);
console.log(configIni.key);
console.log(configIni.aaa);
console.log(configIni["my-account-as-committee"]);
console.log(configIni["my-sk-as-committee"]);

//修改config.ini文件
// fs.writeFileSync('./config.ini',ini.stringify(configIni,{section:''}));


var myAccountAsCommittee = configIni["my-account-as-committee"];
var mySkAsCommittee = configIni["my-account-as-committee"];

function transferToUltrainio() {
    u3.contract('utrio.token').then(actions => {
        actions.transfer('wxjhust12345', 'ultrainio', '1.0000 UGAS', 'transfer memo').then((unsigned_transaction) => {
            u3.sign(unsigned_transaction, config.keyProvider[0], config.chainId).then((signature) => {
                if (signature) {
                    let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                    console.log(signedTransaction);

                    u3.pushTx(signedTransaction).then((processedTransaction) => {
                        console.log(processedTransaction);
                        // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                    });
                }
            })
        })
    });
}

var jsonArray = [];

function buildCommittee(resultJson, jsonArray, add, result) {
    /**
     *
     * [{
            "account": "user.114",
            "public_key": "b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
            "url": "https://user.115.com",
            "location": 0
        }, {
            "account": "user.115",
            "public_key": "8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
            "url": "https://user.115.com",
            "location": 0
        }]

     * **/

    for(var i in resultJson) {

        if (resultJson[i]) {
            console.log("result[i]=", resultJson[i].owner);
            console.log("result[i]=", resultJson[i].miner_pk);

            if(jsonArray.length == 0) {
                result.push({account: resultJson[i].owner, public_key: resultJson[i].miner_pk, url: "https://user.115.com", location:0, adddel_miner:add});
                continue;
            }

            var isInJsonArray = false;
            for(var i2 in jsonArray) {

                if (jsonArray[i2]) {
                    if(jsonArray[i2].owner == resultJson[i].owner) {
                        isInJsonArray = true;
                        break;
                    }
                }
            }

            if(isInJsonArray) {
                continue;
            }

            result.push({account: resultJson[i].owner, public_key: resultJson[i].miner_pk, url: "https://user.115.com", location:0, adddel_miner:add});
        }
    }

    console.log("result array=", result);
    console.log("result array=", result.length);

    return result;
}

function invokeSystemContract( resultJson) {

    var result = [];

    result = buildCommittee(resultJson, jsonArray, 1, result);

    result = buildCommittee(jsonArray, resultJson, 0, result);

    u3Sub.contract('ultrainio').then(actions => {
        actions.votecommittee(myAccountAsCommittee, result).then((unsigned_transaction) => {
            u3.sign(unsigned_transaction, mySkAsCommittee, config.chainId).then((signature) => {
                if (signature) {
                    let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                    console.log(signedTransaction);

                    u3.pushTx(signedTransaction).then((processedTransaction) => {
                        console.log(processedTransaction);
                        // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                    });
                }
            })
        })
    });

    //列表更新
    jsonArray = resultJson;
}

pushHeaderToTestnet = async (timestamp, proposer, version, previous, transaction_mroot, action_mroot, committee_mroot, header_extensions) => {
    console.log("=======" + timestamp + "," + proposer + "," + version + "," + previous + "," + transaction_mroot + "," + action_mroot + "," + committee_mroot + "," + header_extensions)

    await u3.contract('ultrainio').then(async actions => {
        let unsigned_transaction = await actions.acceptheader(format.encodeName('111'), {
                "timestamp": timestamp,
                "proposer": proposer,
                // "proposerProof": proposerProof,
                "version": version,
                "previous": previous,
                "transaction_mroot": transaction_mroot,
                "action_mroot": action_mroot,
                "committee_mroot": committee_mroot,
                "header_extensions": [],
            },
            {
                authorization: [`genesis@active`]
            })

        console.log("=======================unsigned_transaction============" + unsigned_transaction)

        u3.sign(unsigned_transaction, config.keyProvider[0], config.chainId).then((signature) => {
            if (signature) {
                let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                console.log("=======================invoke============" + signedTransaction);

                u3.pushTx(signedTransaction).then((processedTransaction) => {
                    console.log(processedTransaction);
                    // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                });
            }
        })
    });
}

const getData = async() => {

    let result = await u3.getAccountInfo({
        "account_name": "wxjhust12345"
    });
    console.log("u3.getAccountInfo result is:", result);


    result = await u3.getContract({
        // "block_id": "000523f0200dbaf61f5415a7078e57c3c87df0e4450226ae4a654e68a3c34f74"
        "account_name": "wxjhust12345"
    });
    console.log("u3.getContract result is:", result);

    result = await u3.getSourcerate({
        "account_name": "wxjhust12345"
    });
    console.log("u3.getSourcerate result is:", result);

    result = await u3.getRawCodeAndAbi({
        "account_name": "wxjhust12345"
    });
    console.log("u3.getRawCodeAndAbi result is:", result);

    result = await u3.getAllAccounts({
        'page': 1,
        'pageSize': 10,
        'queryParams': {},
        'sortParams': { _id: -1 }
    });
    console.log("u3.getAllAccounts result is:", result);

    // result = await u3.getAllBlocks({
    //     'page': 1,
    //     'pageSize': 10,
    //     'queryParams': {},
    //     'sortParams': { _id: -1 }
    // });
    // console.log("u3.getAllBlocks result is:", result);

    result = await u3.getAllTxs({
        'page': 1,
        'pageSize': 10,
        'queryParams': {},
        'sortParams': { _id: -1 }
    });
    console.log("u3.getAllTxs result is:", result);

    result = await u3.getBlocksByContract({
        'block_num': 1,
        'account': "ultrainio",
        'contract': "utrio.token",
        'contract_method': "transfer"
    });
    console.log("u3.getBlocksByContract result is:", result);

    result = await u3.getContractByName({
        'name': 'utrio.token'
    });
    console.log("u3.getContractByName result is:", result);

    result = await u3.getContracts({
        'page': 1,
        'pageSize': 10,
        'queryParams': {},
        'sortParams': { _id: -1 }
    });
    console.log("u3.getContracts result is:", result);


    result = await u3.getExistAccount(
        'ultrainio'
    );
    console.log("u3.getExistAccount result is:", result);

    result = await u3.getTxByTxId('f71c0a90c40dbb2b5a8bb107c18c9c91305dcb89685ffdf43fde56fd9e685a5f'
    );
    console.log("u3.getTxByTxId result is:", result);


    result = await u3.getTxsByBlockNum(1, 10, {block_num: 111,}, {_id: -1});
    console.log("u3.getTxsByBlockNum result is:", result);

    result = await u3.search("wxjhust12345");
    console.log("u3.search result is:", result);
}


var schedule = require('node-schedule');

var blockNum = 1;
function scheduleCronstyle(){
    schedule.scheduleJob('*/3 * * * * *', function(){
        console.log('scheduleCronstyle:' + new Date());
        getBlocks();

        getSubchainCommittee();

    });
}




// getData();
// transferToUltrainio();

// invokeSystemContract();

const getBlocks = async () => {
    // let resultJson = await u3.getAllBlocksHeader(1, 2, {}, {_id: -1});
    //
    // console.log("===============u3.resultJson.results.results[0].block result is:", resultJson.results);

    while(true) {
        let result = await u3.getBlockInfo(blockNum.toString());

        if (result) {
            blockNum++;
            console.log("u3.getBlockInfo result is:", result);

            pushHeaderToTestnet(result.timevalue, 'genesis', result.version, result.previous, result.transaction_mroot, result.action_mroot, result.committee_mroot, result.header_extensions)

        } else {
            console.log("u3.getBlockInfo result is null,break", result);
            break;
        }
    }




    // result = resultJson.results;
    // let blocks = result[0].block;
    //
    // for(var i in result) {
    //     if(result[i]) {
    //         console.log("result[i]=",result[i]);
    //         console.log("===================u3.getBlockInfo result.proposer is:",result[i].block.proposer);
    //         console.log("===================u3.getBlockInfo result.timestamp is:",result[i].block.timestamp);
    //         console.log("===================u3.getBlockInfo result.proposerProof is:",result[i].block.proposerProof);
    //         console.log("===================u3.getBlockInfo result.version is:",result[i].block.version);
    //         console.log("===================u3.getBlockInfo result.previous is:",result[i].block.previous);
    //         console.log("===================u3.getBlockInfo result.transaction_mroot is:",result[i].block.transaction_mroot);
    //         console.log("===================u3.getBlockInfo result.action_mroot is:",result[i].block.action_mroot);
    //         console.log("===================u3.getBlockInfo result.header_extensions is:",result[i].block.header_extensions);
    //         console.log("===================u3.getBlockInfo result.block_num is:",result[i].block_num);
    //         console.log("===================u3.getBlockInfo result.ref_block_prefix is:",result[i].ref_block_prefix);
    //
    //         // pushHeaderToTestnet(new Date(result[i].timestamp).getTime()/1000, result[i].proposerProof, result[i].version, result[i].previous, result[i].transaction_mroot, result[i].action_mroot)
    //
    //         pushHeaderToTestnet(result[i].timevalue, 'genesis', result[i].version, result[i].previous, result[i].transaction_mroot, result[i].action_mroot, result[i].committee_mroot, result[i].header_extensions)
    //
    //
    //     }
    // }

}
const getSubchainCommittee = async () => {
    console.log("缓存的jsonArray="+jsonArray);
    // let result = await u3.getSubchainCommittee({"chain_name":"0"});

    let result = [];

    result.push({owner: "genesis"+Math.round(Math.random()*10), miner_pk: "369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"});
    result.push({owner: "genesis"+Math.round(Math.random()*10), miner_pk: "369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"});
    result.push({owner: "genesis"+Math.round(Math.random()*10), miner_pk: "369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"});

    for(var i in result) {
        if (result[i]) {
            console.log("result[i]=", result[i].owner);
            console.log("result[i]=", result[i].miner_pk);

        }
    }

    invokeSystemContract(result);

}



// let u3.getActionsByAccount({
//     'page': 1,
//     'pageSize': 10,
//     'queryParams': {account:'ultrainio'},
//     'sortParams': { _id: -1 }
// }).then((result) => {
//     console.log("u3.getActionsByAccount result is:", result);
// })




// const name = 'abc';
// let params = {
//     creator: 'ultrainio',
//     name: name,
//     owner: pubkey,
//     active: pubkey,
//     updateable: 0,
//     ram_bytes: 6666,
//     stake_net_quantity: '1.0000 UGAS',
//     stake_cpu_quantity: '1.0000 UGAS',
//     transfer: 0
// };
//
// let result = await u3.createUser(params);




// getBlocks();
scheduleCronstyle();




