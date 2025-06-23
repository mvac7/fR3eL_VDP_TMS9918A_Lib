# VDP TMS9918A MSX SDCC Library (fR3eL Project)

```
Author: mvac7
Architecture: MSX
Format: C Object (SDCC .rel)
Programming language: C and Z80 assembler
```


## Description

Open Source library with basic functions to work with the TMS9918A/28A/29A video processor.

It does not use the MSX BIOS but it does require system variables, so it can be used for any MSX application although it is designed for the MSX-DOS environment.

Use them for developing MSX applications using [Small Device C Compiler (SDCC)](http://sdcc.sourceforge.net/) cross compiler.

You can access the documentation here with [`How to use the library`](docs/HOWTO.md).

Within this project you will find the [`examples/`](examples/) folder with applications for testing and learning purposes.

This library is part of the [MSX fR3eL Project](https://github.com/mvac7/SDCC_MSX_fR3eL).

Enjoy it!



## History of versions
- v1.5 (11/12/2023) 
	- Update to SDCC (4.1.12) Z80 calling conventions
	- Added SetVRAMtoREAD and SetVRAMtoWRITE functions
	- Added FastVPOKE and FastVPEEK functions
	- Added initialization of MC mode (in SCREEN function) with sorted map.
	- The order of input values in the VPOKE function has been reversed to optimize the function by taking advantage of the new Z80 calling conventions.
- v1.4 (16 August 2022) Bug #2 (initialize VRAM address in V9938) and code optimization 
- v1.3 (23 July  2019) COLOR function improvements
- v1.2 ( 4 May   2019) 
- v1.1 (25 April 2019) 
- v1.0 (14 February 2014) Initial version



## Requirements

- [Small Device C Compiler (SDCC) v4.4](http://sdcc.sourceforge.net/)
- [Hex2bin v2.5](http://hex2bin.sourceforge.net/)




## Notes about operation

It is important to know that the SCREEN function does not behave exactly like 
the functions of the BIOS with the same purpose (CHGMOD, INITXT, INIGRP, etc.).
SCREEN does not clean the entire VRAM and does not set the patterns from the MSX font in text modes. 
This function changes to the indicated screen mode, writes to the registers of the VDP the same configuration of the different tables used 
in the MSX and fill the Name Table and the Sprite attribute table with the value 0 and the Y position for hiding (209).

It is also necessary to know that in the case of graphic mode 2 (screen 2), the table of names will not be initialized, 
with consecutive values (normally used to display a graphic without the use of repeated tiles).

Due to the fact that the VDP registers can not be consulted, the writing of the values of these has been included in the system variables used by the MSX. 
In the case of wanting to adapt this library to another computer, they would have to be deleted or placed in the memory area that is available.

The colors of ink and background of the COLOR function are only useful in text mode, 
since the BIOS uses these values to initialize the color table in the screen startup routines and this library does not. 
In all other modes it is useful to adjust the border color of the screen.
