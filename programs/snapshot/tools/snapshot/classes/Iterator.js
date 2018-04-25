class Iterator  {
  constructor( fn, options ){

    this.increment = 1
    this.from       = 0
    this.max        = 1

    //Fn to run
    this.fn         = fn || ( () => {} )

    //Arbitrary Storage
    this.args       = {}
    this.data       = null
    this.cache      = null

    //Callback
    this.onComplete = ( () => {} )

    Object.assign(this, options)

    this.finish     = false
    this.pass       = 0

    //Stats
    this.clock      = true
    this.start      = null
    this.now        = null
    this.elapsed    = null
    this.avg        = 0

    //Setup
    this.incrementTo()
  }

  incrementFrom(){
    this.from = this.from+this.increment
  }

  incrementTo(){
    this.to = this.from+this.increment-1
  }

  next(){
    this.incrementFrom()
    this.incrementTo()
    this.iterate()
  }

  check(){
    let max_to = this.to >= this.max
    if(max_to)
      this.finish = true,
      this.to = this.max

    let max_from = this.from >= this.max
    if(max_from)
      this.finish = true,
      this.from = this.max
  }

  iterate(){
    this.check()
    this.fn( this )
    this.pass++
    if(this.clock)
      this.time_now(),
      this.time_elapsed(),
      this.time_average()
  }

  time_now(){
    if (this.start==null)
      this.start = Date.now() | 0
    this.now = Date.now() | 0
  }

  time_elapsed(){
    this.elapsed = this.now-this.start
  }

  time_average(){
    this.avg = this.elapsed/this.pass
  }

  time_formatted(){
    return {
      average: this.avg<1000
        ? `${(this.avg).toFixed(2).toString()} ms/pass`
        : this.avg<60*1000
          ? `${((this.avg)/1000).toFixed(2).toString()} seconds per pass`
          : `${((this.avg)/1000/60).toFixed(2).toString()} minutes per pass`,
      elapsed: this.elapsed<60*1000
        ? `${((this.elapsed)/1000).toFixed(2).toString()} seconds ago`
        : this.elapsed<60*60*1000
          ? `${((this.elapsed)/1000/60).toFixed(2).toString()} minutes ago`
          : `${((this.elapsed)/1000/60/60).toFixed(2).toString()} hours ago`
    }
  }

}

module.exports = Iterator
