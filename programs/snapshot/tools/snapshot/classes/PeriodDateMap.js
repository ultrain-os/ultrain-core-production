let util = require('../utilities')

class PeriodDateMap {

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
    let periodLength = 60*60*23

    this.map = new Array(351).fill({})
    this.map.forEach( (value, index) => {
      let begin, end

      if(index == 0) {
        begin = CS_OPEN_TIME
      }
      else if (index == 1) {
        begin = CS_START_TIME
      }
      else {
        begin = CS_START_TIME + periodLength * (index-1)
      }
      end = begin + periodLength - 1

      this.map[index] = {begin: begin, end: end}

    })

    onComplete( this )
  }

}

module.exports = PeriodDateMap
