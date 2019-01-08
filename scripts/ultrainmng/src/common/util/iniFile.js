var fs = require('fs');
var ini = require('ini');
var logger = require("../../config/logConfig");
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
            this.obj = ini.parse(fs.readFileSync(filepath, encoding));
        } catch (e) {
            logger.error("parse ini file error:" + filepath);
        }
    }

    /**
     * 根据key 输出内容
     * @param key
     */
    getValue(key) {
        if (utils.isNotNull(this.obj)) {
            return this.obj[key];
        }
    }

    /**
     * 设置key，value
     * @param key
     * @param value
     */
    setValue(key, value) {
        if (utils.isNotNull(this.obj)) {
            this.obj[key] = value;
        }
    }


    /**
     * 写入文件
     * @param filepath
     * @param encoding
     */
    writefile(filepath, encoding) {
        try {
            fs.writeFileSync(filepath, ini.stringify(this.obj, {section: ''}, encoding));
            return true;
        } catch (e) {
            logger.error("write ini file error: " + filepath, e);
            return false;
        }
    }


}

module.exports = IniFile;
