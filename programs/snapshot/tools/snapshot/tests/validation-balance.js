// count(negative-balances) > 0 (fail)
let db         = require('../models')

module.exports = ( state, callback ) => {
  db.sequelize
    .query(`SELECT address FROM wallets WHERE balance_wallet>=1 and register_error="balance_insufficient"`, {type: db.sequelize.QueryTypes.SELECT})
    .then(anomalies => {
      if(anomalies.length)
        state.tests.validation_balance = `Found ${anomalies.length} wallets that should not have an insufficient balance error!!! [SELECT address FROM wallets WHERE balance_wallet>=1 and register_error="balance_insufficient"]`
      else
        state.tests.validation_balance = true

      callback(null, state)
    })
}
