## ULTRAINIO Developer Tutorial 

The purpose of this tutorial is to demonstrate how to setup a local blockchain 
that can be used to experiment with smart contracts. The first part of this
tutorial will focus on:

1. Starting a Node
2. Creating a Wallet
3. Creating Accounts
4. Deploying Contracts
5. Interacting with Contracts

The second part of this tutorial will walk you through creating and deploying
your own contracts.

This tutorial assumes that you have installed ULTRAINIO and that `nodultrain` and 
`clultrain` are in your path. 


## Starting a Private Blockchain

You can start your own single-node blockchain with this single command:

```
$ nodultrain -e -p ultrainio --plugin ultrainio::wallet_api_plugin --plugin ultrainio::chain_api_plugin --plugin ultrainio::account_history_api_plugin 
...
ultrainio generated block 046b9984... #101527 @ 2018-04-01T14:24:58.000 with 0 trxs
ultrainio generated block 5e527ee2... #101528 @ 2018-04-01T14:24:58.500 with 0 trxs

```

This command sets many flags and loads some optional plugins which we will need for the rest of this tutorial. Assuming everything worked properly, you should see a block generation message every 0.5 seconds.  

```
ultrainio generated block 046b9984... #101527 @ 2018-04-01T14:24:58.000 with 0 trxs
```

This means your local blockchain is live, producing blocks, and ready to be used.

For more information about the arguments to `nodultrain` you can use:

```
nodultrain --help
```

## Creating a Wallet

A wallet is a repository of private keys necessary to authorize actions on the blockchain.  These keys are stored on disk encrypted using a password generated for you.  This password should be stored in a secure password manager.

```
$ clultrain wallet create
Creating wallet: default
Save password to use in the future to unlock this wallet.
Without password imported keys will not be retrievable.
"PW5JuBXoXJ8JHiCTXfXcYuJabjF9f9UNNqHJjqDVY7igVffe3pXub"
```

For the purpose of this simple development environment, your wallet is being managed by your local `nodultrain` via the `ultrainio::wallet_api_plugin` we enabled when we started `nodultrain`.  Any time you restart `nodultrain` you will have to unlock your wallet before you can use the keys within.

```
$ clultrain wallet unlock --password PW5JuBXoXJ8JHiCTXfXcYuJabjF9f9UNNqHJjqDVY7igVffe3pXub
Unlocked: default
```

It is generally not secure to use your password directly on the commandline where it gets logged to your bash history, so you can also unlock in interactive mode:

```
$ clultrain wallet unlock
password:
```

For security purposes it is generally best to leave your wallet locked when you are not using it.  To lock your wallet without shutting down `nodultrain` you can do:

```
$ clultrain wallet lock
Locked: default
```

You will need your wallet unlocked for the rest of this tutorial.

All new blockchains start out with a master key for the sole initial account, `ultrainio`. To interact with the blockchain you will need to import this initial account's private key into your wallet.

Import the master key for the `ultrainio` account into your wallet.  The master key can be found in the `config.ini` file in the config folder for `nodultrain`.  In this example, the default config folder is used.  On Linux systems, this will be in `~/.local/share/ultrainio/nodultrain/config` and on MacOS, this will be in `~/Library/Application Support/ultrainio/nodultrain/config`.

```
$ clultrain wallet import 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
imported private key for: ULTRAIN6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
```

## Load the Bios Contract

Now that we have a wallet with the key for the `ultrainio` account loaded, we can set a default system contract.  For the purposes of development, the default `ultrainio.bios` contract can be used.  This contract enables you to have direct control over the resource allocation of other accounts and to access other privileged API calls. In a public blockchain, this contract will manage the staking and unstaking of tokens to reserve bandwidth for CPU and network activity, and memory for contracts. 

