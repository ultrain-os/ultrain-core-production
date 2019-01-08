const {U3} = require('u3.js');
var logger = require("../config/logConfig");
var chainConfig = require("./chainConfig")


async  function test() {
   await chainConfig.syncConfig();
    chainConfig.printInfo()
}

test();
