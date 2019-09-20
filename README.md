# ULTRAIN - Decentralized Engine for The Future

Welcome to the ULTRAIN source code repository!

Ultrain's goal is to provide a high-performance and high-security permissionless blockchain operating system to
support massive enterprise application logics. To achieve that, the design of Ultrain system has the following
features:

1. Open system for permissionless blockchain network.
1. Random-POS based consensus with high-performance, deterministic finality and quick block confirmation (~10s).
1. Multi-chain design with smart scheduling for best balance between performance & security.
1. Well designed token economy for predictable DAPPs cost & lucrative mining reward.
1. Smart contract platform powered by Web Assembly & TypeScript

##  Open system for permissionless network

Open means not only the relevant code is open sourced, but also the decision making about the project/network development
is also openly discussed and guided. At different phase of the project development, both on-chain and off-chain governance,
and both committee-led and community-led decision making are used.

Permissionless network is the fundamental vision of the whole project: any one can freely stake to become a staker/"miner";
then with a home PC of average hardware spec, one can download the source code, compile a node binary and join the Ultrain network
to provide computation service for enterprise users, in exchange one can get reasonable economical rewards.

One has to admit that to achieve the true "permissionless-ness", certain degree of performance & security inevitably has to
be sacrificed (compared with permissioned network like consortium blockchain network). And a lot of our design & engineering effort
are put into balancing the tradeoff between permissionless-ness and performance & security.


## Random Proof-of-Stake

Our consensus is called Random Proof-of-Stake (R-POS). First of all it is a Proof-of-Stake based consensus, meaning anyone can
obtain the right of block production through staking into the system. Technically, once properly staked, one enters the consensus
committee. Each block production phase is divided into different rounds. First a few randomly chosen committee members, called
"proposers", will propose their block candidate to the whole network, and the propose message is encoded with Verifiable Random Function (VRF)
so the other committee members can verify the validity of the these messages. Also another subset of the committee members are
chosen (again with VRF) as "voters", and these voters will vote on the proposed blocks and then broadcast their votes to the whole
network. The voting process carries out in 2 rounds and eventually everyone in the network agrees on one proposed block, and the
block finality is deterministic at the end of the 2 rounds of voting. The process is illustrated in the following diagram:

![alt text](http://ultrain.info/public/images/github/consensus.png "Consensus Process")


With the assumption of semi-synchronous network condition, the consensus can work against up to 1/3 of the committee members being offline or malicious. More details can be found in our [consensus yellow paper](https://ultrain.info/downloads/consensus.pdf).

By narrowing down the number of proposers in each round of the block production, the network communication overhead is dramatically
reduced. And to achieve the true fairness and randomness of proposers selection, we devise a novel on-chain random number generator that
has passed the NIST SP800-22 test. More details can be found [here](https://developer.ultrain.info/tutorial/Middleware-random_number);
And vote messages are also aggregated using BLS Aggregation Signature to further reduce the network overhead.


## Scalability & Multi-Chain Design

Scalability is an unavoidable topic for any permissionless blockchain project that is serious enough to seek
mass adaption. The architectural solution of Ultrain can be considered as one of the on-chain horizontal scaling solutions.
The fundamental of the solution is to run all the Dapps on shard-chains, while the main-chain manages and coordinates all
the shard-chains. Each shard-chain has a maximum capacity on the hardware resources – when Dapps on the shard-chain reaches
the maximum capacity, a new shard-chain will be created to accommodate the new demands, this is also known as horizontal scaling.
Each shard-chain comprises a group of Dapps, which is segregated from other shard-chains, as a result of such design,
we naturally segregated the computation and storage resources between shard-chains. There will be no user level Dapps running
on the main-chain, which its tasks involve users’ accounts creation/maintenance, miners’ registration/token staking
and scheduling of mining nodes among different shard-chains.

![alt text](http://ultrain.info/public/images/github/sidechain.png "Multichain")


### Main/Shard chain workflow

Following is a brief description of the main-chain/shard-chain workflow:

1. User registers on main-chain to be the miner node, during the registration procedure,
the user is required to stake native token of Ultrain (UGAS); once registration is successful,
miner will be queued for dispatch, the system smart contract on main-chain will randomly assign the miner to a shard-chain.

2. The miner will perform block proposition and validation (mining) and the results are reported to the system smart contract
on main-chain, and once the main-chain verifies the validity of the reported block, the block reward will be distributed to the miner.

3. To enhance the network safety, the system smart contract on the main-chain will reshuffle miners to different shard-chains,
therefore there will be no miner stay permanently on a particular shard-chain. The random reshuffling of miners is executed by
the smart contract, the randomness is verifiable and transparent on the blockchain, the long-term economic return of the miner will not be affected by this.

4. For Dapp users, the registration of account must be performed on the main-chain; the main-chain has the snapshot of all the accounts
of the network; when user needs to interact with a Dapp on a particular shard-chain, the user needs to apply for authorization to access
(empower to) the shard-chain, the IBC mechanism between the main/shard-chain will clone the user’s account to the designated shard-chain.

5. For Dapp developers, when they want to deploy a Dapp on a particular shard-chain, they need to purchase appropriate resource packages
(CPU/Storage/Network) on the main-chain, the purchased packages will be propagated to the designated shard-chain, where the developers can deploy their Dapps.

### Fast World State Syncing

We have mentioned previously that data among each shard-chain are segregated, the benefit is that miner nodes only need to maintain the data
of one shard-chain at a time, this reduces the on-chain data storage pressure; however, data sync is required when miners are reassigned to
a new shard-chain by the main-chain’s smart contract. The most common way of data sync on the blockchain is the block replay model – it syncs all
the history blocks and replays all the transactions on the shard-chain to build the latest world state for the miner node before it can
participate in the consensus process. As the time needed for data sync is proportional to the block height, when the height grows,
it can take days for the data sync to complete, this is unfriendly to miner nodes, to tackle this issue, we designed the world state snapshot mechanism:

1. For a fixed period of time, a world state snapshot is taken and stored locally from all mining nodes on a shard-chain,
the hash value of this world sate snapshot is recorded on the main-chain.

2. Main-chain’s smart contact is responsible to record the shard-chains’ hash value of the world state snapshot.
Only when the world state is validated by miner nodes no less than 2/3 of the committee members on the shard-chain,
the main-chain deems the hash value of the world state snapshot valid.

3. When miner nodes are reshuffled to a new shard-chain, they can retrieve the hash values of recent world state snapshots
from the main-chain; when new miner nodes sync with existing miner nodes on the shard-chain for the world state snapshot,
the new miner nodes can then know if the world state is, in fact, safe and sound.

4. Sync the world state is far more efficient than sync and replay all the blocks, this subsequently reduces the economic
loss from the miner nodes reshuffling.

### IBC between main/shard-chain and light client

With multi-chain architecture, we must consider the notion of inter-blockchain communication mechanism.
From a designing perspective, we need to consider what is needed to be communicated? To achieve inter-blockchain communication,
what native infrastructures are needed by layer 1? And what can be supplied by the library or Dapp on layer 2?
Take all considerations into account, we came up with the following design:

1. Both main-chain and shard-chains provide mutually receptive light-client support, meaning main-chain and shard-chains store
each others’ block header information, they can verify each other and confirm a particular transaction on other’s chain indeed happened.

2. Provide an IBC mechanism based on committee voting, for example, when shard-chain A needs to sync particular data from the main-chain,
committees on shard-chain A will initiate multi-sig validation toward this data on the main-chain. When no less than 2/3 of the committee
members validate, we can consider the data on the main-chain is verified by the shard-chain A. This is an excellent use case of using
Oracle (off-chain data to on-chain) in inter-blockchain communication.

3. For the specific type of IBC, the system achieves natively: 1) main-chain’s block headers are synced to all shard-chains
2) each shard-chain’s block header is synced to main-chain; 3) reshuffling results of the miner nodes among shard-chains i
synced from main-chain to shard-chains; 4) account information on main-chain is synced to shard-chain;
5) the purchased resource package of Dapp developer on main-chain is synced to shard-chains; 6) UGAS can be transferred across the user’s main-chain and shard-chains accounts.