The `ultrainio.bios` contract can be found in the `contracts/ultrainio.bios` folder of your ULTRAINIO source code.  The command sequence below assumes it is being executed from the root of the ULTRAINIO source, but you can execute it from anywhere by specifying the full path to `${ULTRAINIO_SOURCE}/build/contracts/ultrainio.bios`.

```
$ clultrain set contract ultrainio build/contracts/ultrainio.bios -p ultrainio
Reading WAST...
Assembling WASM...
Publishing contract...
executed transaction: 414cf0dc7740d22474992779b2416b0eabdbc91522c16521307dd682051af083  4068 bytes  10000 cycles
#         ultrainio <= ultrainio::setcode               {"account":"ultrainio","vmtype":0,"vmversion":0,"code":"0061736d0100000001ab011960037f7e7f0060057f7e7e7e...
#         ultrainio <= ultrainio::setabi                {"account":"ultrainio","abi":{"types":[],"structs":[{"name":"set_account_limits","base":"","fields":[{"n...
```

The result of this command sequence is that `clultrain` generated a transaction with two actions, `ultrainio::setcode` and `ultrainio::setabi`.  

The code defines how the contract runs and the abi describes how to convert between binary and json representations of the arguments.  While an abi is technically optional, all of the ULTRAINIO tooling depends upon it for ease of use.  

Any time you execute a transaction you will see output like:
```
executed transaction: 414cf0dc7740d22474992779b2416b0eabdbc91522c16521307dd682051af083  4068 bytes  10000 cycles
#         ultrainio <= ultrainio::setcode               {"account":"ultrainio","vmtype":0,"vmversion":0,"code":"0061736d0100000001ab011960037f7e7f0060057f7e7e7e...
#         ultrainio <= ultrainio::setabi                {"account":"ultrainio","abi":{"types":[],"structs":[{"name":"set_account_limits","base":"","fields":[{"n...
```

This can be read as: The action `setcode` as defined by `ultrainio` was executed by `ultrainio` contract with `{args...}`.

```
#         ${executor} <= ${contract}:${action} ${args...}
> console output from this execution, if any
```

As we will see in a bit, actions can be processed by more than one contract.

The last argument to this call was `-p ultrainio`.  This tells `clultrain` to sign this action with the active authority of the `ultrainio` account, i.e., to sign the action using the private key for the `ultrainio` account that we imported earlier. 


## Create an Account

Now that we have setup the basic system contract, we can start to create our own accounts.  We will create two accounts, `user` and `tester`, and we will need to associate a key with each account.  In this example, the same key will be used for both accounts.

To do this we first generate a key for the accounts. 

```
$ clultrain create key
Private key: 5Jmsawgsp1tQ3GD6JyGCwy1dcvqKZgX6ugMVMdjirx85iv5VyPR
Public key: ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
```

Then we import this key into our wallet:
```
$ clultrain wallet import 5Jmsawgsp1tQ3GD6JyGCwy1dcvqKZgX6ugMVMdjirx85iv5VyPR
imported private key for: ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
```
**NOTE:** Be sure to use the actual key value generated by the `clultrain` command and not the one shown in the example above!

Keys are not automatically added to a wallet, so skipping this step could result in losing control of your account.

## Create Two User Accounts

Next we will create two accounts, `user` and `tester`, using the key we created and imported above.

```
$ clultrain create account ultrainio user ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
executed transaction: 8aedb926cc1ca31642ada8daf4350833c95cbe98b869230f44da76d70f6d6242  364 bytes  1000 cycles
#         ultrainio <= ultrainio::newaccount            {"creator":"ultrainio","name":"user","owner":{"threshold":1,"keys":[{"key":"ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZ...

$ clultrain create account ultrainio tester ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
executed transaction: 414cf0dc7740d22474992779b2416b0eabdbc91522c16521307dd682051af083 366 bytes  1000 cycles
#         ultrainio <= ultrainio::newaccount            {"creator":"ultrainio","name":"tester","owner":{"threshold":1,"keys":[{"key":"ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZ...
```
**NOTE:** The `create account` subcommand requires two keys, one for the OwnerKey (which in a production environment should be kept highly secure) and one for the ActiveKey.  In this tutorial example, the same key is used for both.

