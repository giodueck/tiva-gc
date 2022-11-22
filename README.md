# Tiva GC
Game console made with TM4C123 and the EduMkII BoosterPack.

# Building
Developed using Keil 5, compiler version 6.
All interfacing with the Tiva devices is done with direct register management (DRM), which also made me write a driver for the LCD for the project (with Energia's EduMkII drivers as the reference material).

# Status
This is an assignment project, so this is its final form as it proves difficult to work with DRM and Keil is very unconfortable to code in. The core ideas for the game console and the games themselves may be ported to Energia with abstracted interfacing to the pins and with Tivaware libraries being used.
