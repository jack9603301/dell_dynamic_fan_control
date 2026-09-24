# Server Dynamic Fan Control

This program implements temperature monitoring and forced fan overrides through continuous monitoring and IPMI control; it currently supports the following features:

[![zread](https://img.shields.io/badge/Ask_Zread-_.svg?style=flat&color=00b0aa&labelColor=000000&logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuOTYxNTYgMS42MDAxSDIuMjQxNTZDMS44ODgxIDEuNjAwMSAxLjYwMTU2IDEuODg2NjQgMS42MDE1NiAyLjI0MDFWNC45NjAxQzEuNjAxNTYgNS4zMTM1NiAxLjg4ODEgNS42MDAxIDIuMjQxNTYgNS42MDAxSDQuOTYxNTZDNS4zMTUwMiA1LjYwMDEgNS42MDE1NiA1LjMxMzU2IDUuNjAxNTYgNC45NjAxVjIuMjQwMUM1LjYwMTU2IDEuODg2NjQgNS4zMTUwMiAxLjYwMDEgNC45NjE1NiAxLjYwMDFaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00Ljk2MTU2IDEwLjM5OTlIMi4yNDE1NkMxLjg4ODEgMTAuMzk5OSAxLjYwMTU2IDEwLjY4NjQgMS42MDE1NiAxMS4wMzk5VjEzLjc1OTlDMS42MDE1NiAxNC4xMTM0IDEuODg4MSAxNC4zOTk5IDIuMjQxNTYgMTQuMzk5OUg0Ljk2MTU2QzUuMzE1MDIgMTQuMzk5OSA1LjYwMTU2IDE0LjExMzQgNS42MDE1NiAxMy43NTk5VjExLjAzOTlDNS42MDE1NiAxMC42ODY0IDUuMzE1MDIgMTAuMzk5OSA0Ljk2MTU2IDEwLjM5OTlaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik0xMy43NTg0IDEuNjAwMUgxMS4wMzg0QzEwLjY4NSAxLjYwMDEgMTAuMzk4NCAxLjg4NjY0IDEwLjM5ODQgMi4yNDAxVjQuOTYwMUMxMC4zOTg0IDUuMzEzNTYgMTAuNjg1IDUuNjAwMSAxMS4wMzg0IDUuNjAwMUgxMy43NTg0QzE0LjExMTkgNS42MDAxIDE0LjM5ODQgNS4zMTM1NiAxNC4zOTg0IDQuOTYwMVYyLjI0MDFDMTQuMzk4NCAxLjg4NjY0IDE0LjExMTkgMS42MDAxIDEzLjc1ODQgMS42MDAxWiIgZmlsbD0iI2ZmZiIvPgo8cGF0aCBkPSJNNCAxMkwxMiA0TDQgMTJaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00IDEyTDEyIDQiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLXdpZHRoPSIxLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8L3N2Zz4K&logoColor=ffffff)](https://zread.ai/jack9603301/DynamicFanControl)

- [x] Multi-curve control
- [x] Fan Device-Level Advanced Fan Speed ​​Mapping
- [x] Dual-threshold/Hysteresis control (dead-band control)
- [x] Support for unidirectional rising step input in the dead-zone (state machine lockout/hysteresis) state.

## Devices

- Dell(Currently only on the Dell R720)

## Contribution Guidelines

This program adheres to a fully C++23 style and must comply with the following specifications:
1. This program adheres to a fully C++23 style and must comply with the following specifications:
2. All class names follow the PascalCase naming convention.
3. Variable names within all functions follow the convention of lowercase letters separated by underscores.
4. One tab/code indentation equals 4 spaces.
5. Following the tree-like directory structure, all device actuators should be placed under the "Devices" directory, organized into subdirectories named after the server brands.

## Compiling from source

For a source-based installation, you should execute the following commands; this will fetch, compile, and install it on your system.

```
git clone https://github.com/jack9603301/DynamicFanControl
# or git clone git@github.com:jack9603301/DynamicFanControl.git
git checkout release-v{major}.{minor}
# or git checkout main
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

For a debug installation, we do not recommend installing it directly onto your system. Debug builds are typically larger than release versions,
and choosing a debug build—rather than installing from source—indicates that the build is intended specifically for debugging purposes, which implies that the control system may be unstable.
You should execute the following command; it will pull, compile, and install the software onto your system.

```
git clone https://github.com/jack9603301/DynamicFanControl
# or git clone git@github.com:jack9603301/DynamicFanControl.git
git checkout release-v{major}.{minor}
# or git checkout main
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$PWD/dist/ -DCMAKE_BUILD_TYPE=Debug -DENABLE_CLANGD=ON ..
make
make install
```

For debug builds, we recommend enabling the `ENABLE_CLANGD` option; this generates a `compile_commands.json` file for LSP parsing, which helps your IDE (such as Neovim) locate code symbol definitions.

The explanations for the compilation options are as follows:

- CMAKE_INSTALL_PREFIX: The installation path for this program. for source-based installations, it is typically `/usr`.
- CMAKE_BUILD_TYPE: The build type depends on your CMake configuration; we typically select the following value:
    - Debug
    - Release
- ENABLE_CLANGD: Generate compile_commands.json for IDEs

## Get help from the community

This is a personal hobby project; feel free to open issues or contact me directly for assistance. While I strive to respond as quickly as possible, please do not expect an immediate reply.

I may not be able to provide support for every scenario, as the software was created solely to handle server fan control and noise reduction. 
You are welcome to share any suggestions or submit a pull request (PR) to add features you find valuable.

email: jack9603301@qhjack.top

## Donate
This project was developed based on personal needs, but if you would like to make a personal donation, please feel free to contact me.
my email address is jack9603301@qhjack.top

However, I only accept donations made in fiat currency; once the donation is complete, the funds are treated as personal income. 
If you would like to offer support, please feel free to contact me to make a donation—you could even just buy me a coffee.