Because we are using the `ultrainio::account_history_api_plugin` we can query all accounts that are controlled by our key:

```
$ clultrain get accounts ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
{
  "account_names": [
    "tester",
    "user"
  ]
}
```

## Create Token Contract

At this stage the blockchain doesn't do much, so let's deploy the `ultrainio.token` contract.  This contract enables the creation of many different tokens all running on the same contract but potentially managed by different users. 

Before we can deploy the token contract we must create an account to deploy it to.

```
$ clultrain create account ultrainio ultrainio.token  ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
...
```

Then we can deploy the contract which can be found in `${ULTRAINIO_SOURCE}/build/contracts/ultrainio.token`

```
$ clultrain set contract ultrainio.token build/contracts/ultrainio.token -p ultrainio.token
Reading WAST...
Assembling WASM...
Publishing contract...
executed transaction: 528bdbce1181dc5fd72a24e4181e6587dace8ab43b2d7ac9b22b2017992a07ad  8708 bytes  10000 cycles
#         ultrainio <= ultrainio::setcode               {"account":"ultrainio.token","vmtype":0,"vmversion":0,"code":"0061736d0100000001ce011d60067f7e7f7f7f7f00...
#         ultrainio <= ultrainio::setabi                {"account":"ultrainio.token","abi":{"types":[],"structs":[{"name":"transfer","base":"","fields":[{"name"...
```

### Create the ULTRAIN Token

You can view the interface to `ultrainio.token` as defined by `contracts/ultrainio.token/ultrainio.token.hpp`:
```
   void create( account_name issuer,
                asset        maximum_supply,
                uint8_t      can_freeze,
                uint8_t      can_recall,
                uint8_t      can_whitelist );


   void issue( account_name to, asset quantity, string memo );

   void transfer( account_name from,
                  account_name to,
                  asset        quantity,
                  string       memo );
```

To create a new token we must call the `create(...)` action with the proper arguments. This command will use the symbol of the maximum supply to uniquely identify this token from other tokens. The issuer will be the one with authority to call issue and or perform other actions such as freezing, recalling, and whitelisting of owners.

The concise way to call this method, using positional arguments:
```
$ clultrain push action ultrainio.token create '[ "ultrainio", "1000000000.0000 UTR", 0, 0, 0]' -p ultrainio.token
executed transaction: 0e49a421f6e75f4c5e09dd738a02d3f51bd18a0cf31894f68d335cd70d9c0e12  260 bytes  1000 cycles
#   ultrainio.token <= ultrainio.token::create          {"issuer":"ultrainio","maximum_supply":"1000000000.0000 UTR","can_freeze":0,"can_recall":0,"can_whitelis...
```

Alternatively, a more verbose way to call this method, using named arguments:

```
$ clultrain push action ultrainio.token create '{"issuer":"ultrainio", "maximum_supply":"1000000000.0000 UTR", "can_freeze":0, "can_recall":0, "can_whitelist":0}' -p ultrainio.token
executed transaction: 0e49a421f6e75f4c5e09dd738a02d3f51bd18a0cf31894f68d335cd70d9c0e12  260 bytes  1000 cycles
#   ultrainio.token <= ultrainio.token::create          {"issuer":"ultrainio","maximum_supply":"1000000000.0000 UTR","can_freeze":0,"can_recall":0,"can_whitelis...
```


This command created a new token `ULTRAIN` with a pecision of 4 decimials and a maximum supply of 1000000000.0000 ULTRAIN. 

In order to create this token we required the permission of the `ultrainio.token` contract because it "owns" the symbol namespace (e.g. "UTR"). Future versions of this contract may allow other parties to buy symbol names automatically.  For this reason we must pass `-p ultrainio.token` to authorize this call.

