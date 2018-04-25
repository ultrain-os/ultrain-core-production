const contract = require('./web3-contract')
let   collection = {}

collection.transfers = (blockFrom = 0, blockTo = 'latest') => {
  return contract.$token
    .getPastEvents('Transfer', { fromBlock : blockFrom, toBlock : blockTo })
}
collection.buys = (blockFrom = 0, blockTo = 'latest') => {
  return contract.$crowdsale
    .getPastEvents('LogBuy', { fromBlock : blockFrom, toBlock : blockTo })
}
collection.claims = (blockFrom = 0, blockTo = 'latest') => {
  return contract.$crowdsale
    .getPastEvents('LogClaim', { fromBlock : blockFrom, toBlock : blockTo })
}
collection.registrations = (blockFrom = 0, blockTo = 'latest') => {
  return contract.$crowdsale
    .getPastEvents('LogRegister', { fromBlock : blockFrom, toBlock : blockTo })
}

collection.reclaimables = (blockFrom = 0, blockTo = 'latest') => {
  return contract.$token
    .getPastEvents('Transfer', { fromBlock : blockFrom, toBlock : blockTo, filter: { to: [CS_ADDRESS_CROWDSALE,CS_ADDRESS_TOKEN] } } )
}

module.exports = collection
