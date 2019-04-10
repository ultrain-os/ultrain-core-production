const { createU3 } = require("u3.js/src");
const fs = require('fs');
const ini = require('ini');
const process = require('child_process');

var config = require("../config");

const mngConfigPath = "/root/.local/share/ultrainio/ultrainmng/config/config.ini";
function getRandToolsPath() {
  var nodeConfig = ini.parse(fs.readFileSync(mngConfigPath, 'utf-8'));
  return nodeConfig["randToolsPath"];
}

// global config
var toolsPath = getRandToolsPath();
var g_vrf_client_path = `${toolsPath}/vrf_client`;
var g_node_config_path = "/root/.local/share/ultrainio/nodultrain/config/config.ini";
var g_perl_script_path = `${toolsPath}/b58.pl`;
var g_seed_config_path = "/root/.local/share/ultrainio/ultrainmng/config/seedconfig.json";
var g_chain_id = undefined;

function base64Sk(securityKey) {
  var script = g_perl_script_path;
  var cmd = `perl ${script} ${securityKey}`;
  return process.execSync(cmd).toString();
}

var votingData = [];
function isDuplicate(key) {
  var found = votingData.find(function(element){
    return element == key;
  });
  if (found) {
    return found;
  }
  votingData.push(key);
  if (votingData.length > 10) {
    votingData.shift();
  }
  return false;
}

function generateVrfProof(sk, randNum) {
  var basedSk = base64Sk(sk);
  randNum = randNum.length % 2 == 0 ? randNum : randNum + "0";
  var cmd = `${g_vrf_client_path} -p ${basedSk} ${randNum}`;
  var res = process.execSync(cmd).toString();
  return res.split(";")[1];
}

function readNodeConfig() {
  var filePath = g_node_config_path;
  var nodeConfig = ini.parse(fs.readFileSync(filePath, 'utf-8'));
  return nodeConfig;
}

async function getChainId(config){
  if (g_chain_id){
    return g_chain_id;
  }
  var u3 = createU3(config);
  var chainInfo = await u3.getChainInfo();
  return chainInfo.chain_id;
}

function getHttpServerAddress(){
  try {
    var filePath = g_seed_config_path;
    var chainName = readNodeConfig()["chain-name"];
    var seedConfig = JSON.parse(fs.readFileSync(filePath, 'utf-8'));
    for (let i = 0; i < seedConfig.length; i++) {
      let len = seedConfig[i].seedIp.length;
      if (seedConfig[i].chainName == chainName && len != 0) {
        let seedIp = seedConfig[i].seedIp;
        let index = Math.floor(Math.random() * len);
        return `http://${seedIp[index]}:8888`;
      }
    }
  } catch (exception){
    return "http://127.0.0.1:8888";
  }
}

async function votingRandomNum() {
  var nodeConfig = readNodeConfig();
  var account = nodeConfig["my-account-as-committee"];
  var securityKey = nodeConfig["my-sk-as-account"];
  var httpServerAddress = getHttpServerAddress();

  config.httpEndpoint = httpServerAddress;
  config.keyProvider = [securityKey];
  config.chainId = await getChainId(config);
  var u3 = createU3(config);

  const contName = "utrio.rand";
  const contract = await u3.contract(contName);
  const authorize = { authorization: [`${account}@active`] };

  var tr = await contract.getMainVoteInfo(authorize);

  var returnValue = tr.processed.action_traces[0].return_value;
  var parameters = returnValue.split(",");
  var bckNum = parameters[0];
  var randNum = parameters[1];
  var code = parameters[2];

  if (code != -1) {
    if (isDuplicate(bckNum)) {
      console.error(`The voting of ${bckNum} is duplicated`);
    } else {
      let proof = generateVrfProof(securityKey, randNum);
      await contract.vote(proof, bckNum, authorize);
    }
  } else {
    console.warn(`Current user ${account} is not main voter.`);
  }
}

module.exports = {
  votingRandomNum
}
