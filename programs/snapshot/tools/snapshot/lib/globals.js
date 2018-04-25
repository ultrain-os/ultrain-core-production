const bn = require('bignumber.js')

global.SS_STARTED_TIMESTAMP               = Date.now()/1000 | 0
global.SS_MAINNET_TIMESTAMP_END           = 0 //A cutoff timestamp should be provided so snapshot audits are not impossible. Mainnet snapshot would only account for registrations/claims up to the last block with a timestamp lte this timestamp.
global.SS_ACCEPTABLE_SUPPLY_DEVIATION     = 5 //total number of ULTRAIN deviation allowed in for accurate snapshot, usually less than 1/-1. 5 is generous.

global.WAD                                = new bn(1000000000000000000)

global.CS_ADDRESS_CROWDSALE               = "0xd0a6e6c54dbc68db5db3a091b171a77407ff7ccf"
global.CS_ADDRESS_TOKEN                   = "0x86fa049857e0209aa7d9e616f7eb3b3b78ecfdb0"
global.CS_ADDRESS_UTILITIES               = "0x860fd485f533b0348e413e65151f7ee993f93c02"
global.CS_ADDRESS_B1                      = "0x00000000000000000000000000000000000000b1"

global.CS_OPEN_TIME                       = 1498482000
global.CS_START_TIME                      = 1498914000
global.CS_BLOCK_FIRST                     = 3904416

global.CS_CREATE_FIRST_PERIOD             = new bn(200000000).times(WAD)
global.CS_CREATE_PER_PERIOD               = new bn(2000000).times(WAD)
global.CS_NUMBER_OF_PERIODS               = 351
global.CS_PERIOD_ETH                      = []

global.web3
global.mysql
global.redis
