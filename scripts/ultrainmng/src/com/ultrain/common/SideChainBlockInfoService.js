// import {getActionsByAccount} from "u3.js/src";

const { U3 } = require('u3.js');

path = require('path');
const fs = require('fs');
const ini = require('ini');

const { createU3, format } = U3;

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

    chainId: '0eaaff4003d4e08a541332c62827c0ac5d96766c712316afe7ade6f99b8d70fe', // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
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
    // httpEndpoint: 'http://172.16.10.5:8899',
    // httpEndpoint_history: 'http://172.16.10.5:3000',
    httpEndpoint: 'http://127.0.0.1:8888',
    httpEndpoint_history: 'http://127.0.0.1:3000',
    keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..

    chainId: '0eaaff4003d4e08a541332c62827c0ac5d96766c712316afe7ade6f99b8d70fe', // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
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

// let u3 = createU3(config);
let u3 = createU3({ ...config, sign: true, broadcast: true });
let u3NotPush = createU3({ sign: false, broadcast: false });

let u3Sub = createU3({ ...configSub, sign: true, broadcast: true });

// // 异步打开文件
// console.log("准备打开文件！");
// fs.readFile('CommandRun.js', {flag: 'r+', encoding: 'utf8'}, function(err, fd) {
//     if (err) {
//         return console.error(err);
//     }
//     console.log("文件打开成功！"+ fd.toString());
//
// });

var path = '/root/.local/share/ultrainio/nodultrain/config/config.ini';
path = "config.ini";

var configIni = ini.parse(fs.readFileSync(path,'utf-8'));

configIni.name = 'helloworld';
configIni.version = '2,0.0';

console.log(configIni);
console.log(configIni.key);
console.log(configIni.aaa);
console.log(configIni["my-account-as-committee"]);
console.log(configIni["my-sk-as-committee"]);

//修改config.ini文件
fs.writeFileSync('./config.ini',ini.stringify(configIni,{section:''}));


var myAccountAsCommittee = configIni["my-account-as-committee"];
var mySkAsCommittee = configIni["my-account-as-committee"];

function transferToUltrainio() {
    u3.contract('utrio.token').then(actions => {
        actions.transfer('user.111', 'user.112', '1.0000 UGAS', 'transfer memo').then((unsigned_transaction) => {
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

    try {

        u3Sub.contract('ultrainio').then(actions => {
            actions.votecommittee(myAccountAsCommittee, result).then((unsigned_transaction) => {
                u3Sub.sign(unsigned_transaction, mySkAsCommittee, config.chainId).then((signature) => {
                    if (signature) {
                        let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                        console.log(signedTransaction);

                        u3Sub.pushTx(signedTransaction).then((processedTransaction) => {
                            console.log(processedTransaction);
                            // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                        });
                    }
                })
            })
        });
    } catch (e) {
        console.log("u3 push tx error...",e)
    }

    //列表更新
    jsonArray = resultJson;
}

pushHeaderToTestnet = async (timestamp, proposer, version, previous, transaction_mroot, action_mroot, committee_mroot, header_extensions) => {
    console.log("=======" + timestamp + "," + proposer + "," + version + "," + previous + "," + transaction_mroot + "," + action_mroot + "," + committee_mroot + "," + header_extensions)

    // try{
    //
    //     await u3.contract('ultrainio').then(async actions => {
    //     let unsigned_transaction = await actions.acceptheader(format.encodeName('11'), {
    //             "timestamp": timestamp,
    //             "proposer": proposer,
    //             // "proposerProof": proposerProof,
    //             "version": version,
    //             "previous": previous,
    //             "transaction_mroot": transaction_mroot,
    //             "action_mroot": action_mroot,
    //             "committee_mroot": committee_mroot,
    //             "header_extensions": [],
    //         },
    //         {
    //             authorization: [`user.111@active`]
    //         })
    //
    //     console.log("=======================unsigned_transaction============" + unsigned_transaction)
    //
    //     u3NotPush.sign(unsigned_transaction, config.keyProvider[0], config.chainId).then((signature) => {
    //         if (signature) {
    //             let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
    //             console.log("=======================invoke============" + signedTransaction);
    //
    //             u3.pushTx(signedTransaction).then((processedTransaction) => {
    //                 console.log(processedTransaction);
    //                 // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
    //             });
    //         }
    //     })
    // });
    // }
    // catch(e){
    //     console.log('error..',e);
    // }

    const params = {
        chain_name: 11,
        header: {
            "timestamp": timestamp,
            "proposer": proposer,
            // "proposerProof": proposerProof,
            "version": version,
            "previous": previous,
            "transaction_mroot": transaction_mroot,
            "action_mroot": action_mroot,
            "committee_mroot": committee_mroot,
            "header_extensions": [],
        }
    };

    contractInteract('ultrainio', "acceptheader", params, "user.111", config.keyProvider[0] );
}



var schedule = require('node-schedule');

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

    let blockNum = await u3.getSubchainBlockNum({"chain_name":"11"});
    console.log("u3.getSubchainBlockNum  blockNum="+blockNum);

    let blockNumInt = parseInt( blockNum, 10 )+1;
    if(blockNumInt<2) {
        blockNumInt = 2;
    }

        let result = await u3Sub.getBlockInfo((blockNumInt).toString() );

        if (result) {
            console.log("u3Sub.getBlockInfo result is:", result);
            pushHeaderToTestnet(result.timevalue, result.proposer, result.version, result.previous, result.transaction_mroot, result.action_mroot, result.committee_mroot, result.header_extensions)

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
    let result = await u3.getSubchainCommittee({"chain_name":"11"});

    // let result = [];
    //
    // result.push({owner: "genesis"+Math.round(Math.random()*10), miner_pk: "369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"});
    // result.push({owner: "genesis"+Math.round(Math.random()*10), miner_pk: "369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"});
    // result.push({owner: "genesis"+Math.round(Math.random()*10), miner_pk: "369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"});

    // for(var i in result) {
    //     if (result[i]) {
    //         console.log("result[i]=", result[i].owner);
    //         console.log("result[i]=", result[i].miner_pk);
    //
    //     }
    // }

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


// transferToUltrainio();

async function contractInteract(contractName, actionName, params, accountName,  privateKey) {
    try {
        const keyProvider = [privateKey];
        const u3 = createU3({ ...config, keyProvider });

        const contract = await u3.contract(contractName);
        console.log(contract);
        if (!contract) {
            throw new Error("can't found contract");
        }
        if (!contract[actionName] || typeof contract[actionName] !== 'function') {
            throw new Error("action doesn't exist");
        }
        const data = await contract[actionName](params, {
            authorization: [`${accountName}@active`],
        });
        console.log('success')
    } catch (err) {
        console.error(err);
    }
}