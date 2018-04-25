let prompt = require('prompt'),
    colors = require("colors/safe"),
    optimist = require('optimist'),
    fs = require('fs')

let period  = require('./utilities/periods')

let schema = {

  properties: {

    //Unoad Config?
    load_config: {
      type: 'boolean',
      description: colors.magenta('Config file detected, load it?'),
      message: 'Must be boolean',
      default: true,
      ask: () => fs.existsSync('./config.js')
    },

    //Required Configuration
    period: {
      description: colors.magenta('Snapshot up to which Period?'),
      message: `Period must be in the past and closed (Default: ${period.last_closed()})`,
      required: true,
      type: 'number',
      conform: function (p) {
        if(p > period.last_closed())
          return
        return true
      },
      default: period.last_closed(),
      ask: () => prompt.history('load_config') ? prompt.history('load_config').value===false : true
    },

    //Options
    include_b1: {
      type: 'boolean',
      description: colors.magenta('Include B1 Distribution in Snapshot?'),
      message: 'Must be boolean',
      default: true,
      ask: () => prompt.history('load_config') ? prompt.history('load_config').value===false : true
    },

    fallback: {
      type: 'boolean',
      description: colors.magenta('Run experimental registration fallback? (will take days, not exagerating)'),
      message: 'Must be boolean',
      default: false,
      ask: () => prompt.history('load_config') ? prompt.history('load_config').value===false : true
    },

    //Meta
    cache: {
      type: 'boolean',
      description: colors.magenta('Cache Keys? (advanced)'),
      message: 'Must be true or false',
      default: false,
      ask: () => prompt.history('load_config') ? prompt.history('load_config').value===false : true
    },

    //Meta
    author: {
      type: 'string',
      description: colors.magenta('Author name? (optional)'),
      message: 'Author must be only letters, spaces, or dashes',
      default: "Anonymous",
      ask: () => prompt.history('load_config') ? prompt.history('load_config').value===false : true
    },

    //Meta
    overwrite_snapshot: {
      type: 'string',
      description: colors.magenta('Overwrite snapshot in project root? (optional)'),
      message: 'Must be true or false',
      default: true,
      ask: () => prompt.history('load_config') ? prompt.history('load_config').value===false : true
    }

    // //Custom Configuration?
    // custom_config: {
    //   type: 'boolean',
    //   description: colors.magenta('Do you require custom eth rpc node, mysql and/or redis connection details?'),
    //   message: 'Must be boolean',
    //   default: false,
    //   ask: () => !prompt.history('load_config')
    // },
    //
    // //ETH Node
    // custom_eth_node_config: {
    //   type: 'boolean',
    //   description: colors.magenta('Custom ethereum node rpc configuration?'),
    //   message: 'Would you like to setup custom etherem node configuration? (Note: Infura nodes are not compatible with this script)',
    //   default: false,
    //   ask: () => !prompt.history('load_config') && prompt.history('custom_config')
    // },
    //
    // eth_node_type: {
    //   type: 'string',
    //   description: colors.magenta('What RPC type?  (options: http, ws, ipc)'),
    //   message: 'What type of node? (options: http, ws, ipc)',
    //   default: false,
    //   conform: value => (value == 'ipc' || value == 'http' || value == "ws"),
    //   ask: () => !prompt.history('load_config') && prompt.history('custom_config') && prompt.history('custom_eth_node_config')
    // },
    //
    // eth_path_type: {
    //   type: 'string',
    //   description: colors.magenta('Node path? Example: localhost:8456 (if left empty, default for type will be used)'),
    //   ask: () => !prompt.history('load_config') && prompt.history('custom_config') && prompt.history('custom_eth_node_config')
    // },
    //
    // // MySQL
    // custom_mysql_config: {
    //   type: 'boolean',
    //   description: colors.magenta('Custom Mysql Config?  (options: http, ws, ipc)'),
    //   default: false,
    //   ask: () => !prompt.history('load_config') && prompt.history('custom_config')
    // },
    //
    // //Redis
    // custom_redis_config: {
    //   type: 'boolean',
    //   description: colors.magenta('Custom Redis Config?  (options: http, ws, ipc)'),
    //   default: false,
    //   ask: () => !prompt.history('load_config') && prompt.history('custom_config')
    // }

  }
};

prompt.override = optimist.argv

process.on( 'SIGINT', () => process.exit() )

module.exports = {
  schema: schema,
  start: () => prompt.start({noHandleSIGINT: true}),
  get: prompt.get
}
