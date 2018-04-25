const SNAPSHOT_FILE = 'https://raw.githubusercontent.com/ULTRAINIO/genesis/master/snapshot.csv';
let snapshot = null;
genesis = null;

window.onload = () => init();

const init = () => {
  document.body.id = "loading", snapshot_load();
};

const snapshot_load = () => {
  if (check_url(SNAPSHOT_FILE)) snapshot_load_csv(SNAPSHOT_FILE, response => {
    if (response) SNAPSHOT = snapshot_csv_to_array(response), setTimeout(() => document.body.id = "ready", 2500);
  });else document.body.id = "error";
};

const snapshot_csv_to_array = csv => {
  let rows = csv.split(/\r\n|\n/);
  let output = [];

  for (var i = 0; i < rows.length; i++) {
    let _registrant,
        registrant = rows[i].replace(/"/g, '').split(','),
        [eth, ultrain, balance] = registrant;
    if (eth && ultrain && balance) _registrant = { eth: eth, ultrain: ultrain, balance: balance }, output.push(_registrant);
  }
  return output;
};

const snapshot_load_csv = (filename, callback) => {
  var xobj = new XMLHttpRequest();
  xobj.overrideMimeType("application/json");
  xobj.open('GET', filename, true); // Replace 'my_data' with the path to your file
  xobj.onreadystatechange = function () {
    if (xobj.readyState == 4 && xobj.status == "200") {
      // Required use of an anonymous callback as .open will NOT return a value but simply returns undefined in asynchronous mode
      callback(xobj.responseText);
    }
  };
  xobj.send(null);
};

const check_url = uri => {
  let http;
  if (window.XMLHttpRequest) http = new XMLHttpRequest();else http = new ActiveXObject("Microsoft.XMLHTTP");
  http.open('HEAD', uri, false);
  http.send();
  return http.status != 404;
};

const bind_snapshot_upload = () => {
  document.forms['snapshot-upload'].elements['snapshot-file'].onchange = function (event) {
    if (!window.FileReader) return; // Browser is not compatible
    let reader = new FileReader();
    reader.onload = function (event) {
      if (event.target.readyState != 2) return;
      if (event.target.error) {
        alert('Error while reading file');
        return;
      }
      genesis = event.target.result;
    };
    reader.readAsText(event.target.files[0]);
  };
};
class Genesis {

  constructor(snapshot) {
    this.json = {};
    this.snapshot = snapshot;
    this.options;
  }

  config(options) {
    this.options = options;
  }

  build(args) {

    this.timestamp();
    this.initial_parameters();
    this.immutable_parameters();
    this.accounts();
    this.chainid();
  }

  accounts() {
    this.load();
    this.format();
    this.append();
    this.index();
    this.encode();
  }

  load() {
    this.load_initial_producers();
    this.load_initial_accounts();
  }

  append() {
    this.append_to_initial_accounts();
  }

  format() {
    this.format_initial_accounts();
  }

  index() {
    this.index_initial_producers();
  }

  encode() {
    this.encode_initial_producers();
    this.encode_initial_accounts();
  }

  load_initial_producers() {
    this.json.initial_producers = this.options.initial_producers || [];
  }

  load_initial_accounts() {
    this.json.initial_accounts = this.snapshot;
  }

  //Format account object from snapshot to format and member names suitable for genesis
  format_initial_accounts() {
    this.json.initial_accounts = this.json.initial_accounts.map(account => {
      return {
        "name": account.eth.replace(/"/g, ''),
        "owner_key": account.ultrain.replace(/"/g, ''),
        "active_key": account.ultrain.replace(/"/g, ''),
        "liquid_balance": `${account.balance.replace(/"/g, '')} ULTRAIN`
      };
    });
  }

  //Aggregate the index for producers so a consistent base32 encoded username can be generated. Throw error if no matching account can be found.
  index_initial_producers() {
    this.json.initial_producers.forEach((producer, producer_index) => {
      this.json.initial_accounts.forEach((account, account_index) => {
        if (account.name == producer.owner_name) this.json.initial_producers[producer_index].index = account_index;
      });
      if (typeof this.json.initial_producers[producer_index].index === "undefined") throw new Error(`Initial Producer ${producer.owner_name} Was Not Found in Initial Accounts`);
    });
  }

  //Encode the initial producers with base32 (if the owner_name is NOT an ethereum address.)
  encode_initial_producers() {
    this.json.initial_producers = this.json.initial_producers.map(producer => {
      console.log(producer);
      producer.owner_name = generate_account_name(producer.owner_name.replace(/"/g, ''), producer.index);
      delete producer.index;
      return producer;
    });
  }

  //Encode the initial accounts with base32 (if the owner_name is NOT an ethereum address.)
  encode_initial_accounts() {
    this.json.initial_accounts = this.json.initial_accounts.map((account, index) => {
      if (typeof account !== "undefined") {
        account.name = generate_account_name(account.name, index);
        return account;
      } else {
        return null;
      }
    });
  }

  append_to_initial_accounts() {
    this.options.additional_accounts.map(account => {
      account.liquid_balance = `${account.liquid_balance} ULTRAIN`;
      return account;
    });
    this.json.initial_accounts = this.json.initial_accounts.concat(this.options.additional_accounts);
  }

  initial_parameters() {
    const defaults = {
      "maintenance_interval": 86400,
      "maintenance_skip_slots": 3,
      "maximum_transaction_size": 2048,
      "maximum_block_size": 2048000000,
      "maximum_time_until_expiration": 86400,
      "maximum_producer_count": 1001
    };
    this.json.initial_parameters = Object.assign(defaults, this.options.initial_parameters);
    console.log(this.json.initial_parameters);
  }

  immutable_parameters() {
    this.json.immutable_parameters = {
      "min_producer_count": 21
    };
  }

  timestamp() {
    this.json.initial_timestamp = this.options.initial_timestamp || "2017-03-30T12:00:00";
  }

  chainid() {
    this.json.initial_chain_id = "0000000000000000000000000000000000000000000000000000000000000000";
  }

}
// Genesis JSON
const download_genesis_json = data => export_json(data, 'genesis');

const export_json = (items, filename) => {
  filename = filename + '.json' || 'export.csv';
  let json = JSON.stringify(items, null, '\t');
  let blob = new Blob([json], { type: 'application/json;charset=utf-8;' });
  if (navigator.msSaveBlob) {
    // IE 10+
    navigator.msSaveBlob(blob, filename);
  } else {
    let link = document.createElement("a");
    if (link.download !== undefined) {
      // feature detection
      // Browsers that support HTML5 download attribute
      let url = URL.createObjectURL(blob);
      link.setAttribute("href", url);
      link.setAttribute("download", filename);
      link.style.visibility = 'hidden';
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
    }
  }
};
