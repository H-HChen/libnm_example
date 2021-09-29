# libnm_example

## Introduction
C++ example of libnm(Network Manager) to manipulate internet connection setting.

For API reference please check https://github.com/NetworkManager/NetworkManager.git

## Prerequisite
```bash
sudo apt install libnm-dev libglib2.0-dev
```

## Build
```
cd ~
git clone https://github.com/H-HChen/libnm_example.git
cd ~/libnm_example/example
cmake -Bbuild -H.
cmake --build build
```