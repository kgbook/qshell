# How to Build qshell

# linux (ubuntu22)
## install dependencies
```shell
sudo apt install git cmake build-essential qt6-base-dev libqt6serialport6-dev clang-tidy 
```

## checkout source
```shell
git clone https://github.com/qiushao/qshell.git
```

## build & install
```shell
cmake -B build -S .
cmake --build build
sudo cmake --install build
```

# windows
## 环境配置
windows 开发 qt 环境配置比较复杂，我们采用的是 msvc2022 + qt6 online installer

## build & package
在仓库根目录打开 powershell，执行 windows 的编译脚本即可
```shell
.\scripts\windows-build.ps1
```

# macos
暂无环境测试，不确定要安装哪些依赖， 但编译步骤应该跟 linux 是一样的