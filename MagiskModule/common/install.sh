echo "欢迎使用旁路供电模块"
echo "此模块可能会对您的设备造成一些不可消除的影响"
echo "因为此模块的功能尚未完全测试，可能存在一些bug"
echo "由于此模块所造成的损失，作者不承担任何责任"
echo "在使用时可能会出现一些问题，请谨慎操作"
echo "如有任何问题，请联系作者"
echo "如果您同意，请输入 y 继续安装"
read input
if [ "$input" == "y" ]; then
    echo "正在安装旁路供电模块"
else
    echo "安装已取消"
    exit 1
fi