# VexV5NanoboyAdvance

This is a port of [NanoboyAdvance](https://github.com/flerovii/NanoboyAdvance) for the Vex V5, using the [PROS](https://github.com/purduesigbots/pros) kernel.  It can play most GBA games.


This is a work in progress. I'm working on improving the performance. It currently can't save games.

## How to use
* Compile and upload the project to the V5
* Add `bios.bin` to the microSD card
* Add a GBA ROM to the microSD card named "game.gba" (will be able to choose roms in the future)
* Start the program

It might take 10-20sec for the ROM to load. If you want to see what's happening, use `pros terminal` to see its output.

Also there's a chance that the ROM might be too big for the heap, but I haven't tested it yet.
