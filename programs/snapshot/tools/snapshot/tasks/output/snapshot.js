module.exports = ( state, complete ) => {

  const series    = require('async').series
  const Snapshot  = require('../../models').Snapshot

  const mkdir = (config, callback) => {
    const fs = require('fs')
    if (!fs.existsSync(config.data_dir))
      fs.mkdirSync(config.data_dir)
    if (!fs.existsSync(config.dir))
      fs.mkdirSync(config.dir)
    callback()
  }

  const csv = (config, callback) => {
    const output = require('./snapshot.csv')
    output(config, callback)
  }

  const json = (config, callback) => {
    const output = require('./snapshot.json')
    output(config, callback)
  }

  const get_ss_fs = (state) => {
    ss_fs_config = {}
    ss_fs_config.file_csv = 'snapshot.csv'
    ss_fs_config.file_json = 'snapshot.json'
    ss_fs_config.data_dir = './data'
    ss_fs_config.dir = `${ss_fs_config.data_dir}/${config.period}`
    ss_fs_config.path_csv = `${ss_fs_config.dir}/${ss_fs_config.file_csv}`
    ss_fs_config.path_json = `${ss_fs_config.dir}/${ss_fs_config.file_json}`
    return ss_fs_config
  }

  state.files = get_ss_fs( state )
  series([
    next => Snapshot.destroy({ truncate : true, cascade: false }).then(next),
    next => mkdir(state.files, next),
    next => csv(state.files, next),  //dumps snapshot table into csv
    next => json(state, next)  //outputs some metadata about the snapshot
  ], () => complete( null, state ) )

}
