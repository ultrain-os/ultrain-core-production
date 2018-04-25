module.exports = ( state, complete ) => {

  const async     = require('async'),
        db        = require('../../models'),
        Table     = require('ascii-table'),
        util        = require('../../utilities')

  let   block_index = 46147, //first ethereum block with txs
        blocks_total = 0,
        pks_found = 0,
        blocks_processed = 0,
        status_log_intval = false,
        fallback = {}

  const tx_hash_from_query_results = ( results, callback ) => (results.length) ? callback( results[0].dataValues.tx_hash ) : callback( false )
  const pubkey_from_tx_hash        = ( tx_hash, callback ) => util.misc.get_tx(tx_hash).then( result => { callback( result.publicKey ) })

  //TODO Fix callback hell
  const iterate_blocks = ( callback ) => {
    const iterate = () => iterate_block_txs( iterate, callback )
    iterate()
  }

  const iterate_block_txs = (next_block, finished) => {
    if(block_index > state.block_end) {
      finished()
      return
    }
    try {
      web3.eth
        .getBlock( block_index, true )
        .then( block => {
          async.each(
            block.transactions,
            (tx, next_tx) => check_tx(tx, next_tx),
            (err) => {
              if(err)
                throw new Error(err)
              else
                block_index++,
                blocks_processed++,
                next_block()
            }
          )
        })
    }
    catch(e) {
      throw new Error(e)
    }
  }

  //Check TX
  const check_tx = (tx, next_tx) => {
    // console.log(tx.blockNumber, tx.hash)
    try {
      redis.get(tx.from, (err, reply) => {
        if(reply)
          update_address_key(tx.from, tx.hash, next_tx)
        else
          next_tx()
      });
    }
    catch(e) {
      throw new Error(e)
    }
  }

  const update_address_key = (address, tx_hash, next_tx) => {
    pubkey_from_tx_hash(tx_hash, pubkey => {
      const ultrain_key = util.misc.convert_ethpk_to_ultrainpk(pubkey)
      if(util.misc.is_ultrain_public_key(ultrain_key))
        db.Wallets
          .update({
            ultrain_key: ultrain_key,
            fallback: true,
            fallback_error: null,
            valid: true
          },
          {
            where : {
              address: address
            }
          })
          .then( () => {
            if(config.cache)
              db.Keys.upsert({ address: address, tx_hash: tx_hash, public_key: pubkey, derived_ultrain_key: ultrain_key })
            // console.log(`ULTRAIN Key Generated: #${block_index} => ${tx_hash} => ${address} => ${pubkey} => ${ultrain_key}`)
            redis.del(address);
            pks_found++
            next_tx()
          })
      else
        next_tx()
    })
  }

  const status_log = () => {
    blocks_total = state.block_end-block_index
    status_log_intval = setInterval( () => {
      redis.keys('*', (err, addresses) => {

        let table = new Table(`Slow Fallback`),
            success_rate = Math.floor(pks_found/(addresses.length+pks_found)*100)

        table.addRow('Progress', `${Math.floor(blocks_processed/blocks_total*100)}%` )
        table.addRow('PKs Found (slow)', pks_found)
        table.addRow('PKs Unfound (slow)', addresses.length)
        table.addRow('Found Rate (slow)', `${success_rate}%`)
        table.addRow('Current Block', block_index)
        table.addRow('Syncing to Block', state.block_end)
        table.addRow(`Total Checked`, blocks_processed)
        table.addRow(`Total Blocks`, blocks_total)
        console.log(table.setAlign(0, Table.RIGHT).setAlign(1, Table.LEFT).render())

      })
    }, 60000)
  }

  console.log('Fallback: Slow')
  status_log( state )
  iterate_blocks( () => { complete(null, state) } )

}
