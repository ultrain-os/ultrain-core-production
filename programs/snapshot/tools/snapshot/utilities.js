Number.prototype.between = function (min, max) {
  return this >= min && this <= max;
}

module.exports = {
  period:   require('./utilities/periods.js'),
  block:    require('./utilities/blocks.js'),
  misc:     require('./utilities/misc.js'),
  balance:  require('./utilities/balances.js')
}
