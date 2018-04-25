var genesis_config

$( function(){
  $('input#date-time').datetimepicker({format: 'y-m-dTh:m:s'});

  $('input#config-file')
    .on('click', function(){
      this.value = null
    })
    .on('change', function(upload_event){
      console.log(upload_event)
      read_genesis_config( upload_event, function( json) {
        if(validate_json( json ))
          console.log(json),
          load_from_config( json )
        else
          alert('Genesis Config File Malformed')
      })
    })

  $('button#export_genesis_config')
    .on('click', function(e){
      e.preventDefault()
      export_json( to_genesis_options_object( $(this).parents('form:first') ), 'genesis-config')
    })

  $('form')
    .on('submit', function(e){
      e.preventDefault()
      var genesis = new Genesis( SNAPSHOT )
      genesis.config( to_genesis_options_object( this ) )
      genesis.build()
      download_genesis_json( genesis.json )
    })

})

var read_genesis_config = function( upload_event, callback ){

  var reader = new FileReader();
  reader.readAsText(upload_event.target.files[0], "UTF-8");
  reader.onload = function (e) {
      callback(e.target.result)
  }
  reader.onerror = function (e) {
      alert('Could not load file')
  }
}

var validate_json = function( json ) {
  if (/^[\],:{}\s]*$/.test(json.replace(/\\["\\\/bfnrtu]/g, '@').
    replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g, ']').
    replace(/(?:^|:|,)(?:\s*\[)+/g, ''))) {
      return true
    }else{
      return false
    }
}

var to_genesis_options_object = function( form ) {
  var obj = {};
  $(form).find('.form-group').each( function(key, value){
    var $group = $(this),
        group = $group.data('key')
    //
    if($group.find('.repeatables').length) {
      obj[group] = []
      $('.repeatables', $group).find('.repeated').each( function(key, value){
        var _obj = {},
            members = 0

        $(this).find('input').each(function(key, value){
          if($(this).val().length) members++
          _obj[$(this).data('key')] = $(this).val()
        })
        if(members == $(this).find('input').length) obj[group].push(_obj)
      })
    } else if( $group.data('single') == "yes" ) {
      var $field = $group.find('input:first')
      var value = $field.val()
      if(value.length > 0) obj[group] = value
    } else {
      obj[group] = {}
      $group.find('input').each(function(key, value){
        if($(this).val().length) obj[group][$(this).data('key')] = $(this).val()
      })
    }
  })
  return obj;
}

var load_from_config = function( config ){
  if(typeof config === "string") config = JSON.parse(config)
  load_additional_accounts(config)
  load_initial_producers(config)
  load_initial_parameters(config)
  load_initial_timestamp(config)
}

var load_additional_accounts = function(config){
  $.each( config.additional_accounts, function(){
    var account = this
    $('input.add-account').trigger('click')
    $group = $('.additional-accounts.repeatables .repeated:last')
    $group.find('input[data-key=name]').attr('value', account.name )
    $group.find('input[data-key=owner_key]').attr('value', account.owner_key )
    $group.find('input[data-key=active_key]').attr('value', account.active_key )
    $group.find('input[data-key=liquid_balance]').attr('value', account.liquid_balance )
  })
}
var load_initial_producers = function(config){
  $.each( config.initial_producers, function( index, value ){
    var producer = value
    $group = $('.initial-producers.repeatables .repeated').eq(index)
    $group.find('input[data-key=owner_name]').attr('value', producer.owner_name )
    $group.find('input[data-key=block_signing_key]').attr('value', producer.block_signing_key )
  })
}
var load_initial_parameters = function(config){
  var $group = $('.fields.initial-parameters')
  for (var key in config.initial_parameters) {
    if (config.initial_parameters.hasOwnProperty(key)) {
      $group.find('input[name='+key+']').attr('value', config.initial_parameters[key])
    }
  }
}

var load_initial_timestamp = function(config){
  if(config.initial_timestamp && config.initial_timestamp.length) $('input[name=initial_timestamp]').attr('value', config.initial_timestamp)
}

// CONFIG = {
// 	"initial_producers": [
// 		{
// 			"owner_name": "inita",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initb",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initc",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initd",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "inite",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initf",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initg",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "inith",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initi",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initj",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initk",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initl",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initm",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initn",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "inito",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initp",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initq",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initr",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "inits",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initt",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		},
// 		{
// 			"owner_name": "initu",
// 			"block_signing_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
// 		}
// 	],
// 	"additional_accounts": [
// 		{
// 			"name": "inita",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initb",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initc",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initd",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "inite",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initf",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initg",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "inith",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initi",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initj",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initk",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initl",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initm",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initn",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
//     {
//       "name": "inito",
//       "owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
//       "active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
//       "liquid_balance": "1000000.0000"
//     },
// 		{
// 			"name": "initp",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initq",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initr",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "inits",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initt",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "initu",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "1000000.0000"
// 		},
// 		{
// 			"name": "faucet",
// 			"owner_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"active_key": "UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
// 			"liquid_balance": "90000000.0000"
// 		}
// 	]
// }
