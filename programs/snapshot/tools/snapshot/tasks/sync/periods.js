module.exports = ( state, complete )  => {

  const async           = require('async'),
        db              = require('../../models')

  const get_period_map = (callback = () => {}) => {
    db.State
      .findAll({
        where: {
          meta_key: 'period_block_map'
        },
        attributes: ['meta_value']
      })
      .then( result => {
        let map
        if(result.length)
          map = JSON.parse( result[0].dataValues.meta_value )
        else {
          map = []
        }
        callback(map)
      })
  }

  get_period_map( map => {
    const PeriodMap = require('../../classes/PeriodMap')
    let periods = new PeriodMap(map)
    if(periods.syncedToPeriod() < config.period ) {
      periods.periodMax = config.period
      periods.generate( result => {
        db.State
          .upsert({meta_key: 'period_block_map', meta_value: JSON.stringify(periods.map)})
          .then( () => {
            state.period_map          = periods.map
            state.block_begin         = periods.map[0].begin
            state.block_end           = periods.map[config.period].end
            complete( null, state )
          })
      })
    }
    else {
      state.period_map          = periods.map
      state.block_begin         = periods.map[0].begin
      state.block_end           = periods.map[config.period].end
      complete( null, state  )
    }
  })

}
