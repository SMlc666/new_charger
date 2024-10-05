MODDIR=${0%/*}
# Don't modify anything after this
if [ -f $INFO ]; then
  while read LINE; do
    if [ "$(echo -n $LINE | tail -c 1)" == "~" ]; then
      continue
    elif [ -f "$LINE~" ]; then
      mv -f $LINE~ $LINE
    else
      rm -f $LINE
      while true; do
        LINE=$(dirname $LINE)
        [ "$(ls -A $LINE 2>/dev/null)" ] && break 1 || rm -rf $LINE
      done
    fi
  done < $INFO
  rm -f $INFO
fi

rm -rf /data/adb/bypass_charge.log
rm -rf /data/adb/bypass_charge.ini
cp $MODDIR/origin/night_charging /sys/class/power_supply/battery/
cp $MODDIR/origin/force_recharge /sys/class/power_supply/battery/
cp $MODDIR/origin/input_suspend /sys/class/power_supply/battery/
cp $MODDIR/origin/fast_charge_current /sys/class/power_supply/battery/
cp $MODDIR/origin/thermal_input_current /sys/class/power_supply/battery/