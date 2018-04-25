let BigNumber   = require('bignumber.js')

class Balance {

  constructor(){
    this.wallet    = new BigNumber(0)
    this.unclaimed = new BigNumber(0)
    this.reclaimed = new BigNumber(0)
    this.total     = new BigNumber(0)
  }

  set( type, balance ){
    this[ type ] = balance
  }

  exists( type ){
    return (typeof this[ type ] !== "undefined" && this[type].gt( 0 ))
  }

  get( type ){
    return (typeof this[ type ] !== "undefined" && this[type].gt( 0 )) ? this[type] : false;
  }

  sum(){
    this.total = new BigNumber(this.wallet).plus( new BigNumber(this.unclaimed) ).plus( new BigNumber(this.reclaimed) )
  }

  from_wei(){
    //Save dust, convert last
    this.total      = this.total.div(WAD)
    this.wallet     = this.wallet.div(WAD)
    this.unclaimed  = this.unclaimed.div(WAD)
    this.reclaimed  = this.reclaimed.div(WAD)
  }

  format(){
    this.total      = this.total.toFixed(4)
    this.wallet     = this.wallet.toFixed(4)
    this.unclaimed  = this.unclaimed.toFixed(4)
    this.reclaimed  = this.reclaimed.toFixed(4)
  }

}

module.exports = Balance
