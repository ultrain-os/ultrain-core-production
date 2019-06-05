var fs = require('fs');
var ini = require('ini');
var logger = require("../../config/logConfig").getLogger("IniFile");
const utils = require("./utils")

/**
 * ini文件对象
 */
class IniFile {

    /**
     * 构造函数
     * @param filepath
     * @param encoding
     */
    constructor(filepath, encoding) {
        try {
            this.lines = fs.readFileSync(filepath, encoding).split("\n");
        } catch (e) {
            logger.error("parse ini file error:" + filepath);
        }
    }

    /**
     * 根据key 输出内容
     * @param key
     */
    getValue(key) {

        let value = null;
        for (var i=0;i<this.lines.length;i++) {
            let line = this.lines[i];
            if (this.isCommentLine(line)) {
                continue;
            }

            var array = this.convertKV(line);
            if (utils.isNotNull(array) && array[0].trim() == key) {
                value = array[1];
                break;
            }
        }

        return value;
    }

    /**
     * 判断首字母是不是#
     * @param lineStr
     * @returns {boolean}
     */
    isCommentLine(lineStr) {

        if (utils.isNull(lineStr)) {
            return true;
        }

        if (lineStr.indexOf("#") == 0) {
            return true;
        }

        return false;
    }

    /**
     * 转换为kv对
     * @param lineStr
     * @returns {*}
     */
    convertKV(lineStr) {
        let array = lineStr.split("=");
        if (array.length != 2) {
            return null;
        }

        return array;
    }

    /**
     * 设置key，value
     * @param key
     * @param value
     */
    setValue(key, value) {

        let findFlag = false;
        for (var i=0;i<this.lines.length;i++) {
            let line = this.lines[i];
            if (this.isCommentLine(line)) {
                continue;
            }

            var array = this.convertKV(line);
            if (utils.isNotNull(array) && array[0].trim() == key) {
                findFlag = true;
                this.lines[i]=array[0]+"="+value;
                break;
            }
        }

        if (findFlag == false) {
            this.lines.push(key+"="+value)
        }

        return value;
    }

    /**
     *
     * @param key
     * @param value
     * @returns {boolean}
     */
    checkKVExist(key, value) {
        for (var i=0;i<this.lines.length;i++) {
            let line = this.lines[i];
            if (this.isCommentLine(line)) {
                continue;
            }

            var array = this.convertKV(line);
            if (utils.isNotNull(array) && array[0].trim() == key && array[1].trim() == value) {
                return true;
            }
        }

        return false;
    }

    /**
     * 删除key
     * @param key
     */
    removeKey(key){
        for (var i=0;i<this.lines.length;i++) {
            let line = this.lines[i];
            if (this.isCommentLine(line)) {
                continue;
            }
            var array = this.convertKV(line);
            if (utils.isNotNull(array) && array[0].trim() == key) {
                this.lines[i] = "";
                delete this.lines[i];
            }
        }

    }

    /**
     * 添加key-value，不管是否会重复
     * @param key
     * @param value
     * @returns {*}
     */
    addKeyValue(key, value) {
        this.lines.push(key+"="+value)
    }


    /**
     * 写入文件
     * @param filepath
     * @param encoding
     */
    writefile(filepath, encoding) {
        try {
            let output = "";
            let empty = false;
            for (var i=0;i<this.lines.length;i++) {
                if (this.lines[i] != undefined) {
                    if (this.lines[i].length >0) {
                        output = output + this.lines[i] + "\n";
                        empty = false;
                    } else {
                        if (empty == false) {
                            output = output + this.lines[i] + "\n";
                        }
                        empty = true;
                    }
                }
            }

            fs.writeFileSync(filepath, output, encoding);
            return true;
        } catch (e) {
            logger.error("write ini file error: " + filepath, e);
            return false;
        }
    }


}

module.exports = IniFile;