More details of the multi-chain design can be found [here](https://ultrain.info/downloads/Ultrain%E2%80%99s%20Multi-chain%20architecture%20-%20revised.pdf)


## Smart contract platform powered by WebAssembly & TypeScript

WebAssembly(Wasm) is a new, portable, size-/load-time-efficient format. WebAssembly aims to execute at native speed by taking advantage of common hardware capabilities available
on a wide range of platforms. WebAssembly is currently being designed as an open standard by a W3C Community Group. We choose WebAssembly as the smart contract runtime format and
WABT as the VM implementation base to take advantage of WASM's high-performance (close to native hardware execution speed) and high-security (highly sandboxed runtime environment).

The toolchain has already supported C++ as the frontend smart contract developing language if the developers want the maximal control of the runtime performance. But for developer
who wants better developing productivity, we also support TypeScript as another (recommended) option for developing smart contract and provide the full [compiler toolchian](https://github.com/ultrain-os/ultrascript) support, a richful set of [TS library](https://github.com/ultrain-os/ultrain-ts-lib), and some widely adopted [UIPs](https://github.com/ultrain-os/UIPs) (Ultrain Improvement Proposals) spec and ref implementations.

More develop resource can be found at our [developer portal](https://developer.ultrain.info/).


## Token Economy and Resource Package Model

First we believe many blockchain projects failed to design a healthy and developer friendly resource pricing model,
for example, ETH and EOS are both adapting the “overselling model” – with more developers purchase their resource while the overall
resource available remains unchanged, each developer/user will have fewer resources for the money they paid.
We use “preselling model” – we divide the resource of a shard-chain to a fixed number of equivalent portion the developer can
forecast how much CPU/storage/network power its Dapp needed and purchase accordingly. We would like to use the train ticket analogy
to distinguish the “overselling” and “preselling” models, in an “overselling” scenario, when more tickets are sold than the number
of seats a train has, all passengers’ seat space is reduced. In a “preselling” model, we only sell tickets no more than the to
seats available, this ensures everyone’s seat space remains unchanged. In essence, the “overselling” model disregards the outcome of inflation while reaping the full benefit from selling the resource packages, the “preselling” model, on the other hand, balances our package purchasers’ cost and user experience.

More of the resource package model can be found [here](https://developer.ultrain.info/resources)


## LICENSE

ULTRAIN source code is released under the open source MIT license and is offered “AS IS” without warranty of any kin
express or implied. Any security provided by the ULTRAIN software depends in part on how it is used, configured,
and deployed. ULTRAIN is built upon many third-party libraries such as Binaryen and WABT (Apache License) and EOSIO (MIT License)
which are also provided "AS  IS” without warranty of any kind. Without limiting the generality of the foregoing, ULTRAIN.IO
makes no representation or guarantee that ULTRAIN or any third-party libraries will perform as intended or will be free of errors,
bugs or faulty code. Both may fail in large or small ways that could completely or partially limit functionality or compromise
computer systems. If you use or implement ULTRAIN, you do so at your own risk. In no event will Ultrain be liable to any party
for any damages whatsoever, even if it had been advised of the possibility of damage.

## Supported Operating Systems
ULTRAINIO currently supports the following operating systems:
1. Ubuntu 18.04 LTS

Docker image with pre-compiled Ultrain software can be requested for.

## Code Structure
Following are the code structure of some of the core components:
```
├── ...
├── LICENSE
├── README.md
├── base                      // useful base lib
├── consensus                 // RPoS consensus implementation
├── contracts                 // system & test smart contracts
├── core                      // core data structure for consensus
├── crypto                    // various crypto (e.g. ED25519, BLS) related components
├── lightclient               // light client for cross chain communication
├── p2p                       // p2p communication impl
├── plugins                   // feature components as plugins
├── programs                  // node, cli and wallet binary
├── scripts                   // deployment related tools
├── tests                     // mostly performance related tests
├── tools                     // various tools, e.g. key generator
├── ultrainio_build.sh        // main build script
├── unittests
└── vendor                    // Third-Party library, like chainbase, wabt

```

## BUILD & STARTUP
### Perface

  Following steps is an example to create a single chain including 7 docker containers (1 Genesis , 5 Producers, 1 None-Producer) in your host machine. 
  
  For convenience of operation，all docker contaniers `share the host directory`(including ULTRAIN source code dir).
  
### Pull Docker Image 

 Official docker image is recommended to build ULTRAIN source code, it's easy to get docker images from Ultrain's official Dockerhub repository by following cmd.
 
    $ docker pull ultraincore/ultraincore-public
    $ docker images
```text
REPOSITORY                       TAG                 IMAGE ID            CREATED             SIZE
ultraincore/ultraincore-public   latest              64fbde4c84a4        3 hours ago         2.85GB
```

### Start Docker Containers 

    $ cd ultrain-core-production/scripts/docker/
    $ ./createDocker.sh ultrainchain 7 ultraincore/ultraincore-public:latest 8877:8888 /home/sidechain/
    
Parameter Description

* `ultrainchain` : Prefix of docker container name
* `7` : Number of containers(First contanier is Genesis, last is None-Producer, others are Producers)
* `ultraincore/ultraincore-public:latest` : Name of Docker Image
* `8877:8888`: Port mapping(8877 can be changed to any other avaliable port in your host machine).
* `/home/sidechain/`: Host Directory(including ULTRAIN source code dir, you can directly access this directory  by path "`/root/workspace/`" in docker container).

### Build Code 

Build source code in any docker container.

    $ docker attach ultrainchain-7
    $ cd /root/workspace/ultrain-core-production
    $ ./ultrainio_build.sh

### Boot Nodultrain 

Folowing commands must be executed in your host machine(not in docker container), meanwhile make sure `Python` environment is ready. All Nodultrains  will be started up at the same time in 7 docker containers.

    $ cd ultrain-core-production/scripts/
    $ ./start_all_ultrain.sh ultrainchain /root/workspace/ultrain-core-production --httpAlias 172.16.10.5:8877 -ws
    
Parameter Description

* `ultrainchain` : Prefix of docker container name
* `/root/workspace/ultrain-core-production`: Ultrain Code directory in docker container 
* `-ws`: Add this parameter will enable World State Function of chain
* `--httpAlias 172.16.10.5:8877`: Corresponding to your host machine IP and port

Request chain info's API to check whether chain is startup successfully.

    $ docker attach ultrainchain-7
    $ curl 127.0.0.1:8888/v1/chain/get_chain_info
    or you can just request by host machine IP and port :
    curl http://172.16.10.5:8877/v1/chain/get_chain_info

### Perform Genesis BIOS

    $ docker attach ultrainchain-7
    $ cd /root/workspace/ultrain-core-production/scripts/
    $ ./bios-test.py -a -p /root/workspace/ultrain-core-production
    
### Other DevOps Tool


#### Restart Nodultrain

* Single Nodultrain（Docker Container）

    ```text
      $ killall nodultrain
      $ nohup /root/workspace/ultrain-core-production/build/programs/nodultrain/nodultrain &>> /tmp/nod.log &
    ```

* All Nodultrain (Host Machine)

    ```text
      $ cd ultrain-core-production/scripts/
      $ ./restart_all_ultrain.sh ultrainchain /root/workspace/ （`ultrainchain`：Prefix of docker container name ， `/root/workspace`：Base directory in docker container (need not change)）
    ```

#### Stop Nodultrain

* Single Nodultrain（Docker Container）

    ```text
      $ killall nodultrain
    ```
    
* Stop And Remove All Nodultrain and Blocks (Host Machine)

    ```text
      $ cd ultrain-core-production/scripts/
      $ ./stop_all_ultrain.sh ultrainchain
    ```

#### Docker Ops

* Stop All Dockers

    ```text
      $ cd ultrain-core-production/scripts/docker/
      $ ./stopDocker.sh ultrainchain
    ```
    
* Start All Dockers   

    ```text
      $ cd ultrain-core-production/scripts/docker/
      $ ./startDocker.sh ultrainchain
    ```
    
* Remove All Dockers

    ```text
      $ cd ultrain-core-production/scripts/docker/
      $ ./rmDocker.sh ultrainchain
    ```



# Resources
1. [ULTRAIN Website](https://ultrain.info)
2. [ULTRAIN Developer portal](https://developer.ultrain.info)
3. [ULTRAIN Explorer](https://explorer.ultrain.info/)
4. [Community Telegram Group](https://t.me/ultrainchain)
