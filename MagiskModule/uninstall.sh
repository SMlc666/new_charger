#!/bin/bash
MODDIR=${0%/*}

rm -rf /data/adb/bypass_charge.log
rm -rf /data/adb/bypass_charge.ini
cp -f $MODDIR/origin/input_suspend /sys/class/power_supply/battery/
cp -f $MODDIR/origin/fast_charge_current /sys/class/power_supply/battery/
cp -f $MODDIR/origin/thermal_input_current /sys/class/power_supply/battery/