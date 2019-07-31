#!/bin/bash

## para arguments
if [ $# -ne 4 ];then
    echo -e "need 4 arguments.\n1.bin dir.\n2.mongodb dir\n3.backup dir\n4.result file"
    exit
fi

## global para
BIN_DIR=$1
MONGD=$BIN_DIR/mongod
MONGO=$BIN_DIR/mongo
MONGORESTORE=$BIN_DIR/mongorestore

DATA=$2
LOG_DIR=$DATA/log
LOG=mongodb.log
DATA_DIR=$DATA/data
OUT_DIR=$3
RESULT=$4

## prepare mongo info
nodconfig=~/.local/share/ultrainio/nodultrain/config/config.ini
info=`grep "mongodb-uri" $nodconfig |cut -d '/' -f 3`
account=`echo $info | cut -d '@' -f 1`
socket=`echo $info | cut -d '@' -f 2`
USER=`echo $account | cut -d ':' -f 1`
PASS=`echo $account | cut -d ':' -f 2`
IP=`echo $socket | cut -d ':' -f 1`
PORT=`echo $socket|cut -d ':' -f 2`

BASE_PARA="--dbpath $DATA_DIR --logpath $LOG_DIR/$LOG --fork --bind_ip 0.0.0.0"
AUTH_PARA="-u $USER -p $PASS --gzip --authenticationDatabase ultrain"
DB="$IP/ultrain"
ACCOUNT='db.createUser( {user: "'$USER'", pwd: "'$PASS'",roles: [ { role: "readWriteAnyDatabase", db: "admin" }]})'

##### remove all the old date
pid=`pidof mongod`
if test $pid ; then
   kill -9 $pid
fi

while (true)
do
   sleep 2
   exist=`pidof mongod`
   if [ -z "$exist" ] ; then
       break
   fi
done

rm -rf $LOG_DIR/*
rm -rf $DATA_DIR/*

mkdir -p $LOG_DIR 
mkdir -p $DATA_DIR

###### start mongod and create mongo account
$MONGD $BASE_PARA
sleep 5
$MONGO $DB --eval "$ACCOUNT"

killall mongod
while (true)
do
    sleep 2
    result=`pidof mongod`
	if [ -z "$result" ] ; then
	   break
	fi
done

##### start mongod with auth
$MONGD --auth $BASE_PARA
sleep 5

##### resotre mongo
echo "restore"
end_blknum=1
for i in `ls -v $OUT_DIR`;
do
   start_blknum=`echo $i|cut -d '-' -f 1`
   expr $start_blknum "+" 1 &>/dev/null
   if [ $? -ne 0 ] ; then
       continue
   fi
   if [ $start_blknum -eq $end_blknum ] ; then
       echo "restore $i"
       $MONGORESTORE $AUTH_PARA -d ultrain --excludeCollection account_controls --excludeCollection accounts --excludeCollection pub_keys $OUT_DIR/$i/ultrain
       if [ "$?" -ne 0 ] ; then
           echo "{ \"code\": 2, \"restore $i failed\" }" > $RESULT
           exit
       fi
       let end_blknum=`echo $i|cut -d '-' -f 2`+1
       extra_dir=$i
	else
		continue
   fi
done
if [ $end_blknum -eq 1 ] ; then
   echo "backupfiles was invalid"
   echo "{ \"code\": 1, \"backupfiles was invalid\" }" > $RESULT
   exit
fi
echo "extra table: $extra_dir"
$MONGORESTORE $AUTH_PARA -d ultrain -c account_controls $OUT_DIR/$extra_dir/ultrain/account_controls.bson.gz 
$MONGORESTORE $AUTH_PARA -d ultrain -c accounts $OUT_DIR/$extra_dir/ultrain/accounts.bson.gz 
$MONGORESTORE $AUTH_PARA -d ultrain -c pub_keys $OUT_DIR/$extra_dir/ultrain/pub_keys.bson.gz 
echo $[end_blknum-1]
echo "{ \"code\": 0, \"block_num:$end_blknum\" }" > $RESULT
echo "done"
