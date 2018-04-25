# ULTRAINIO Genesis

Tools used to initialize a genesis block for the ULTRAIN Platform

## Tools

- Snapshot Generator ([readme](https://github.com/ULTRAINIO/genesis/tree/master/tools/snapshot)) - Generates a snapshot from the ULTRAIN Crowdsale and Token contracts (saves a snapshot.csv and snapshot.json [metadata] into root directory of project)
- [Genesis Block Generator](https://ultrainio.github.io/genesis/) ([readme](https://github.com/ULTRAINIO/genesis/tree/master/tools/genesis)) - Enables configuration of a well-formed genesis.json file from a snapshot.csv. 
- [Account Name Lookup](https://ultrainio.github.io/genesis/tools/account-name/index.html) Look up the generated account name that was included in the genesis block with either your ETH address or ULTRAIN key
- ULTRAIN Genesis Key Utility ([readme](https://github.com/ULTRAINIO/genesis/tree/master/tools/keys)) - Offline ULTRAIN key gen and validator
- _Snapshot Frontend (planned)_

## Useful Files
- **snapshot.csv** Comma-delimited distribution list
- **snapshot.json** Meta data for the snapshot in the repo, describes the parameters of snapshot, some statistics and data for debugging
