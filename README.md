# 6502

Integrated macro assembler, simulator and debugger for 650x microprocessor family. Allows you to write, test, and run 6502/65c02 programs.

Can be built with free Community Edition Visual Studio 2022.

This branch is currently being ported to wxWidgets and CMake so it can be built on multiple platforms.

Build works now for Linux and Windows:
```base
mkdir build
cd build
cmake ../kowalski/CMakeFiles.txt
cmake --build
```

If you are using Visual Studio 2022 Community edition, you can also open the folder from the IDE.
It will pick up the cmake configs and set everything up accordingly.  Once it's done you should
be able to build and run strate from Visual Studio!
