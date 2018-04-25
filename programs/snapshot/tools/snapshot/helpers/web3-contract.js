const abi = require('../lib/abi.js')

module.exports = {
  $crowdsale: new web3.eth.Contract(abi.crowdsale,  CS_ADDRESS_CROWDSALE),
  $token:     new web3.eth.Contract(abi.token,      CS_ADDRESS_TOKEN),
  $utility:   new web3.eth.Contract(abi.utility,    CS_ADDRESS_UTILITIES)
}
