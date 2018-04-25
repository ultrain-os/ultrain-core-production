var redis = require("redis")
module.exports = (host, port) => {
  redis.createClient( { host:host, port: port } )
}
