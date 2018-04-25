module.exports = (type, path) => {

  const Web3 = require('web3'),
        abi  = require('../lib/abi.js')

  let web3

  type = typeof type!=="undefined" ? type : null,
  path = typeof path!=="undefined" ? path : null

  if(type == 'ipc' && typeof path !== null)
    web3  = new Web3( new Web3.providers.IpcProvider( path, require('net') ) )
  else if(type == 'ws')
    web3  = new Web3( path || "ws://localhost:8546" )     //default ws path
  else
    web3  = new Web3( path || "http://localhost:8545"  )  //default http rpc path

  return web3
}
