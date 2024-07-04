# 6502/65816

Integrated macro assembler, simulator and debugger for 650x microprocessor family. Allows you to write, test, and run 6502/65c02 programs.

# Status
![Build Status](https://github.com/kelmar/kowalski/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=wx_port)

# Building

Can be built with free Community Edition Visual Studio 2022.

This branch is currently being ported to wxWidgets and CMake so it can be built on multiple platforms.

## Ubuntu
You need to make sure you have wxWidgets installed with the development libraries:
```bash
sudo apt install -y libwxgtk3.0-gtk3-dev
```

You can then build once that is installed:
```bash
mkdir build
cd build
cmake ../kowalski/CMakeFiles.txt
cmake --build
```

# Windows
For Windows it is easist to just open the folder with the Visual Studio 2022 IDE and let it run CMake for you.

wxWidgets will be downloaded and compiled as part of the build.
