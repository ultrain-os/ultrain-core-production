const contract = require('./web3-contract')

let address = {}
address.transfersFrom = address => {
  return contract.$token
    .getPastEvents('Transfer', { filter: { from: address } })
}
address.transfersTo = address => {
  return contract.$token
    .getPastEvents('Transfer', { filter: { to: address } } )
}

address.tokenBalance = address => {
  return contract.$token.methods
    .balanceOf( address ).call()
}

address.buys = address => {
  return contract.$utility.methods
    .userBuys( address ).call()
}

address.claims = address => {
  return contract.$utility.methods
    .userClaims( address ).call()
}

address.key = address => {
  return contract.$crowdsale.methods
    .keys( address ).call()
}

module.exports = user
