module.exports = ( state, complete ) => {

  if(config.recalculate_wallets === true) {
    console.log('recalculate_wallets set to true, skipping contract sync')
    complete(null, state)
    return
  }

  const async          = require('async'),
        db             = require('../../models'),

        db_config    = {ignoreDuplicates: true}

  let   util           = require('../../utilities'),
        scanCollection = require('../../helpers/web3-collection'),
        bn             = require('bignumber.js'),
        Iterator       = require('../../classes/Iterator'),

        sync = {},
        log_intval,
        iterator

  state.sync_contracts = {
    buys:0,
    claims:0,
    registrations:0,
    transfers:0,
    reclaimables:0
  }

  const transfers = (iterator, next) => {
    scanCollection.transfers( iterator.from, iterator.to )
      .then( transfers => {
        let request = []
        if(transfers.length) {
          transfers.forEach( transfer => {
            request.push({
              tx_hash:      transfer.transactionHash,
              block_number: transfer.blockNumber,
              from:         transfer.returnValues.from,
              to:           transfer.returnValues.to,
              ultrain_amount:   new bn(transfer.returnValues.value).toFixed()
            })
          })
          state.sync_contracts.transfers+=request.length
          db.Transfers.bulkCreate( request )
            .then( () => { next() })
            .catch(console.log)
        } else {
          next()
        }
      })
  }

  const buys = (iterator, next) => {
    scanCollection.buys( iterator.from, iterator.to )
      .then( buys => {
        let request = []
        if(buys.length) {
          buys.forEach( buy => {
            request.push({
              tx_hash:      buy.transactionHash,
              block_number: buy.blockNumber,
              address:      buy.returnValues.user,
              period:       buy.returnValues.window,
              eth_amount:   new bn(buy.returnValues.amount).toFixed()
            })
          })
          state.sync_contracts.buys+=request.length
          db.Buys.bulkCreate( request )
            .then( () => { next() })
            .catch(console.log)
        } else {
          next()
        }
      })
  }

  const claims = (iterator, next) => {
    scanCollection.claims( iterator.from, iterator.to )
      .then( claims => {
        let request = []
        if(claims.length) {
          claims.forEach( claim => {
            request.push({
                tx_hash:      claim.transactionHash,
                block_number: claim.blockNumber,
                address:      claim.returnValues.user,
                period:       claim.returnValues.window,
                ultrain_amount:   new bn(claim.returnValues.amount).toFixed()
              })
          })
          state.sync_contracts.claims+=request.length
          db.Claims.bulkCreate( request )
            .then( () => { next() })
            .catch(console.log)
        } else {
          next()
        }
      })
  }

  const registrations = (iterator, next) => {
    scanCollection.registrations( iterator.from, iterator.to )
      .then( registrations => {
        let request = []
        if(registrations.length) {
          registrations.forEach( registration => {
            request.push({
              tx_hash:      registration.transactionHash,
              block_number: registration.blockNumber,
              address:      registration.returnValues.user,
              ultrain_key:      registration.returnValues.key
            })
          })
          state.sync_contracts.registrations+=request.length
          db.Registrations.bulkCreate( request, db_config )
            .then( () => { next() })
            .catch(console.log)
        } else {
          next()
        }
      })
  }

  const reclaimables = (iterator, next) => {
    scanCollection.reclaimables( iterator.from, iterator.to )
      .then( reclaimables => {
        let request = []
        if(reclaimables.length) {
          reclaimables.forEach( reclaimable => {
            let ultrain_amount = new bn(reclaimable.returnValues.value)
            if(ultrain_amount.gt(0)) {
              request.push({
                tx_hash:      reclaimable.transactionHash,
                block_number: reclaimable.blockNumber,
                address:      reclaimable.returnValues.from,
                ultrain_amount:   ultrain_amount.toFixed()
              })
            }
          });
          state.sync_contracts.reclaimables+=request.length
          db.Reclaimables.bulkCreate( request, db_config )
            .then( () => { next() })
            .catch(console.log)
        } else {
          next()
        }
      })
  }

  const log = (color, complete) => {
    const Table  = require('ascii-table'),
          colors = require('colors/safe')

    let   table

    if(complete)
      table = new Table(`Complete: ${state.block_begin} ~> ${state.block_end}`)
    else
      table = new Table(`${state.block_begin} ~> ${iterator.to}`)

    table.addRow('Transfers', state.sync_contracts.transfers)
    table.addRow('Buys', state.sync_contracts.buys)
    table.addRow('Claims', state.sync_contracts.claims)
    table.addRow('Registrations', state.sync_contracts.registrations)
    table.addRow('Reclaimables', state.sync_contracts.reclaimables)
    console.log(colors[color](table.setAlign(0, Table.RIGHT).setAlign(1, Table.LEFT).render()))
    console.log(colors.gray.italic(`Started: ${iterator.time_formatted().elapsed}, Average: ${iterator.time_formatted().average}`))
  }

  const log_periodically = () => {
    log_intval = setInterval( () => log('gray'), 30*1000 )
  }

  const sync_contracts = complete => {
    console.log(`Syncing Contract State between block #${state.block_begin} & ${state.block_end}`)

    const sync_txs = iterator => {
      async.series([
          next => transfers( iterator, next ),
          next => buys( iterator, next ),
          next => claims( iterator, next ),
          next => registrations( iterator, next ),
          next => reclaimables( iterator, next )
      ], result => {
        if(iterator.finish)
          console.log('finished'),
          iterator.onComplete()
        else
          iterator.next()
      })
    }

    const sync_options = {
      from: state.block_begin,
      max: state.block_end,
      increment: 100, //only scan 100 blocks at a time, if there's too many transactions per block it will cause memory heap issues. Ethereum wasn't built to be queried, it was built to be synced (kind of)
      onComplete: (err, res) => {
        clearInterval(log_intval)
        log('green', true)
        setTimeout(complete, 5000)
      }
    }

    iterator = new Iterator( sync_txs, sync_options )

    console.log(`Syncing Contracts between block #${state.block_begin} and #${state.block_end}, this may take a while.`)
    log_periodically()

    iterator.iterate()
  }

  sync_contracts( () => {
    complete( null, state )
  })

}
