const ecc       = require('ultrainjs-ecc'),
      secp256k1 = require('secp256k1'),
      ejs       = require('ethereumjs-util'),
      contract  = require('../helpers/web3-contract')

const repeat = (x, n) => {
  return new Array(n + 1).join("x")
}

const iota = n => {
  return repeat("x", n).split("").map((_, i) => i)
}

const hex_to_ascii = hex => {
  return web3.utils.hexToAscii( hex )
}

const is_ultrain_public_key = ultrain_key => {
  return ecc.PublicKey.fromString(ultrain_key) != null
}

const get_pubkey_from_tx = ( tx_hash, callback ) =>{
  web3.eth.getTransaction(txhash).then( tx => callback(tx.publicKey) )
}

const get_tx = (tx_hash) => {
  return web3.eth.getTransaction(tx_hash)
}

const get_user_key = (address, callback) => {
  contract.$crowdsale.methods.keys( address ).call().then( callback )
}

const is_tx_confirmed = ( txhash, callback ) => {
  web3.eth.getTransactionReceipt(txhash)
    .then( tx => ( typeof tx === 'object' ? callback(true) : callback(false) ) )
}

const address_is_contract = ( address, callback ) => {
  web3.eth.getCode(address)
    .then( result => callback(result=='0x' ? false : true) )
}

const tx_hash_from_query_results = ( results, callback ) => (results.length) ? callback( results[0].dataValues.tx_hash ) : callback( false )

const pubkey_from_tx_hash = ( tx_hash, callback ) => get_tx(tx_hash).then( result => { callback( result.publicKey ) })

const convert_ethpk_to_ultrainpk = ( pubkey ) => {
  let buffer    = Buffer.from(pubkey.slice(2), 'hex'),
      converted = secp256k1.publicKeyConvert(Buffer.concat([ Buffer.from([4]), buffer ]), true),
      ultrainkey    = ecc.PublicKey.fromBuffer(converted).toString()
  return ultrainkey
}

module.exports = {
  iota: iota,
  hex_to_ascii: hex_to_ascii,
  is_ultrain_public_key: is_ultrain_public_key,
  get_pubkey_from_tx: get_pubkey_from_tx,
  get_user_key: get_user_key,
  get_tx: get_tx,
  is_tx_confirmed: is_tx_confirmed,
  tx_hash_from_query_results: tx_hash_from_query_results,
  pubkey_from_tx_hash: pubkey_from_tx_hash,
  convert_ethpk_to_ultrainpk: convert_ethpk_to_ultrainpk,
  address_is_contract: address_is_contract
}
