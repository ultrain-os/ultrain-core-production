

var process = require('child_process');
//直接调用命令
exports.createDir = function (){
        process.exec('pwd & mkdir ttt',
            function (error, stdout, stderr) {
                if (error !== null) {
                    console.log('exec error: ' + error);
                } else {
                    console.log('stdout:',stdout);
                }
            });

}
//调用执行文件
exports.openApp = function(){
    process.execFile('D:/testweb/aaa.bat',null,{cwd:'D:/'},
        function (error,stdout,stderr) {
            if (error !== null) {
                console.log('exec error: ' + error);
            }
        });
}


exports.createDir();
exports.openApp ();