### Issue Tokens to Account "User"

Now that we have created the token, the issuer can issue new tokens to the account `user` we created earlier. 

We will use the positional calling convention (vs named args).

```
$ clultrain push action ultrainio.token issue '[ "user", "100.0000 UTR", "memo" ]' -p ultrainio
executed transaction: 822a607a9196112831ecc2dc14ffb1722634f1749f3ac18b73ffacd41160b019  268 bytes  1000 cycles
#   ultrainio.token <= ultrainio.token::issue           {"to":"user","quantity":"100.0000 UTR","memo":"memo"}
>> issue
#   ultrainio.token <= ultrainio.token::transfer        {"from":"ultrainio","to":"user","quantity":"100.0000 UTR","memo":"memo"}
>> transfer
#         ultrainio <= ultrainio.token::transfer        {"from":"ultrainio","to":"user","quantity":"100.0000 UTR","memo":"memo"}
#          user <= ultrainio.token::transfer        {"from":"ultrainio","to":"user","quantity":"100.0000 UTR","memo":"memo"}
```

This time the output contains several different actions:  one issue and three transfers.  While the only action we signed was `issue`, the `issue` action performed an "inline transfer" and the "inline transfer" notified the sender and receiver accounts.  The output indicates all of the action handlers that were called, the order they were called in, and whether or not any output was generated by the action.

Technically, the `ultrainio.token` contract could have skipped the `inline transfer` and opted to just modify the balances directly.  However, in this case, the `ultrainio.token` contract is following our token convention that requires that all account balances be derivable by the sum of the transfer actions that reference them.  It also requires that the sender and receiver of funds be notified so they can automate handling deposits and withdrawals. 

If you want to see the actual transaction that was broadcast, you can use the `-d -j` options to indicate "don't broadcast" and "return transaction as json".

```
$ clultrain push action ultrainio.token issue '["user", "100.0000 UTR", "memo"]' -p ultrainio -d -j
{
  "expiration": "2018-04-01T15:20:44",
  "region": 0,
  "ref_block_num": 42580,
  "ref_block_prefix": 3987474256,
  "net_usage_words": 21,
  "kcpu_usage": 1000,
  "delay_sec": 0,
  "context_free_actions": [],
  "actions": [{
      "account": "ultrainio.token",
      "name": "issue",
      "authorization": [{
          "actor": "ultrainio",
          "permission": "active"
        }
      ],
      "data": "00000000007015d640420f000000000004454f5300000000046d656d6f"
    }
  ],
  "signatures": [
    "UTRJzPywCKsgBitRh9kxFNeMJc8BeD6QZLagtXzmdS2ib5gKTeELiVxXvcnrdRUiY3ExP9saVkdkzvUNyRZSXj2CLJnj7U42H"
  ],
  "context_free_data": []
}
```

### Transfer Tokens to Account "Tester"

Now that account `user` has tokens, we will transfer some to account `tester`.  We indicate that `user` authorized this action using the permission argument `-p user`.

```
$ clultrain push action ultrainio.token transfer '[ "user", "tester", "25.0000 UTR", "m" ]' -p user
executed transaction: 06d0a99652c11637230d08a207520bf38066b8817ef7cafaab2f0344aafd7018  268 bytes  1000 cycles
#   ultrainio.token <= ultrainio.token::transfer        {"from":"user","to":"tester","quantity":"25.0000 UTR","memo":"m"}
>> transfer
#          user <= ultrainio.token::transfer        {"from":"user","to":"tester","quantity":"25.0000 UTR","memo":"m"}
#        tester <= ultrainio.token::transfer        {"from":"user","to":"tester","quantity":"25.0000 UTR","memo":"m"}
```


## Hello World Contract

We will now create our first "hello world" contract.  Create a new folder called "hello", cd into the folder, then create a file "hello.cpp" with the following contents:

