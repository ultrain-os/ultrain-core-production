var logger = require("../../config/logConfig").getLogger("Schedule");
var utils = require("../util/utils")
var fs = require('fs');
var constant = require("../constant/constants")

/**
 * 缓存对象
 */
class CacheObj {
    /**
     * 构造函数
     * @param persist
     * @param filepath
     */
    constructor(persist, filepath) {
        this.persist = persist;
        this.filepath = filepath;
        this.cacheData = {};
        if (this.persist) {
            try {
                if (fs.existsSync(this.filepath)) {
                    var data = fs.readFileSync((this.filepath), constant.encodingConstants.UTF8);
                    if (utils.isNotNull(data)) {
                        this.cacheData = JSON.parse(data);
                    }
                } else {
                    fs.writeFileSync(this.filepath, "{}");
                }
            } catch (e) {
                logger.error("cacheobj read file error:", e);
            }
        }
    }


    /**
     * 设置
     * @param key
     * @param value
     */
    put(key, value) {
        this.cacheData[key] = value;
        return this.saveTofile();
    }

    /**
     * 获取结果
     * @param key
     * @returns {*}
     */
    get(key) {
        return this.cacheData[key]
    }

    /**
     * 删除key
     * @param key
     */
    delete(key) {
        delete this.cacheData[key];
        return this.saveTofile();
    }

    /**
     * 获取全部
     * @returns {*}
     */
    getAll() {
        return this.cacheData;
    }

    /**
     * 获取全部
     * @returns {*}
     */
    clear() {
        this.cacheData = {}
        this.saveTofile()
    }

    /**
     * 保存文件
     * @returns {boolean}
     */
    saveTofile() {

        if (this.persist == false) {
            return true;
        }

        try {
            fs.writeFileSync(this.filepath, JSON.stringify(this.cacheData));
        } catch (e) {
            logger.error("write file data error:", e);
            return false;
        }

        return true;
    }
}


module.exports = CacheObj

