class Genesis {

  constructor( snapshot ){
    this.json = {}
    this.snapshot = snapshot
    this.options
  }

  config( options ){
    this.options = options
  }

  build( args ){

    this.timestamp()
    this.initial_parameters()
    this.immutable_parameters()
    this.accounts()
    this.chainid()
  }

  accounts(){
    this.load()
    this.format()
    this.append()
    this.index()
    this.encode()
  }

  load(){
    this.load_initial_producers()
    this.load_initial_accounts()
  }

  append(){
    this.append_to_initial_accounts()
  }

  format(){
    this.format_initial_accounts()
  }

  index(){
    this.index_initial_producers()
  }

  encode(){
    this.encode_initial_producers()
    this.encode_initial_accounts()
  }

  load_initial_producers(){
    this.json.initial_producers = this.options.initial_producers || []
  }

  load_initial_accounts(){
    this.json.initial_accounts = this.snapshot
  }

  //Format account object from snapshot to format and member names suitable for genesis
  format_initial_accounts(){
    this.json.initial_accounts = this.json.initial_accounts.map( (account) => {
      return {
        "name": account.eth.replace(/"/g, ''),
        "owner_key": account.ultrain.replace(/"/g, ''),
        "active_key": account.ultrain.replace(/"/g, ''),
        "liquid_balance": `${account.balance.replace(/"/g, '')} ULTRAIN`
      }
    })
  }

  //Aggregate the index for producers so a consistent base32 encoded username can be generated. Throw error if no matching account can be found.
  index_initial_producers(){
    this.json.initial_producers.forEach( (producer, producer_index) => {
      this.json.initial_accounts.forEach( (account, account_index ) => {
        if(account.name == producer.owner_name)
          this.json.initial_producers[producer_index].index = account_index
      })
      if(typeof this.json.initial_producers[producer_index].index === "undefined") throw new Error(`Initial Producer ${producer.owner_name} Was Not Found in Initial Accounts`)
    })
  }

  //Encode the initial producers with base32 (if the owner_name is NOT an ethereum address.)
  encode_initial_producers(){
    this.json.initial_producers = this.json.initial_producers.map( producer => {
        console.log(producer)
        producer.owner_name = generate_account_name( producer.owner_name.replace(/"/g, ''), producer.index )
        delete producer.index
        return producer
    })
  }

  //Encode the initial accounts with base32 (if the owner_name is NOT an ethereum address.)
  encode_initial_accounts(){
    this.json.initial_accounts = this.json.initial_accounts.map( (account, index) => {
      if(typeof account !== "undefined") {
        account.name = generate_account_name( account.name, index )
        return account
      } else {
        return null
      }
    })
  }

  append_to_initial_accounts(){
    this.options.additional_accounts.map( account => {
      account.liquid_balance = `${account.liquid_balance} ULTRAIN`
      return account
    })
    this.json.initial_accounts = this.json.initial_accounts.concat( this.options.additional_accounts )
  }

  initial_parameters(){
    const defaults = {
      "maintenance_interval": 86400,
      "maintenance_skip_slots": 3,
      "maximum_transaction_size": 2048,
      "maximum_block_size": 2048000000,
      "maximum_time_until_expiration": 86400,
      "maximum_producer_count": 1001
    }
    this.json.initial_parameters = Object.assign(defaults, this.options.initial_parameters )
    console.log(this.json.initial_parameters)
  }

  immutable_parameters(){
    this.json.immutable_parameters = {
      "min_producer_count": 21
    }
  }

  timestamp( ){
    this.json.initial_timestamp = this.options.initial_timestamp || "2017-03-30T12:00:00"
  }

  chainid(){
    this.json.initial_chain_id = "0000000000000000000000000000000000000000000000000000000000000000"
  }

}