#### hello/hello.cpp
```
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/print.hpp>
using namespace ultrainio;

class hello : public ultrainio::contract {
  public:
      using contract::contract;

      /// @abi action 
      void hi( account_name user ) {
         print( "Hello, ", name{user} );
      }
};

ULTRAINIO_ABI( hello, (hi) )
```

You can compile your code to web assmebly (.wast) as follows:
```
$ ultrainiocpp -o hello.wast hello.cpp
```
**NOTE:**  The compiler might generate warnings.  These can be safely ignored.

Now generate the abi:

```
$ ultrainiocpp -g hello.abi hello.cpp
Generated hello.abi
```

Create an account and upload the contract:

```
$ clultrain create account ultrainio hello.code ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
...
$ clultrain set contract hello.code ../hello -p hello.code
...
```

Now we can run the contract:

```
$ clultrain push action hello.code hi '["user"]' -p user
executed transaction: 4c10c1426c16b1656e802f3302677594731b380b18a44851d38e8b5275072857  244 bytes  1000 cycles
#    hello.code <= hello.code::hi               {"user":"user"}
>> Hello, user
```

At this time the contract allows anyone to authorize it, we could also say:

```
$ clultrain push action hello.code hi '["user"]' -p tester
executed transaction: 28d92256c8ffd8b0255be324e4596b7c745f50f85722d0c4400471bc184b9a16  244 bytes  1000 cycles
#    hello.code <= hello.code::hi               {"user":"user"}
>> Hello, user
```

In this case tester is the one who authorized it and user is just an argument.  If we want our contact to authenticate the user we are saying "hi" to, then we need to modify the contract to require authentication.

Modify the hi() function in hello.cpp as follows:
```
void hi( account_name user ) {
   require_auth( user );
   print( "Hello, ", name{user} );
}
```
Repeat the steps to compile the wast file and generate the abi, then set the contract again to deploy the update.

Now if we attempt to mismatch the user and the authority, the contract will throw an error:
```
$ clultrain push action hello.code hi '["tester"]' -p user
Error 3030001: missing required authority
Ensure that you have the related authority inside your transaction!;
If you are currently using 'clultrain push action' command, try to add the relevant authority using -p option.
Error Details:
missing authority of tester
```

We can fix this by giving the permission of tester:

```
$ clultrain push action hello.code hi '["tester"]' -p tester
executed transaction: 235bd766c2097f4a698cfb948eb2e709532df8d18458b92c9c6aae74ed8e4518  244 bytes  1000 cycles
#    hello.code <= hello.code::hi               {"user":"tester"}
>> Hello, tester
```


## Deploy Exchange Contract
Similar to the examples shown above, we can deploy the exchange contract.  It is assumed this is being run from the root of the ULTRAINIO source.

```
$ clultrain create account ultrainio exchange  ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 ULTRAIN7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
executed transaction: 4d38de16631a2dc698f1d433f7eb30982d855219e7c7314a888efbbba04e571c  364 bytes  1000 cycles
#         ultrainio <= ultrainio::newaccount            {"creator":"ultrainio","name":"exchange","owner":{"threshold":1,"keys":[{"key":"ULTRAIN7ijWCBmoXBi3CgtK7DJxe...

$ clultrain set contract exchange build/contracts/exchange -p exchange
Reading WAST...
Assembling WASM...
Publishing contract...
executed transaction: 5a63b4de8a1da415590778f163c5ed26dc164c960185b20fd834c297cf7fa8f4  35172 bytes  10000 cycles
#         ultrainio <= ultrainio::setcode               {"account":"exchange","vmtype":0,"vmversion":0,"code":"0061736d0100000001f0023460067f7e7f7f7f7f00600...
#         ultrainio <= ultrainio::setabi                {"account":"exchange","abi":{"types":[{"new_type_name":"account_name","type":"name"}],"structs":[{"n...
```

