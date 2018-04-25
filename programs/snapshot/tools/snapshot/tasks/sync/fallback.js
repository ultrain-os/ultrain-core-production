module.exports = ( state, complete ) => {

  const async         = require('async'),
        Sequelize     = require('sequelize'),
        Op            = Sequelize.Op,

        db            = require('../../models'),
        fallback_fast = require('./fallback.fast'),
        fallback_slow = require('./fallback.slow')

  const populate_index = (callback) => {
    console.log('Querying Eligible addresses')

    db.sequelize
      .query('SELECT address FROM wallets WHERE balance_total>=1 AND register_error IS NOT NULL AND fallback=0', {type: db.sequelize.QueryTypes.SELECT})
      .then( addresses => {
        addresses = addresses.map( result => result.address )
        console.log(`${addresses.length} Found`)
        console.log('Redis: Flushing DB')
        redis.flushdb((err, success) => {
          if(success)
            console.log("Redis: Unique addresses Flushed"), // will be true if successfull
            console.log(`Redis: Adding ${addresses.length} Unique Addresses`),
            addresses.forEach( address => redis.set(address, 1) )
            callback()
        })
      })
  }

  if(config.fallback)
    async.series([
        populate_index,
        next => fallback_fast(state, next),
        next => fallback_slow(state, next),
      ],
      () => complete( null, state )
    )
  else
    console.log('Fallback: Skipping Fallback'),
    complete( null, state )
}
