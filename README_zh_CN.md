# 服务器动态风扇控制

该程序通过持续监控和 IPMI 控制来实现温度监测及风扇转速强制覆盖；目前支持以下功能：

[![zread](https://img.shields.io/badge/Ask_Zread-_.svg?style=flat&color=00b0aa&labelColor=000000&logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuOTYxNTYgMS42MDAxSDIuMjQxNTZDMS44ODgxIDEuNjAwMSAxLjYwMTU2IDEuODg2NjQgMS42MDE1NiAyLjI0MDFWNC45NjAxQzEuNjAxNTYgNS4zMTM1NiAxLjg4ODEgNS42MDAxIDIuMjQxNTYgNS42MDAxSDQuOTYxNTZDNS4zMTUwMiA1LjYwMDEgNS42MDE1NiA1LjMxMzU2IDUuNjAxNTYgNC45NjAxVjIuMjQwMUM1LjYwMTU2IDEuODg2NjQgNS4zMTUwMiAxLjYwMDEgNC45NjE1NiAxLjYwMDFaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00Ljk2MTU2IDEwLjM5OTlIMi4yNDE1NkMxLjg4ODEgMTAuMzk5OSAxLjYwMTU2IDEwLjY4NjQgMS42MDE1NiAxMS4wMzk5VjEzLjc1OTlDMS42MDE1NiAxNC4xMTM0IDEuODg4MSAxNC4zOTk5IDIuMjQxNTYgMTQuMzk5OUg0Ljk2MTU2QzUuMzE1MDIgMTQuMzk5OSA1LjYwMTU2IDE0LjExMzQgNS42MDE1NiAxMy43NTk5VjExLjAzOTlDNS42MDE1NiAxMC42ODY0IDUuMzE1MDIgMTAuMzk5OSA0Ljk2MTU2IDEwLjM5OTlaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik0xMy43NTg0IDEuNjAwMUgxMS4wMzg0QzEwLjY4NSAxLjYwMDEgMTAuMzk4NCAxLjg4NjY0IDEwLjM5ODQgMi4yNDAxVjQuOTYwMUMxMC4zOTg0IDUuMzEzNTYgMTAuNjg1IDUuNjAwMSAxMS4wMzg0IDUuNjAwMUgxMy43NTg0QzE0LjExMTkgNS42MDAxIDE0LjM5ODQgNS4zMTM1NiAxNC4zOTg0IDQuOTYwMVYyLjI0MDFDMTQuMzk4NCAxLjg4NjY0IDE0LjExMTkgMS42MDAxIDEzLjc1ODQgMS42MDAxWiIgZmlsbD0iI2ZmZiIvPgo8cGF0aCBkPSJNNCAxMkwxMiA0TDQgMTJaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00IDEyTDEyIDQiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLXdpZHRoPSIxLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8L3N2Zz4K&logoColor=ffffff)](https://zread.ai/jack9603301/DynamicFanControl)

- [x] 多曲线控制
- [x] 风扇设备级高级转速映射
- [x] 双阈值/迟滞控制（死区控制）
- [x] 支持死区状态（状态机锁定/迟滞）下的单向上升阶跃输入。

## 支持设备

- Dell（目前仅支持 Dell R720）

## 贡献指南

本项目采用完全符合 C++23 标准的风格，并须遵守以下规范：
1. 代码风格完全遵循 C++23 标准。
2. 所有类名均采用 PascalCase（大驼峰）命名规范。
3. 函数内的所有变量名均遵循 SnakeCase(蛇形) 命名规范。
4. 一个制表符（Tab）或代码缩进相当于 4 个空格。
5. 遵循树状目录结构，所有设备执行器（actuators）应放置在 `Devices` 目录下，并按服务器品牌名称组织子目录。

## 从源码编译

若要从源码进行安装，请执行以下命令；这将获取源码、进行编译并将其安装到您的系统中。

```
git clone https://github.com/jack9603301/DynamicFanControl
# 或 git clone git@github.com:jack9603301/DynamicFanControl.git
git checkout main
# or git checkout release-v{major}.{minor}
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_SYSCONFDIR=/etc -DCMAKE_INSTALL_LIBDIR=lib -DENABLE_SYSTEMD=ON ..
make
sudo make install
```

对于调试（Debug）版本的安装，我们不建议将其直接安装到系统目录中。调试版本的体积通常大于发布（Release）版本。
选择调试版本（而非标准的源码安装）意味着该构建专用于调试目的，这也暗示控制系统可能存在不稳定性。
请执行以下命令，这将获取源码、进行编译并将其安装到您的系统中。

```
git clone https://github.com/jack9603301/DynamicFanControl
# 或 git clone git@github.com:jack9603301/DynamicFanControl.git
git checkout main
# or git checkout release-v{major}.{minor}
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$PWD/dist/ -DCMAKE_BUILD_TYPE=Debug -DENABLE_CLANGD=ON -DENABLE_SYSTEMD=OFF ..
make
make install
```

对于调试版本，我们建议启用 `ENABLE_CLANGD` 选项；这将生成用于 LSP 解析的 `compile_commands.json` 文件，从而帮助您的 IDE（如 Neovim）定位代码符号定义。

编译选项说明如下：

- CMAKE_INSTALL_PREFIX：程序的安装路径。对于源码安装，通常设为 `/usr`。
- CMAKE_INSTALL_SYSCONFDIR：指定系统配置文件的安装路径。通常为 `/etc`。
- CMAKE_INSTALL_LIBDIR：指定库文件（libdir）的安装路径。该路径可以是相对于 `CMAKE_INSTALL_PREFIX` 的相对路径（通常为 `lib`）。
- CMAKE_BUILD_TYPE：构建类型，取决于您的 CMake 配置；通常选择以下值之一：
    - Debug
    - Release
- ENABLE_CLANGD：为 IDE 生成 compile_commands.json 文件
- ENABLE_SYSTEMD：启用 Systemd 支持

## 获取社区帮助

这是一个个人业余项目，如有需要，欢迎提交 Issue 或直接联系我。我会尽力尽快回复，但请勿期望能立即得到答复。

如果您希望提供对新设备的支持或者有更好的建议，欢迎您提出建议，或提交 Pull Request (PR) 以添加您认为有价值的功能。

邮箱：jack9603301@qhjack.top

## 捐赠
本项目是基于个人需求开发的，但如果您愿意进行个人捐赠，欢迎随时联系我。
我的邮箱地址是：jack9603301@qhjack.top

请注意，我仅接受法币捐赠，捐赠完成后，资金将作为个人收入处理。
如果您愿意提供支持，欢迎联系我进行捐赠——哪怕只是请我喝杯咖啡。
