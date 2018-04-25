module.exports = ( state, complete ) => {

  const async     = require('async'),
        Table     = require('ascii-table'),

        db        = require('../../models'),
        fallback  = require('../../utilities/local-pubkey'),
        util      = require('../../utilities')

  let   intval,
        pks_found   = 0,
        count       = 0

  const get_uniques = ( next ) => {
    redis.keys('*', (err, addresses) => next( null, addresses ) )
  }

  const process_uniques = ( addresses, next ) => {
    async.eachSeries( addresses, (address, next_address) => {
      fallback( address, config.cache, (error, ultrain_key) => {
        if(error || !ultrain_key)
          db.Wallets
            .update(
              { fallback_error: error || "unknown" },
              { where : { address: address } }
            )
            .then( () => next_address() )
        else
          db.Wallets
            .update(
              {
                ultrain_key: ultrain_key,
                fallback: true,
                fallback_error: null,
                valid: true
              },
              { where : { address: address } }
            )
            .then( () => {
              redis.del(address);
              pks_found++
              next_address()
            })
      })
      count++
    },
    (err, result) => {
      clearInterval( intval )
      log()
      next()
    })
  }

  const log = () => {
    redis.keys('*', (err, addresses) => {

      let table = new Table(`Fast Fallback`),
          success_rate = Math.floor(pks_found/(addresses.length+pks_found)*100)

      table.addRow('Progress', `${Math.floor(count/(addresses.length+pks_found)*100)}%`)
      table.addRow('PKs Found (fast)', pks_found)
      table.addRow('Pks Unfound (fast)', addresses.length)
      table.addRow('Found Rate (fast)', `${success_rate}%`)
      table.addRow('Addresses Processed', count)
      console.log( table.setAlign(0, Table.RIGHT).setAlign(1, Table.LEFT).render() )

    })
  }

  const periodic_log = () => {
    intval = setInterval( log, 10000 )
  }

  console.log('Fallback: Fast')
  periodic_log()
  async.waterfall([
      get_uniques,
      process_uniques
    ],
    () => setTimeout( () => complete(null, state), 5000 )
  )
}
