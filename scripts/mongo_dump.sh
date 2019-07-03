#!/bin/bash
BIN_DIR=/usr/bin
DUMP=$BIN_DIR/mongodump

OUT_DIR=/mongobackup
DATE=`date +%Y_%m_%d`

BASE_PARAM="-u root -p Uranus --gzip --authenticationDatabase ultrain -d ultrain"

if [ ! -d $OUT_DIR ]; then
   mkdir -p $OUT_DIR
fi

$DUMP $BASE_PARAM -c block_states -o $OUT_DIR
$DUMP $BASE_PARAM --excludeCollection block_states --excludeCollection profits --excludeCollection producers -o $OUT_DIR

mv $OUT_DIR/ultrain $OUT_DIR/$DATE

# delete 1 day before dump file
DAYS=1
find $OUT_DIR/ -type d -ctime +$DAYS |xargs rm -rf   

exit
