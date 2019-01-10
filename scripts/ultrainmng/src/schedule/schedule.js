const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig").getLogger("Schedule");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');
var ShellCmd = require('../common/util/shellCmd');
var CmdConstants = Constants.cmdConstants;
var WorldState = require("../worldstate/worldstate")
var NodUltrain = require("../nodultrain/nodultrain")
var sleep = require('sleep')

var Chain = require("../chain/chain")
var chainApi = require("../chain/chainApi")
var chainConfig = require("../chain/chainConfig")
/**
 * 调度程序
 * 调度到新链，增加新链
 */

/**
 * 子链切换
 * @returns {Promise<void>}
 */
var chainChange = async function (chainId, nodFilePath) {



}

/**
 * 新节点入子链
 * @returns {Promise<void>}
 */
var addNewNodeToChain = async function (chainId, nodFilePath) {

}

