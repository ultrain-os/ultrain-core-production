const async     = require('async'),
      db        = require('../models')

let util        = require('../utilities'),
    Iterator    = require('../classes/Iterator')

const pubkey_from_transfer = ( address, callback ) => query_address_tx_hash('Transfers',     'from',     address, callback)
const pubkey_from_buy = ( address, callback)       => query_address_tx_hash('Buys',          'address',  address, callback)
const pubkey_from_claim = ( address, callback)     => query_address_tx_hash('Claims',        'address',  address, callback)
const pubkey_from_register = ( address, callback)  => query_address_tx_hash('Registrations', 'address',  address, callback)

const query_address_tx_hash = ( type, field, address, callback ) => {
  db[type]
    .findAll({
      where: {
        [field]: address
      },
      attributes: ['tx_hash'],
      limit:1,
      order : [[ 'block_number', 'DESC' ]]
    })
    .then( results => {
      if(!results)
        callback(`${address} not found in ${type}`)
      else
        util.misc.tx_hash_from_query_results(results, tx_hash => {
          if(tx_hash)
            util.misc.pubkey_from_tx_hash(tx_hash, pubkey => {
              callback(null, pubkey)
            })
          else
            callback(`${address} not found in ${type}`)
        })
    })
}

const pubkey_from_cache = ( address, callback ) => {
  db.Keys
    .findAll({
      where: {
        address : address
      },
      attributes: ['public_key'],
      limit:1
    }, {type: db.sequelize.QueryTypes.SELECT})
    .then( results => {
      if(results.length)
        callback( null, results[0].dataValues.public_key )
      else
        callback('ethpk_not_found')
    })
}

const complete = (error, pubkey, callback) => {
  if(pubkey) {
    let ultrain_key = util.misc.convert_ethpk_to_ultrainpk(pubkey)
    if(util.misc.is_ultrain_public_key(ultrain_key))
      callback(null, ultrain_key)
      // console.log(`ULTRAIN Key Generated: ${pubkey} => ${ultrain_key}`),
    else
      callback('converted_key_invalid')
  }
  else {
    callback('ethpk_not_found')
  }
}

const skip = (address) => [CS_ADDRESS_CROWDSALE, CS_ADDRESS_TOKEN, CS_ADDRESS_B1].indexOf(address.toLowerCase()) > -1

module.exports = ( address, cache, finished ) => {
  if( skip(address) )
    finished()
  else
    util.misc.address_is_contract( address, is_contract => {
      if(is_contract)
        finished( 'address_is_contract' )
      else
        async.tryEach([
            found => pubkey_from_buy( address, found ),           //Most common catch
            found => pubkey_from_transfer( address, found ),      //Second most common catch
            found => pubkey_from_claim( address, found ),         //for confused people? (edge)
            found => pubkey_from_register( address, found ),      //maybe did a register incorrectly from an non-contributor address and recieved later with no outgoing txs? (edge)
            found => pubkey_from_cache(address, found)
        ],
        (error, pubkey) => complete(error, pubkey, finished)) // TODO look into why this callback being looped through, must be mistake. :/
    })
}
