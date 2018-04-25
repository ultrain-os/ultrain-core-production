module.exports = ( config, callback ) => {

  const async        = require('async')
  const json_to_csv  = require('json-to-csv');
  const fs           = require('fs')

  const db           = require('../../models')

  const csv = callback => {
    db.Snapshot
      .findAll({ order: [ ['balance', 'DESC'] ] })
      .then( results => {
        let _results = results.map( result => {
          return {user: result.dataValues.user, key: result.dataValues.key, balance: result.dataValues.balance}
        })
        // TODO Move file/directory variables and functions somewhere else.
        json_to_csv(_results, config.path_csv, false)
          .then(() => {
            console.log(`${results.length} Records Saved to CSV`)
            if(config.overwrite_snapshot) fs.createReadStream(config.path_csv).pipe(fs.createWriteStream(config.file_csv))
            callback()
          })
          .catch( error => { throw new Error(error) })
      })
      .catch( error => { throw new Error(error) })
  }

  db.sequelize
    .query('INSERT INTO `snapshot` (`user`, `key`, `balance`) SELECT `address`, `ultrain_key`, `balance_total` FROM `wallets` WHERE `valid`=1 ORDER BY `balance_total` DESC')
    .then(results => {
      console.log( 'Snapshot Table Synced' )
      csv( callback )
    })
    .catch( error => { throw new Error(error) })

}
