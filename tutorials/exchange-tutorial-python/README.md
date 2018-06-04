The following steps must be taken for the example script to work.

0. Create wallet
0. Create account for ultrainio.token
0. Create account for scott
0. Create account for exchange
0. Set token contract on ultrainio.token
0. Create ULTRAIN token
0. Issue initial tokens to scott

**Note**:
Deleting the `transactions.txt` file will prevent replay from working.


### Create wallet
`clultrain wallet create`

### Create account steps
`clultrain create key`

`clultrain create key`

`clultrain wallet import <private key from step 1>`

`clultrain wallet import <private key from step 2>`

`clultrain create account ultrainio <account_name> <public key from step 1> <public key from step 2>`

### Set contract steps
`clultrain set contract ultrainio.token /contracts/ultrainio.token -p ultrainio.token@active`

### Create ULTRAIN token steps
`clultrain push action ultrainio.token create '{"issuer": "ultrainio.token", "maximum_supply": "100000.0000 UTR", "can_freeze": 1, "can_recall": 1, "can_whitelist": 1}' -p ultrainio.token@active`

### Issue token steps
`clultrain push action ultrainio.token issue '{"to": "scott", "quantity": "900.0000 UTR", "memo": "testing"}' -p ultrainio.token@active`
