const Sequelize = require('sequelize')

module.exports = (db, user, pass, host, port) => {
  return new Sequelize(
    db,
    user,
    pass,
    {
      host: host,
      port: port,
      dialect: "mysql",
      logging:false
    }
  )
}
