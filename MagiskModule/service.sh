MODDIR=${0%/*}
chmod +x $MODDIR/module
until [ -d /data/data/android ]; do sleep 1; done
killall -15 module; nohup $MODDIR/module > /dev/null 2>&1 &
