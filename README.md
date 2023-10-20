## C8
A simple Chip-8 and Super Chip-8 emulator written in C and
![raylib](https://github.com/raysan5/raylib)

### Installation
In order to intall the emulator, raylib must be installed on the system. For
more information on how to install the library follow ![this
link](https://github.com/raysan5/raylib#build-and-installation). The `Makefile`
uses `clang`, but it should work also on `gcc`; `pkg-config` is required in
order to locate the necessary flags for compiling and linking with raylib.

After installing raylib, on the shell use:
```shell
$ make
```
to build the executable
```shell
$ make run
```
to build, if not already built, and run, and
```shell
$ make clean && make
```
to make a clean build.

### Resources
![guide followed](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)  
![test ROM](https://github.com/corax89/chip8-test-rom)  
![more information on the topic](https://chip-8.github.io/links/)  
![more CC0 ROMs to test the emulator](https://johnearnest.github.io/chip8Archive/)  
