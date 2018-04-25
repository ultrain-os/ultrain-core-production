let contract  = require('../helpers/web3-contract')

const head = ( callback ) => {
  web3.eth.getBlock().then( callback )
}

const head_timestamp = ( callback ) => {
  web3.eth.getBlock().then( block => callback(block.timestamp) )
}

const head_number = ( callback ) => {
  web3.eth.getBlockNumber().then( callback )
}

const get = ( block_number, callback ) => {
  web3.eth.getBlock( block_number ).then( callback )
}

module.exports = {
  head: head,
  head_number: head_number,
  head_timestamp: head_timestamp,
  get: get
}
