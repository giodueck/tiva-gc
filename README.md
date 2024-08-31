# Tiva GC
Game console made with TM4C123 and the EduMkII BoosterPack.

# Status
The project could be built again, this time on Linux with GCC and no Keil or ARM compilers.
This may be a point to jump off of when building new things, or this project may be expanded. Who knows.

# Building
## Dependencies
Compiler, CMake and make:
```shell
sudo pacman -S arm-none-eabi-gcc cmake make
```

Flashing tool: lm4flash:
```shell
git clone https://github.com/utzig/lm4tools.git
cd lm4tools/lm4flash
make
```
or get it from the AUR:
```
git clone https://aur.archlinux.org/lm4tools-git.git
cd lm4tools-git
makepkg
```
then install with `sudo pacman -U <output.pkg.tar.zst>`.

Miscelaneous: libusb and unzip
```shell
sudo pacman -S libusb unzip
```

Tivaware: get full .exe from http://software-dl.ti.com/tiva-c/SW-TM4C/latest/index_FDS.html, then
```shell
mkdir tivaware; cd tivaware
mv ~/Downloads/SW-TM4C-2.1.4.178.exe .
unzip SW-TM4C-2.1.4.178.exe
cd driverlib
make
```
The `driverlib` and `inc` directories were copied into the project for a quick and dirty integration.

## Setting up build
To run the CMakeLists script, do
```shell
mkdir build
cmake -B./build -S.
```

## Building and flashing
```shell
cd build
make
make flash
```

# Original Readme
---
# Building
Developed using Keil 5, compiler version 6.
All interfacing with the Tiva devices is done with direct register management (DRM), which also made me write a driver for the LCD for the project (with Energia's EduMkII drivers as the reference material).

# Status
This is an assignment project, so this is its final form as it proves difficult to work with DRM and Keil is very unconfortable to code in. The core ideas for the game console and the games themselves may be ported to Energia with abstracted interfacing to the pins and with Tivaware libraries being used.
