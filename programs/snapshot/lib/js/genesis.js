const SNAPSHOT_FILE = '/snapshot.csv'
let   snapshot = null
      genesis = null

window.onload = () => init()

const init = () => { document.body.id = "loading", generate_genesis() }

const generate_genesis = () => {
  if( check_url(SNAPSHOT_FILE) )
    snapshot_load( SNAPSHOT_FILE, response => {
      if(response)
        snapshot = snapshot_csv_to_array(response),
        snapshot.shift(),
        genesis = new Genesis( snapshot ).json,
        setTimeout( () => document.body.id = "ready", 1000 )
    })
  else 
    document.body.id = "error"
}

const snapshot_csv_to_array = (csv) => {
  let rows = csv.split(/\r\n|\n/)
  let output = []

  for (var i=0; i<rows.length; i++) {
    let _registrant
        ,registrant = rows[i].split(',')
        ,[eth, ultrain, balance] = registrant
    if(eth && ultrain && balance)
      _registrant = { eth: eth, ultrain: ultrain, balance: balance },
      output.push(_registrant)
  }
  return output
}

const snapshot_load = (filename, callback) => {   
    var xobj = new XMLHttpRequest()
        xobj.overrideMimeType("application/json")
    xobj.open('GET', filename, true); // Replace 'my_data' with the path to your file
    xobj.onreadystatechange = function () {
          if (xobj.readyState == 4 && xobj.status == "200") {
            // Required use of an anonymous callback as .open will NOT return a value but simply returns undefined in asynchronous mode
            callback(xobj.responseText)
          }
    };
    xobj.send(null);  
 }

const check_url = (uri) => {
  let http
  if (window.XMLHttpRequest)
    http = new XMLHttpRequest()
  else
    http = new ActiveXObject("Microsoft.XMLHTTP")
  http.open('HEAD', uri, false)
  http.send()
  return http.status!=404
}

const bind_snapshot_upload = () => {
  document.forms['snapshot-upload'].elements['snapshot-file'].onchange = function(event) {
    if(!window.FileReader) return // Browser is not compatible
    let reader = new FileReader()
    reader.onload = function(event) {
      if(event.target.readyState != 2) return
      if(event.target.error) {
          alert('Error while reading file')
          return
      }
      genesis = event.target.result
    };

    reader.readAsText(event.target.files[0])
  };
}