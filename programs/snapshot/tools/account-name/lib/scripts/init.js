$( function(){
  $('form').on('submit', function(e){
    e.preventDefault()
    lookup( $(this).find('input:text').val() )
  })
  $('a.reset').on('click', reset)
})

const lookup = function( value ){

    let found = -1

    if(startsWithETH(value)) {
      found = SNAPSHOT.findIndex( function(row){ return row.eth.toLowerCase() == value.toLowerCase() } )
    }
    else if(startsWithULTRAIN(value)) {
      found = SNAPSHOT.filter( function(row){ return row.ultrain == value } )
    }
    else { display_invalid( value ) }

    if(typeof found == "object" && found.length) {
      if(found.length > 1) { display_account_names( found ) }
      else                 { display_account_name( SNAPSHOT.findIndex( function(row){ return row.eth == found[0].eth } ) ) }
    }
    else if(typeof found == "number"  && found > -1) {
      display_account_name( found )
    }
    else {
      display_not_found( value )
    }

}

const display_account_name = function(row_num) {
  $('body').attr('id','result')
  $('.result').find('span').html( account_name( row_num ) )
}

const display_account_names = function( row_nums ) {
  $('body').attr('id','result')
  let total = row_nums.length
  let string = ""
  row_nums.forEach( function( value, index ){
    string = string + account_name( SNAPSHOT.findIndex( function(row){ return row.eth == value.eth } ) )
    if(index+1 < total) string = string + ", "
    })
  $('.result').html(`Your account names are <span>${string}</strong>`)
}

const display_not_found = function( value ){
  $('body').attr('id','error-not-found')
  $('div.error-not-found span').html(value)
}

const display_invalid = function( value ){
  $('body').attr('id','error-invalid')
  $('span.error-invalid').find('span').text(value)
}

const account_name = function(row_num){
  return base32.encode( row_num.toString() ).replace(/=/g, "").toLowerCase()
}

const reset = function() {
  $('body').attr('id', 'ready')
}

const startsWithETH = (query) => {
  var searchPattern = new RegExp('0x', 'i');
  return searchPattern.test(query)
}

const startsWithULTRAIN = (query) => {
  var searchPattern = new RegExp('ULTRAIN', 'i');
  return searchPattern.test(query)
}

const getAllIndexes = (arr, val) => {
    var indexes = [], i = -1;
    while ((i = arr.indexOf(val, i+1)) != -1){
        indexes.push(i);
    }
    return indexes;
}
