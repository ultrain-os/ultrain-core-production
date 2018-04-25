let util = require('../utilities')

class PeriodMap {

  constructor( periodMap ){
    this.map = periodMap || []
    this.periodMax = false
  }

  beginBlock(){
    return this.map.length ? this.map[0].begin : false
  }

  endBlock( max ){
    if(max)
      return this.map.length ? this.map[max].end : false
    else
      return this.map.length ? this.map[this.map.length-1].end : false
  }

  syncedToPeriod(){
    return this.map.length-1
  }

  generate( onComplete = () => {} ){

    let map             = this.map,
        period_max      = this.periodMax ? this.periodMax : util.period.last_closed(),
        period_today    = util.period.from_date( Date.now()/1000 | 0 ),
        block_index     = map.length ? map[map.length-1].end+1 : CS_BLOCK_FIRST,
        block_cache     = null,
        period_index    = null,
        period_cache    = null

    const message = () => {
      console.log(`---------------------`)
      console.log(`INFO: Period Map needs to be Updated - ${this.map.length > 0 ? this.map.length-1 : 'not synced'}, syncing to Period  (Today is Period ${period_today})`)
      console.log(`Generating new row(s) for Period Block Map (Up to Period ${period_max})`)
      console.log(`This will take a little while`)
      console.log(`---------------------`)
    }

    // This is why we can't have nice things
    // log parameters should indexable by default :|
    const iterate = () => {
      util.block.get( block_index, block => {
          period_index = util.period.from_date( block.timestamp )
          if(period_cache === null) period_cache = period_index

          if(period_index != period_cache)
            map[period_cache].end = block_cache,
            console.log(`Added period ${map.length-1} to the Period Map :)`),
            console.log(map[map.length-1])

          if(typeof map[period_index] !== "object" && period_index <= period_max)
            map.push({ begin: block_index, end: null })

          block_cache  = parseInt(block_index)
          period_cache = parseInt(period_index)
          block_index++
          ( period_cache <= period_max ) ? setTimeout(iterate, 1) : (this.map = map, onComplete( this ))
        })
    }

    message()
    iterate()
  }

}

module.exports = PeriodMap
