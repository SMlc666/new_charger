MODPATH=${0%/*}/..
echo "欢迎使用旁路供电模块"
echo "此模块可能会对您的设备造成一些不可消除的影响"
echo "因为此模块的功能尚未完全测试，可能存在一些bug"
echo "由于此模块所造成的损失，作者不承担任何责任"
echo "在使用时可能会出现一些问题，请谨慎操作"
echo "如有任何问题，请联系作者"
echo "此安装脚本将会暂停30秒"
echo "如果不想安装,请关闭安装工具或关机"
sleep 30
echo "正在安装旁路供电模块"
cd $MODPATH
mkdir $MODPATH/origin
# 检查指定路径下的配置文件是否存在
if [ -f /data/adb/modules/bypass_charge/bypass_charge.ini ]; then
    echo "正在备份原有文件"
    cp /sys/class/power_supply/battery/night_charging $MODPATH/origin/
    cp /sys/class/power_supply/battery/force_recharge $MODPATH/origin/
    cp /sys/class/power_supply/battery/input_suspend $MODPATH/origin/
    cp /sys/class/power_supply/battery/fast_charge_current $MODPATH/origin/
    cp /sys/class/power_supply/battery/thermal_input_current $MODPATH/origin/
    mv $MODPATH/bypass_charge.ini /data/adb/
else
    echo "检测到此次是更新操作，不进行备份"
fi