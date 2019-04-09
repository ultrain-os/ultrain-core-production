const config = {
  httpEndpoint: "http://127.0.0.1:8888",
  httpEndpointHistory: "http://127.0.0.1:3000",
  chainId: "eb07687ab55a8f5ebd0841ffac1b0afacf9909b817f95dcc0477091ed36d24ef",
  broadcast: true,
  sign: true,
  logger: {
    directory: "../../logs", // daily rotate file directory
    level: "info", // error->warn->info->verbose->debug->silly
    console: true, // print to console
    file: false // append to file
  },
  symbol: "UGAS",
  keyProvider: ['5KWwKASXddxnNupNpDpwh1Nh7ywfvdfEj7dRQah9AGP4FyNTeHp']
};
module.exports = config;
