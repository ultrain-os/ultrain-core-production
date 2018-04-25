// [sum(buys.eth_amount) == utility_contract.daily_buys]
// This verifies correct synchronization between contract and db, if this were wrong, it would not be noticed in supply levels, but would instead result in inaccurate user *unclaimed* balances in snapshot

module.exports = (state, callback) => {
  const {series} = require('async')

  let contract = require('../helpers/web3-contract'),
      db         = require('../models'),
      checksum   = require('checksum'),

      buys = {}

  const buys_from_contract = callback => {
    contract.$utility.methods
      .dailyTotals()
      .call()
      .then( periods => {
        periods.length = config.period+1
        buys.contract = checksum(periods.toString())
        callback()
      })
  }

  const buys_from_db = callback => {
    db.sequelize
      .query(`SELECT sum(eth_amount) FROM buys WHERE period<=${config.period} GROUP BY period ORDER BY period ASC`, {type: db.sequelize.QueryTypes.SELECT})
      .then(periods => {
        periods = periods.map( period => period['sum(eth_amount)'] )
        periods.length = config.period+1
        buys.db = checksum(periods.toString())
        callback()
      })
  }

  const test = callback => {
    buys.contract === buys.db ? callback() : callback(`${buys.contract} != ${buys.db}`)
  }

  series([
    buys_from_contract,
    buys_from_db,
    test
  ], (error) => {
    if(error)
      state.tests.daily_buys = error
    else
      state.tests.daily_buys = true

    callback(null, state)
  })
}
