# VDP TMS9918A MSX SDCC Library (fR3eL Project)

<table>
<tr><td>Name</td><td>VDP_TMS9918A</td></tr>
<tr><td>Architecture</td><td>MSX</td></tr>
<tr><td>Environment</td><td>MSXDOS, ROM, MSXBASIC</td></tr>
<tr><td>Format</td><td>C Object (SDCC .rel)</td></tr>
<tr><td>Programming language</td><td>C and Z80 assembler</td></tr>
<tr><td>Compiler</td><td>SDCC v4.4 or newer</td></tr>
</table>

<br/>

## Description

C Library functions to work with the TMS9918A/28A/29A video processor.

This library is designed to develop applications for MSX computers in any of the different environments available (ROM, MSXDOS or MSXBASIC), using the Small Device C Compiler [(SDCC)](http://sdcc.sourceforge.net/) cross compiler.

It is optimized to offer the highest possible speed when using the TMS9918A VDP, especially in functions that work with data blocks (FillVRAM, CopyToVRAM and CopyFromVRAM). Fast read/write functions (FastVPOKE and FastVPEEK) have been added, which access the next used video memory cell.

You can complement it with any of these libraries:
- [VDP_PRINT](https://github.com/mvac7/fR3eL_VDP_PRINT_Lib) library with functions for display text strings in the graphic modes of the TMS9918A (G1 and G2).
- [VDP_SPRITES](https://github.com/mvac7/SDCC_VDP_SPRITES_Lib) Library of functions for directly accessing sprite attributes from the TMS9918A video processor.

You also have a [VDP_TMS9918A_MSXBIOS](https://github.com/mvac7/SDCC_VDP_TMS9918A_Lib) Library, developed using functions of the MSX BIOS. 
The advantage of using the BIOS is that the library is more compact and guarantees compatibility with all MSX models, but it has the disadvantage of being slow.

You can access the documentation here with [`How to use the library`](docs/HOWTO.md).

These libraries are part of the [MSX fR3eL Project](https://github.com/mvac7/SDCC_MSX_fR3eL).

This project is open source under the [MIT license](LICENSE). 
You can add part or all of this code in your application development or include it in other libraries/engines.

Enjoy it!   

<br/>

---

## History of versions

- v1.5 (11 December 2023):
	- Convert Assembler source code to C
	- Update to SDCC (4.1.12) Z80 calling conventions
	- Added SetVRAMtoREAD and SetVRAMtoWRITE functions
	- Added FastVPOKE and FastVPEEK functions
	- Added initialization of MC mode (in SCREEN function) with sorted map.
	- The order of input values in the VPOKE function has been reversed to optimize the function by taking advantage of the new Z80 calling conventions.
	- The FillVRAM, CopyToVRAM, and CopyFromVRAM functions have been optimized for faster access to VRAM.
- v1.4 (16 August 2022) Bug #2 (initialize VRAM address in V9938) and code optimization 
- v1.3 (23 July  2019) COLOR function improvements
- v1.2 ( 4 May   2019) 
- v1.1 (25 April 2019) 
- v1.0 (14 February 2014) Initial version

<br/>

---

## Requirements

- [Small Device C Compiler (SDCC) v4.4](http://sdcc.sourceforge.net/)
- [Hex2bin v2.5](http://hex2bin.sourceforge.net/)

<br/>

---

## Functions

### Initialization

| Name | Declaration | Description |
| ---  | ---         | ---         |
| SCREEN         | `SCREEN(char mode)` | Initializes the display |
| SortG2map      | `SortG2map()` | Initializes the pattern name table, with sorted values |
| SortMCmap      | `SortMCmap()` | Initializes the pattern name table, with sorted values |
| COLOR          | `COLOR(char ink, char background, char border)` | Set the foreground, background, and border screen colors |
| CLS            | `CLS()` | Clear Screen |

<br/>

### Access to the VDP

| Name | Declaration | Description |
| ---  | ---         | ---         |
| GetVDP         | `char GetVDP(char reg)` | Gets the value in a VDP register.<br/>Provides the mirror value stored in system variables. |
| SetVDP         | `SetVDP(char, char)` | Writes a value to a VDP register |

<br/>

### Access to video memory

| Name | Declaration | Description |
| ---  | ---         | ---         |
| VPOKE          | `VPOKE(unsigned int vaddr, char value)` | Writes a value to VRAM |
| FastVPOKE      | `FastVPOKE(char value)` | Writes a value to the next video RAM position |
| VPEEK          | `char VPEEK(unsigned int vaddr)` | Reads a value from VRAM |
| FastVPEEK      | `char FastVPEEK(char value)` | Reads the next video RAM value |
| FillVRAM       | `FillVRAM(unsigned int vaddr, unsigned int length, char value)` | Fill a large area of the VRAM of the same value |
| CopyToVRAM     | `CopyToVRAM(unsigned int addr, unsigned int vaddr, unsigned int length)` | Block transfer from memory to VRAM    |
| CopyFromVRAM   | `CopyFromVRAM(unsigned int vaddr, unsigned int addr, unsigned int length)` | Block transfer from VRAM to memory  |
| SetVDPtoREAD   | `SetVDPtoREAD(unsigned int vaddr)`  | Enable VDP to read (Similar to BIOS SETRD)   |
| SetVDPtoWRITE  | `SetVDPtoWRITE(unsigned int vaddr)` | Enable VDP to write (Similar to BIOS SETWRT) |

<br/>

### Sprites

| Name | Declaration | Description |
| ---  | ---         | ---         |
| ClearSprites   | `ClearSprites()` | Initialises the Sprite Attribute Table (OAM) and Sprite Pattern Table |
| SetSpritesSize | `SetSpritesSize(char size)` | Set size type for the sprites |
| SetSpritesZoom | `SetSpritesZoom(char zoom)` | Set zoom type for the sprites |
| PUTSPRITE      | `PUTSPRITE(char plane, char x, char y, char color, char pattern)` | Displays a sprite |
| GetSPRattrVADDR | `unsigned int GetSPRattrVADDR(char plane)` | Gets the VRAM address of the Sprite attributes of the specified plane |

<br/>

### Functions only accessible from inline assembler

| Label | Description | Input Regs. | Output Regs. | Affected Regs. |
| ---   | ---         | ---         | ---          | ---            |
| `_WriteByte2VRAM` | Writes a value to the video RAM.<br/>Same as VPOKE. | HL - VRAM address<br/>A - value | --- | A' |
| `_SetVDPtoWRITE` | Enable VDP to write.<br/>Similar to BIOS SETWRT. | HL - VRAM address | --- | A |
| `_FastVPOKE` | Writes a value to the next video RAM position. | A - value | --- | --- |
| `_VPEEK` | Reads a value from VRAM | HL - VRAM address | A | --- |
| `_SetVDPtoREAD` | Enable VDP to read.<br/>Similar to BIOS SETRD. | HL - VRAM address | --- | A |
| `_FastVPEEK` | Reads the next video RAM value. | --- | A | --- |
| `_fillVR` | Fill a large area of the VRAM of the same value | HL - VRAM address<br/>DE - Size<br/>A - value | --- | BC |
| `_LDIR2VRAM` | Block transfer from memory to VRAM | BC - blocklength<br/>DE - source Memory address<br/>HL - target VRAM address | --- | A |
| `_GetBLOCKfromVRAM` | Block transfer from VRAM to memory | BC - blocklength<br/>HL - source VRAM address<br/>DE - target RAM address | --- | A |
| `_GetSPRattrVADDR`  | Gets the VRAM address of the Sprite attributes of the specified plane | [A] sprite plane (0-31) | [HL] VRAM address |
| `_GetSpritePattern` | Returns the pattern value according to the Sprite size | [E] sprite pattern | [E] new pattern value | A |

<br/>

---

## Notes about operation

It's important to note that the SCREEN function doesn't behave like the BIOS functions with the same purpose (CHGMOD, INITXT, INIGRP, etc.). 
This is because the graphics will be provided by the developer, and there's no need to duplicate this work.

This function writes the same configuration of the different tables used in the MSX system to the VDP registers and fills the VRAM name table with the value 0.

The sprite attribute table is also initialized with all values ​​set to 0 except for the Y position, which is set to a hide position (209).

It doesn't set the MSX system font patterns in text modes (TEXT1 or GRAPHIC1). 
You'll need to copy the tileset required for your program to VRAM.

It's also important to note that, in GRAPHIC2 (Screen 2) and MULTICOLOR (Screen 3) modes, 
the name table will not be initialized with consecutive values ​​(usually used to display a graphic without using repeated tiles). 
For this case, you can use the SortG2map or SortMCmap functions.

The ink and background colors of the COLOR function are only useful in text mode, as the BIOS uses these values ​​to initialize the color table for the other modes. 
In all other modes, you can use this function to adjust the screen border color.

Because the VDP registers cannot be queried, writing their values ​​has been included in the system variables used by the MSX. 
If you want to adapt this library to another computer, you would need to remove it or move it to available memory.

<br/>

---

## Documentation

- Texas Instruments [TMS9918A application manual](http://map.grauw.nl/resources/video/texasinstruments_tms9918.pdf) `PDF`
- Texas Instruments [VDP Programmer’s Guide](http://map.grauw.nl/resources/video/ti-vdp-programmers-guide.pdf) `PDF`
- Texas Instruments [TMS9918A VDP](http://bifi.msxnet.org/msxnet/tech/tms9918a.txt) by Sean Young `TXT`
- The MSX Red Book · [2 Video Display Processor](https://github.com/gseidler/The-MSX-Red-Book/blob/master/the_msx_red_book.md#chapter_2) `HTML`
- YAMAHA [9938 Technical Data Book](http://map.grauw.nl/resources/video/v9938/v9938.xhtml) `HTML`
