#!/bin/bash
BIN_DIR=/usr/bin
MONGD=$BIN_DIR/mongod
MONGO=$BIN_DIR/mongo
MONGORESTORE=$BIN_DIR/mongorestore

LOG_DIR=/mongodb/log
LOG=mongodb.log
DATA_DIR=/mongodb/data
OUT_DIR=/mongobackup

RESTORE_FILE=`ls -t $OUT_DIR|head -n 1`

BASE_PARA="--dbpath $DATA_DIR --logpath $LOG_DIR/$LOG --fork --bind_ip 0.0.0.0"
AUTH_PARA="--username root --password Uranus --gzip --authenticationDatabase ultrain"
DB="127.0.0.1/ultrain"
ACCOUNT='db.createUser( {user: "root", pwd: "Uranus",roles: [ { role: "readWriteAnyDatabase", db: "admin" }]})'

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
$MONGORESTORE $AUTH_PARA -d ultrain  $OUT_DIR/$RESTORE_FILE
echo "done"
