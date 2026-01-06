PIC Blocks
==========

This is a handheld game I made as a silly Christmas gift for my family. It's a clone of the popular falling tetromino game, with different skins for different family members.

Firmware is located in `pic-game.X`, made with MPLAB X IDE.

Image assets are located in `img`.

3D-printed case is in `case`.

Building
--------

`build.bin` is a binary that can be programmed to a PIC16F15325.

To build the firmware:
- Run `python tools/prepare_data.py` to prepare graphics data.
- Open `pic-game.X` in MPLAB X and hit build.
- Run `python tools/program.py -b` to combine code and data into a binary and save to `build.bin`.
