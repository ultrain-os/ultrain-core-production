#!/bin/sh
cd /opt/ultrainio/bin

if [ -f '/opt/ultrainio/bin/data-dir/config.ini' ]; then
    echo
  else
    cp /config.ini /opt/ultrainio/bin/data-dir
fi

if [ -d '/opt/ultrainio/bin/data-dir/contracts' ]; then
    echo
  else
    cp -r /contracts /opt/ultrainio/bin/data-dir
fi

while :; do
    case $1 in
        --config-dir=?*)
            CONFIG_DIR=${1#*=}
            ;;
        *)
            break
    esac
    shift
done

if [ ! "$CONFIG_DIR" ]; then
    CONFIG_DIR="--config-dir=/opt/ultrainio/bin/data-dir"
else
    CONFIG_DIR=""
fi

exec /opt/ultrainio/bin/nodultrain $CONFIG_DIR "$@"
