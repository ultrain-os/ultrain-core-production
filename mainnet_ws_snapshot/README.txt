Ultrain mainnet replay should be based on the worldstate snapshot contained
in this folder. Using newest version of nodultrain to replay from genesis block
will *NOT* generate consistent worldstate to the current worldstate.

Replay instruction:

./nodultrain --worldstate /tmp/xxx.ws --replay-blockchain

--replay-blockchain option will delete the worldstate folder and replace it with
the snapshot from xxx.ws; and then it will start to replay the remaining blocks
either from local blocks.log or by fetching from other nodes;
