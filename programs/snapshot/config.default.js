module.exports = {

  period: 1,
  include_b1: true,
  cache: false,
  fallback: true,
  overwrite_snapshot: false,
  recalculate_wallets: false,
  author: "Anonymous",

  //ETH node
  eth_node_type: '',
  eth_node_path: '',

  //redis
  redis_host: null,
  redis_port: null,

  mysql_db: "ultrain_snapshot",
  mysql_user: "root",
  mysql_pass: null,
  mysql_host: "localhost",
  mysql_port: 3306

}
