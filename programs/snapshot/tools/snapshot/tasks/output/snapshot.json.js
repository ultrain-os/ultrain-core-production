module.exports = ( state, complete ) => {

  let async     = require('async')
  let fs        = require('fs')
  let bn        = require('bignumber.js')
  let checksum  = require('checksum')
  let md5       = require('md5')
  let Sequelize = require('sequelize')
  let Op        = Sequelize.Op

  let db        = require('../../models')

  let data = {
    parameters : {},
    accounts: {},
    supply: {},
    meta: {},
    checksum: {
      output: {},
      db: {}
    },
    stats: {}
  }

  //TODO move analytics functions to own module for reuse in tests.

  let get_state_variables = callback => {
    data.parameters = {
      period:                config.period,
      block_begin:           state.block_begin,
      block_end:             state.block_end,
      b1_dist:               config.include_b1
    }
    data.meta.author = config.author
    data.meta.timestamp_started = state.started
    callback()
  }

  let get_accounts_total = callback => {
    db.Snapshot
      .count()
      .then( total => {
        data.accounts.valid = total
        callback()
      })
      .catch( error => { throw new Error(error) })
  }

  let get_supply_expected = callback => {
    data.supply.expected = 200000000+(config.period*2000000)
    if(config.include_b1) data.supply.expected += 100000000
    callback()
  }

  let get_supply_included = callback => {
    db.Snapshot
      .sum('balance')
      .then( fdsfdfs => {
        data.supply.included = fdsfdfs
        data.supply.percent_included = `${data.supply.included/data.supply.expected*100 | 0}%`
        callback()
      })
      .catch( error => { throw new Error(error) })
  }

  let get_supply_liquid = callback => {
    //Sequelize giving me trouble, reverted to raw queries.
    let query = `SELECT sum(balance_total) FROM wallets WHERE address!="${CS_ADDRESS_TOKEN}" AND address!="${CS_ADDRESS_CROWDSALE}"`
    if(!config.include_b1)
      query = `${query} AND address!="${CS_ADDRESS_B1}"`

    db.sequelize
      .query(query, {type: db.sequelize.QueryTypes.SELECT})
      .then( sum => {
        data.supply.liquid = parseFloat(sum[0]['sum(balance_total)'])
        data.supply.margin_of_error = `${new bn(100).minus(new bn(data.supply.liquid).div(new bn(data.supply.expected)).times(100)).toFixed(16)}%`
        callback()
      })
      .catch( error => { throw new Error(error) })
  }

  let get_supply_total = callback => {
    let query = `SELECT sum(balance_total) FROM wallets`
    db.sequelize
      .query(query, {type: db.sequelize.QueryTypes.SELECT})
      .then( sum => {
        data.supply.total = parseFloat(sum[0]['sum(balance_total)'])
        callback()
      })
      .catch( error => { throw new Error(error) })
  }

  let get_accounts_registered = callback => {
    db.Wallets
      .count({
        where: { registered: true, valid: true  }
      })
      .then( count => {
        data.accounts.registered = count
        data.accounts.registered_included_percent = new String(data.accounts.registered/data.accounts.valid*100)+"%"
        callback()
      })
      .catch( error => { throw new Error(error) })
  }

  let get_accounts_fallback = (callback) => {
    db.Wallets
      .count({
        where: { fallback: true, valid: true }
      })
      .then( count => {
        data.accounts.fallback = count
        data.accounts.fallback_included_percent = new String(data.accounts.fallback/data.accounts.valid*100)+"%"
        callback()
      })
      .catch( error => { throw new Error(error) })
  }

  let get_snapshot_checksum = callback => {
    checksum.file('./snapshot.csv', (err, sum) => {
      if(err)
        throw new Error(err)
      else
        data.checksum.output.snapshot = sum
      callback()
    })
  }

  let get_table_checksum = callback => {
    console.log(`Generating DB Table Checksums`)
    db.sequelize
      .query('CHECKSUM TABLE wallets, buys, claims, registrations, transfers, snapshot')
      .then( results => {
        results[0].forEach( result => {
          let key = result.Table.split('.')[1]
          data.checksum.db[key] = checksum(result.Checksum.toString())
        })
        callback()
      })
  }

  let get_timestamp = callback => {
    data.meta.timestamp_completed = (Date.now() / 1000 | 0)
    callback()
  }

  let get_time_elapsed = callback => {
    let prettyMs = require('pretty-ms');
    if(data.meta.timestamp_started && data.meta.timestamp_completed) data.meta.total_time = prettyMs( (data.meta.timestamp_completed-data.meta.timestamp_started)*1000 )
    callback()
  }

  let get_state_stats = callback => {
    data.stats.contracts = state.sync_contracts
    callback()
  }

  let output = callback => {
    fs.writeFile(state.files.path_json, JSON.stringify(data, null, "\t"), (err) => {
      console.log("Snapshot meta written to snapshot.json");
      if(config.overwrite_snapshot) fs.createReadStream(state.files.path_json).pipe(fs.createWriteStream(state.files.file_json))
      callback()
    });
  }

  console.log('Generating snapshot.json')
  async.series([
    get_state_variables,
    get_accounts_total,
    get_accounts_registered,
    get_accounts_fallback,
    get_supply_total,
    get_supply_expected,
    get_supply_included,
    get_supply_liquid,
    get_snapshot_checksum,
    get_table_checksum,
    get_timestamp,
    get_time_elapsed,
    get_state_stats,
    output,
  ], () => complete(null, state) )

}
