const Sequelize    = require('sequelize'),
      Op           = Sequelize.Op,
      async        = require('async'),
      bn           = require('bignumber.js'),
      db           = require('./models')

      query = {}

// Wallet queries
query.wallets_bulk_upsert = ( wallets ) => {
  return db.Wallets.bulkCreate( wallets, { updateOnDuplicate: true })
}

query.address_uniques = ( block_begin, block_end, callback ) => {
  db.sequelize
    .query('SELECT `from` FROM `transfers` WHERE block_number>='+block_begin+' AND block_number<='+block_end+' UNION SELECT `to` FROM `transfers` WHERE block_number>='+block_begin+' AND block_number<='+block_end+' UNION SELECT `address` FROM `claims` WHERE block_number>='+block_begin+' AND block_number<='+block_end+' UNION SELECT `address` FROM `buys` WHERE block_number>='+block_begin+' AND block_number<='+block_end+' ', {type: db.sequelize.QueryTypes.SELECT})
    .then( results => {
      addresses = results.map( result => result.address || result.from || result.to  )
      callback( addresses )
    })
}

query.last_register = (address, begin, end, callback) => {
  db.Registrations
    .findAll({
      attributes: ['ultrain_key'],
      where: {
        address: address,
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      },
      order: [['block_number', 'DESC']],
      limit: 1
    })
    .then( results => callback( results.length ? results[0].dataValues.ultrain_key : null ) )
}

// Address qeuries
query.address_claims = (address, begin, end) => {
  return db.Claims
    .findAll({
      where : {
        address: address,
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

query.address_buys = (address, begin, end) => {
  return db.Buys
    .findAll({
      where : {
        address: address,
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

query.address_transfers_in = (address, begin, end) => {
  return db.Transfers
    .findAll({
      attributes: ['ultrain_amount'],
      where: {
        to: address,
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    }, {type: db.sequelize.QueryTypes.SELECT})
}

query.address_transfers_out = (address, begin, end) => {
  let db = require('./models')
  return db.Transfers
    .findAll({
      attributes: ['ultrain_amount'],
      where: {
        from: address,
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    }, {type: db.sequelize.QueryTypes.SELECT})
}

query.address_reclaimables = (address, begin, end) => {
  return db.Reclaimables
    .findAll({
      attributes: ['ultrain_amount'],
      where: {
        address: address,
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

//tx range queries
query.transfers_in_range = (begin, end) => {
  return db.Transfers
    .findAll({
      attributes: ['from', 'to'],
      where: {
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

query.registrations_in_range = (begin, end) => {
  return db.Registrations
    .findAll({
      attributes: ['address'],
      where: {
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

query.buys_in_range = (begin, end) => {
  return db.Buys
    .findAll({
      attributes: ['address'],
      where: {
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

query.claims_in_range = (begin, end) => {
  return db.Claims
    .findAll({
      attributes: ['address'],
      where: {
        [Op.and] : [
          {
            block_number: {
              [Op.gte] : begin
            }
          },
          {
            block_number: {
              [Op.lte] : end
            }
          }
        ]
      }
    })
}

module.exports = query
