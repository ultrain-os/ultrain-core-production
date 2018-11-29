// var spawn = require('child_process').spawn;
// var process = require('process');
//
// var p = spawn('node',['SideChainBlockInfoService.js']);
// console.log(process.pid, p.pid);


var spawn = require('child_process').spawn;
var process = require('process');

var p = spawn('node',['SideChainBlockInfoService.js'],{
    detached : true
});
console.log(process.pid, p.pid);
process.exit(0);

