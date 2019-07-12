#!/bin/bash

## para arguments
if [ $# -ne 2 ];then
   echo -e "need 2 arguments.\n1.bin dir.\n2.backup dir"
   exit
fi

## global para
BIN_DIR=$1
OUT_DIR=$2

DUMP=$BIN_DIR/mongodump
MONGO=$BIN_DIR/mongo
DBs="action_traces actions block_states blocks transaction_traces transactions"
DB_FULL="account_controls pub_keys accounts"

INTERVAL=10000
## prepare mongo info
nodconfig=~/.local/share/ultrainio/nodultrain/config/config.ini
info=`grep "mongodb-uri" $nodconfig |cut -d '/' -f 3`
account=`echo $info | cut -d '@' -f 1`
socket=`echo $info | cut -d '@' -f 2`
USER=`echo $account | cut -d ':' -f 1`
PASS=`echo $account | cut -d ':' -f 2`
IP=`echo $socket | cut -d ':' -f 1`
PORT=`echo $socket|cut -d ':' -f 2`

BASE_PARAM="-u $USER -p $PASS --gzip --authenticationDatabase ultrain -d ultrain"
MONGO_EVAL="-u $USER -p $PASS --authenticationDatabase ultrain $IP/ultrain --eval"

## function for dumping
dump(){
   s=$1
   e=$2
   blk_range=$s-$e
   echo $blk_range
   mkdir -p $OUT_DIR/$blk_range

   ## increse dump
   for db in $DBs;
   do
      $DUMP $BASE_PARAM -c $db -q '{block_num:{$gte:'$s',$lte:'$e'}}' -o $OUT_DIR/$blk_range
   done

   ##account_controls ,pub_keys ,accounts tables are fullly dumped
   for db in $DB_FULL;
   do
      $DUMP $BASE_PARAM -c $db -o $OUT_DIR/$blk_range
   done
}

##get the latest mongo backup block height
if [ ! -d $OUT_DIR ]; then
   mkdir -p $OUT_DIR
fi
for i in `ls -vr $OUT_DIR`;
do
   tmp_blknum=`echo $i|cut -d '-' -f 2`
   expr $tmp_blknum "+" 1 &>/dev/null
   if [ $? -eq 0 ] ; then
       block_begin=$tmp_blknum
	   break
   fi
done
if [ -z $block_begin ] ; then
    block_begin=0
fi
((block_begin=block_begin+1))
echo "begin $block_begin"

## get the current block height
block_end=`mongo $MONGO_EVAL "db.block_states.find({'irreversible':{ \\$exists:false}},{block_num:1,_id:0}).sort({block_num:1}).limit(1)" | grep block_num|sed s/[[:space:]]//g  | awk -F'[:} ]' '{print $2;}'`
if [ -z $block_end ] ; then
    block_end=`mongo $MONGO_EVAL "db.block_states.find({'irreversible':{ \\$exists:true}},{block_num:1,_id:0}).sort({block_num:-1}).limit(1)"| grep block_num|sed s/[[:space:]]//g  | awk -F'[:} ]' '{print $2;}'`
fi
echo "end $block_end"
if [ -z $block_end ] ; then
    echo "cannot get the current block."
	exit
fi

if [ $INTERVAL -gt $((block_end-block_begin)) ] ; then
    echo "block range is 0"
	exit
fi

## dump
while (true)
do
   tmp=$(((block_begin-1+INTERVAL)/INTERVAL*INTERVAL))
   if [ $tmp -le $block_end ]; then
      dump $block_begin $tmp
      block_begin=$((tmp+1))
   else
      break
   fi
done
