# 新的旁路供电模块

## 介绍

欢迎使用我们全新的旁路供电模块！
该模块主要为Redmi Note10Pro(chopin)提供一个电源管理方案
用于改善在一些场景的发热情况
希望各位踊跃参与提交代码（点点star也是可以哒）😊

## 编译

- **环境配置**
  - 设备: x86_64
  - 系统: Ubuntu 24.04 LTS
  - 工具链: Android-NDK r27
- **编译**
  - Topic 1:
    - 安装xmake(构建管理) Android-NDK r27(配置方法见xmake官方文档) or 其他交叉编译工具链
  - Topic 2:
    - git clone https://github.com/SMlc666/new_charger.git(git这个项目地址)
    - cd new_charger(或者你指定的项目文件夹)
    - xmake f -a arm64-v8a -m release -p android --ndk=your_ndk_dir(配置构建管理)
    - xmake(开始构建)