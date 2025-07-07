/* ==============================================================================                                                                            
# VDP_TMS9918A MSX Library (fR3eL Project)

- Version: 1.5 (11/12/2023)
- Author: mvac7/303bcn
- Architecture: MSX
- Environment: ROM, MSX-DOS or BASIC
- Format: SDCC Relocatable object file (.rel)
- Programming language: C and Z80 assembler
- Compiler: SDCC 4.4 or newer 

## Description:                                                              
Opensource library for acces to VDP TMS9918A/28A/29A

Features:
- Not use the BIOS 
- It does not copy the tileset with the MSX system font
  Requires your own tileset or copying the one from the BIOS
- Using the ports 0x98 and 0x99 from MSX computers.
- Uses the memory settings of the display modes set in MSX computers System
- Save VDP registers values in MSX System variables	
 
## History of versions (dd/mm/yyyy):
- v1.5 (11/12/2023) Convert Assembler source code to C
					Update to SDCC (4.1.12) Z80 calling conventions
					Added SetVDPtoREAD and SetVDPtoWRITE functions
					Added FastVPOKE and FastVPEEK functions
					Added initialization of MULTICOLOR mode (in SCREEN function) 
					with sorted map.
					Added initialization of the color table in GRAPHIC1 mode 
					(based on the values ​​previously given by the COLOR function).
					The FillVRAM, CopyToVRAM, and CopyFromVRAM functions 
					have been optimized for faster access to VRAM.
- v1.4 (16/08/2022) Bug#2 (init VRAM addr in V9938) and code optimization 
- v1.3 (23/07/2019) COLOR function improvements
- v1.2 (04/05/2019)
- v1.1 (25/04/2019) 
- v1.0 (14/02/2014)                                                                             
============================================================================== */

#include "../include/msxSystemVariables.h"

#include "../include/VDP_TMS9918A.h"






/* =============================================================================
SCREEN
Description:
		Initializes the display to one of the four standardized modes on the MSX.
		- All screen modes will be initialized with the pattern name table set 
		  to 0, just like the CLS function.  
		- Initialization of the color table in GRAPHIC1 mode 
		  (based on the values ​​previously given by the COLOR function).
		- Initializing the Sprite Attribute Table (OAM) in graphic modes.
		
Input:	[char] number of screen mode
			0 = Text1
			1 = Graphic1
			2 = Graphic2
			3 = MultiColor
Output:	-
============================================================================= */
void SCREEN(char mode) __naked
{
mode;	//A
__asm
	push IX

	xor  A
	jr   Z,TMS_screen0$
	cp   #1
	jr   Z,TMS_screen1$
	cp   #3
	jr   Z,TMS_screen3$

//TMS_screen2$:
	call ClearG1G2
	call _ClearSprites      
	ld   HL,#mode_GFX2
	jr   TMS_setREGs$

  
TMS_screen0$:
	call ClearT1

	ld   HL,#mode_TXT1
	;screen 0 > 40 columns mode
	ld   A,#39  ;default value
	ld   (#LINL40),A
	jr   TMS_setREGs$

TMS_screen1$:
	call TMS_initG1$
	call ClearG1G2
	call _ClearSprites    
	ld   HL,#mode_GFX1
	jr   TMS_setREGs$
	
TMS_screen3$:
	call ClearMC
	call _ClearSprites  
	ld   HL,#mode_MC

TMS_setREGs$:
	ld   B,#7
	ld   C,#0
TMS_REGSloop$:
	ld   A,(HL)
	call writeVDP
	inc  HL
	inc  C
	djnz TMS_REGSloop$
  
//------------------------------------------------------------------------------
//initialize VRAM access on MSX2 or higher (V9938)
	LD   HL,#MSXID3
	LD   A,(#EXPTBL)		//EXPTBL=main BIOS-ROM slot address
	CALL 0x000C				//RDSLTReads the value of an address in another slot
	EI
	or   A   
	jr   Z,TMS_screenEND$
  
//clear upper bits (A14,A15,A16) from VRAM address for only acces to first 16k
	xor  A
	out  (VDPSTATUS),A		//clear three upper bits for 16bit VRAM ADDR (128K)
	ld   A,#14+128			//V9938 reg 14 - Control Register
	out  (VDPSTATUS),A
  
TMS_screenEND$: 
	pop  IX
	ret
//------------------------------------------------------------------------------



/* --------------------------------------------------------------
Writes in the color table, with the last values of the ink and
background indicated with the COLOR function
-------------------------------------------------------------- */
TMS_initG1$:
	ld   A,(#BAKCLR)
	ld   B,A 
	ld   A,(#FORCLR)
	sla  A
	sla  A
	sla  A
	sla  A
	or   B
	ld   DE,#32
	ld   HL,#G1_COL
	jp   fillVR			;fill color table with Ink+BG Colors
// --------------------------------------------------------------




/* --------------------------------------------------------------
Screens data  

Reg/Bit  7     6     5     4     3     2     1     0
0        -     -     -     -     -     -     M3    EXTVID
1        4/16K	BLK   GINT	M1    M2    -     SIZE  MAG
-------------------------------------------------------------- */

//Text1> M1=1; M2=0; M3=0  
mode_TXT1:
 .db 0B00000000	//reg0 $00 
 .db 0B11110000 //reg1 $F0 
 .db 0x00		//reg2 Name Table				(0000h)
 .db 0x00		//reg3 --
 .db 0x01		//reg4 Pattern Table			(0800h)
 .db 0x00		//reg5 --
 .db 0x00		//reg6 --

//Graphic1> M1=0; M2=0; M3=0  
mode_GFX1:
 .db 0B00000000 //reg0 $00
 .db 0B11100000 //reg1 Default sprites 8x8 No Zoom
 .db 0x06		//reg2 Name Table				(1800h)
 .db 0x80		//reg3 Color Table				(2000h)
 .db 0x00		//reg4 Pattern Table			(0000h)
 .db 0x36		//reg5 Sprite Attribute Table	(1B00h)
 .db 0x07		//reg6 Sprite Pattern Table		(3800h)

//Graphic2> M1=0; M2=0; M3=1  
mode_GFX2:
 .db 0B00000010 //reg0
 .db 0B11100000 //reg1 Default sprites 8x8 No Zoom
 .db 0x06		//reg2 Name Table				(1800h)
 .db 0xFF		//reg3 Color Table				(2000h)
 .db 0x03		//reg4 Pattern Table			(0000h)
 .db 0x36		//reg5 Sprite Attribute Table	(1B00h)
 .db 0x07		//reg6 Sprite Pattern Table		(3800h)

//MultiColor> M1=0; M2=1; M3=0
mode_MC:
 .db 0B00000000	//reg0 $00
 .db 0B11101000	//reg1 $E8 Default sprites 8x8 No Zoom
 .db 0x02		//reg2 Name Table				(0800h)
 .db 0x00		//reg3 Pattern Table			(0000h)
 .db 0x00		//reg4 --
 .db 0x36		//reg5 Sprite Attribute Table	(1B00h)
 .db 0x07		//reg6 Sprite Pattern Table		(3800h)  
// --------------------------------------------------------------
__endasm;
} 




/* =============================================================================
SortG2map 
Description: 
		Initializes the pattern name table, with sorted values. 
		Designed to be able to display a Graphic2 (256x192px) image.
Input:	-
Output:	-
============================================================================= */
void SortG2map(void) __naked
{
__asm
	ld   HL,#G2_MAP
	call _SetVDPtoWRITE
	ld	 DE,#0x0300
	xor  A
TMSinitG2_loop$:
	out  (VDPVRAM),A
	inc  A
	dec  E
	jp   NZ,TMSinitG2_loop$
	dec  D
	jp   NZ,TMSinitG2_loop$
	ret	
__endasm;	
}



/* =============================================================================
SortMCmap 
Description: 
		Initializes the pattern name table, with sorted values. 
		Designed to be able to display a MultiColor (64x48 blocks) image.
Input:	-
Output:	-
============================================================================= */
void SortMCmap(void) __naked
{
__asm
	ld   HL,#MC_MAP
	call _SetVDPtoWRITE  //init VDP to Write to BASE15
	
	ld   C,#0	//row
	ld   D,#24
TMS_ROWloop$:	
	ld   A,C
	
	SRL  A 
	SRL  A		//row/4
	SLA  A
	SLA  A
	SLA  A
	SLA  A
	SLA  A		//row*32
	
	ld   B,#32
TMS_printROWloop$:
	out  (VDPVRAM),A
	inc  A
	djnz TMS_printROWloop$
	
	inc  C
	dec  D
	jp   NZ,TMS_ROWloop$
		
	ret
__endasm;	
}



/* =============================================================================
CLS 
Description: 
		 Clear Screen
		 Fill VRAM Name Table with the value 0
Input:	-
Output:	-
============================================================================= */
void CLS(void) __naked
{
__asm

	ld   A,(#RG0SAV+1)	;reg1
	bit  4,A			;M1=1 Text1
	jr   NZ,ClearT1
	bit  3,A			;M2=1 MultiColor
	jr   NZ,ClearMC

//	ld   A,(#RG0SAV)	;reg0  
//	bit  1,A			;M3=1 G2
//	jr   NZ,ClearG2

ClearG1G2::
	xor  A
	ld   DE,#0x300		//32*24
	ld   HL,#G1_MAP
	jp   fillVR
  
ClearMC::
	xor  A
	ld   DE,#0x300		//32*24 (2x2 blocks)
	ld   HL,#MC_MAP		//Name Table
	jp   fillVR
  
ClearT1::
	xor  A
	ld   DE,#0x3C0		//40*24
	ld   HL,#T1_MAP
	jp   fillVR
  
__endasm;
} 



/* =============================================================================
COLOR
Description:
		Specifies the ink, foreground and background colors.
		This function has different behaviors depending on the screen mode.
		In Text1 mode, the color change is instantaneous except the 
		border color which has no effect.
		In Graphic1, Graphic2 and Multicolor modes, only the border color has 
		an instant effect. 
		Ink and background colors are only used when starting the screen in
		Graphic1 mode.

Input:	[char] ink color
		[char] background color
		[char] border color
Output:	-     
============================================================================= */
void COLOR(char ink, char background, char border)
{
ink;		//A
background;	//L
border;		//STack
__asm
	push IX
	ld   IX,#0
	add  IX,SP

;save values in system vars
	ld   (#FORCLR),A		//ink color (foreground) 
	ld   A,L
	ld   (#BAKCLR),A		//background color
	ld   A,4(IX)  
	ld   (#BDRCLR),A		//border color

	ld   A,(#RG0SAV+1)		//reads the last value of register 1 of the VDP in the system variables
	bit  4,A				//M1=1   IF screen0?
	jr   NZ,TMS_colorMode0$

	ld   A,(#BDRCLR)
	ld   B,A  
	ld   A,(#BAKCLR)
	jr   TMS_SAVEcolorREG$

TMS_colorMode0$:
	ld   A,(#BAKCLR)
	ld   B,A 
	ld   A,(#FORCLR)

TMS_SAVEcolorREG$:
	sla  A
	sla  A
	sla  A
	sla  A
	or   B

/* --------------------------------------------------------------------------  
(info by Portar Doc)
Register 7: colour register.
  Bit  Name  Expl.
  0-3  TC0-3 Background colour in SCREEN 0 (also border colour in SCREEN 1-3)
  4-7  BD0-3 Foreground colour in SCREEN 0      
-------------------------------------------------------------------------- */  
	ld   C,#0x07	//VDP reg 7
	call writeVDP

	pop  IX
  
__endasm;
} 

	

/* =============================================================================
VPOKE
Description:
		Writes a value to the video RAM. 
Input:	[unsigned int] VRAM address
		[char] value		
Output:	- 
============================================================================= */
void VPOKE(unsigned int vaddr, char value)
{
vaddr;	//HL
value;	//Stack
__asm
	push IX
	ld   IX,#0
	add  IX,SP
  
	call _SetVDPtoWRITE
	
	ld   A,4(IX)
	call _FastVPOKE
	pop  IX
__endasm;
}



/* =============================================================================
VPEEK
Description:
		Reads a value from video RAM. 
Input:	[unsigned int] VRAM address
Output:	[char] value
============================================================================= */
char VPEEK(unsigned int vaddr) __naked
{
vaddr;	//HL
__asm	


/* =============================================================================
ReadByteFromVRAM                                
Description:
		Reads a value from video RAM.
Input:	HL - VRAM address
Output:	A - value
Regs:	-
============================================================================= */
ReadByteFromVRAM::
	call _SetVDPtoREAD
  
/* =============================================================================
FastVPEEK                                
Description:
		Reads the value from the last position in video RAM and increments it.
		This is a fast way to read consecutive values ​​from VRAM.
		It requires the VDP to be in read mode, using the SetVDPtoREAD or VPEEK 
		function at the beginning of the sequence.
Input    : --
Output   : A - value
============================================================================= */ 
_FastVPEEK::  
	in   A,(VDPVRAM)

	ret 




/* =============================================================================
WriteByteToVRAM                                
Description:
		Writes a value to the video RAM. Same as VPOKE.
Input:	HL - VRAM address
		A - value
Output:	-
Regs:	A'
============================================================================= */
WriteByteToVRAM::
	ex   AF,AF
	call _SetVDPtoWRITE
	ex   AF,AF
	
/* =============================================================================
FastVPOKE                                
Description:
		Writes a value to the last position in video RAM and increments it.
		This is a fast way to write consecutive values ​​to VRAM.
		Requires the VDP to be in write mode, using the SetVDPtoWRITE or VPOKE 
		function at the beginning of the sequence.
Input:	A - value
Output:	-
============================================================================= */
_FastVPOKE::  
	out  (VDPVRAM),A  
	ret

__endasm;
} 



/* =============================================================================
FillVRAM                               
Description:
		Fills an area of ​​VRAM with the same value.
Input:	[unsigned int] VRAM address
		[unsigned int] block size
		[char] Value to fill
Output:	- 
============================================================================= */
void FillVRAM(unsigned int vaddr, unsigned int size, char value)
{
vaddr;	//HL
size;	//DE
value;	//STack
__asm
	push IX
	ld   IX,#0
	add  IX,SP

	ld   A,4(IX) ;value

	call fillVR

	pop  IX
__endasm;
} 



/* =============================================================================
CopyToVRAM
Description:
		Block transfer from memory to VRAM 
Input:	[unsigned int] Memory address
		[unsigned int] VRAM address
		[unsigned int] block size
Output:	- 
============================================================================= */
void CopyToVRAM(unsigned int addr, unsigned int vaddr, unsigned int size)
{
addr;	//HL
vaddr;	//DE
size;	//STack
__asm
	push IX
	ld   IX,#0
	add  IX,SP  

	ex   DE,HL

	ld   C,4(IX)	//size
	ld   B,5(IX)

	call LDIR2VRAM

	pop  IX
__endasm;
} 



/* =============================================================================
CopyFromVRAM
Description:
		Block transfer from VRAM to memory
Input:	[unsigned int] VRAM address                     
		[unsigned int] RAM address
		[unsigned int] block size
Output:	-
============================================================================= */
void CopyFromVRAM(unsigned int vaddr, unsigned int addr, unsigned int size)
{
vaddr;	//HL
addr;	//DE
size;	//STack
__asm
  push IX
  ld   IX,#0
  add  IX,SP
  
  ld   C,4(IX)	//length
  ld   B,5(IX)
    
  call GetBLOCKfromVRAM
    
  pop  IX
__endasm;
}   



/* =============================================================================
GetVDP
Description:
		Gets the value in a VDP register.
		Provides the mirror value of a VDP register stored in system variables.
Input:	[char] register number (0-7)           
Output:	[char] value
Regs:	HL,DE
============================================================================= */
char GetVDP(char reg) __naked
{
reg;	//A
__asm
readVDP::
	ld   HL,#RG0SAV		//Mirror of VDP register 1
	ld   E,A
	ld   D,#0
	add  HL,DE
	ld   A,(HL)
	ret
__endasm;		
}



/* =============================================================================
SetVDP
Description:
		Writes a value to a VDP register and 
		saves the value in the system variables.
Input:	[char] register number (0-7)                    
		[char] value
Output:	-
============================================================================= */
void SetVDP(char reg, char value) __naked
{
reg;	//A
value;	//L
__asm      
	ld   C,A	//reg
	ld   A,L	//value
  

/* =============================================================================
writeVDP
Description:
		Writes a value to a VDP register
Input:	A  - value
        C  - register number (0-7) 
Output:	-
Registers: IY, DE
============================================================================= */
writeVDP::

	ld   IY,#RG0SAV
	ld   E,C
	ld   D,#0
	add  IY,DE
	ld   (IY),A ;save copy of vdp value in system variable

	di
	out  (VDPSTATUS),A
	ld   A,C
	or   #0b10000000	;add 128 to VDP register number
	out  (VDPSTATUS),A
	ei
	ret



;#####################################################
;# I add here routines that cannot be added          #
;# to their functions because these functions        #
;# use the stack to pass in some values,             #
;# it is necessary for the compiler to add code to   #
;# the end of the function to recover the thread of  #
;# execution.                                        #
;#####         #######################################
;    #         #
;    #         #
;    #         #
;    #         #
; ####         ####
;  #             #
;   #           #
;    #         #
;     #       #
;      #     #
;       #   #
;        # #
;         #

/* =============================================================================
fillVR                                
Description:
		Fills an area of ​​VRAM with the same value.
Input:	HL - VRAM address
		DE - Size
		A  - value
Output:	-
Regs:	BC
============================================================================= */
fillVR::
  
	ld   C,A

	call _SetVDPtoWRITE  
      
	ld   A,C 
	ld   C,#VDPVRAM
	
    ld   B,E
    dec  DE					//IF E=0 then D--
    inc  D	
TMS_VFILLloop$:
	nop						//( 5ts)
    out  (VDPVRAM),A		//(12ts)   14+5+12 = 31ts
    djnz TMS_VFILLloop$		//(14/9ts)
    dec  D
    jp   NZ,TMS_VFILLloop$
	ret


/* =============================================================================
LDIR2VRAM
Description:
		Block transfer from memory to VRAM 
Input:	DE - source Memory address
		HL - target VRAM address
		BC - block size
Output:	-
Regs:	A
============================================================================= */
LDIR2VRAM::

	call _SetVDPtoWRITE

	ex   DE,HL

	ld   D,B
	ld   E,C
		
	ld   C,#VDPVRAM
  
    ld   B,E
    dec  DE				//IF E=0 then D--
    inc  D	
TMS_COPYBYTE_loop:
	outi						//(18ts) out [c],[HL] + INC HL + dec B
    jp   NZ,TMS_COPYBYTE_loop	//(11ts) 29 T-States (18 + 11)
	
    dec  D
    jp   NZ,TMS_COPYBYTE_loop
	ret
    

        
/* =============================================================================
GetBLOCKfromVRAM
Description: 
		Block transfer from VRAM to memory.  
Input:	HL - source VRAM address                     
		DE - target RAM address
		BC - block size
Output:	-
Regs:	A        
============================================================================= */
GetBLOCKfromVRAM::
	call _SetVDPtoREAD

	ex   DE,HL

	ld   D,B
	ld   E,C

	ld   C,#VDPVRAM

    ld   B,E
    dec  DE				//IF E=0 then D--
    inc  D	
VREAD_loop:
	ini           		//read value from C port, write in [HL] and INC HL
    jp   NZ,VREAD_loop	//(11ts) 29 T-States (18 + 11)
	
    dec  D
    jp   NZ,VREAD_loop
	ret

__endasm;
}   



/* =============================================================================
SetVDPtoREAD
Description:
		Enable VDP to read and indicates the VRAM address where the reading 
		will be performed.
Input:	[unsigned int] VRAM address
Output:	-
Regs:	A
============================================================================= */
void SetVDPtoREAD(unsigned int vaddr) __naked
{
vaddr;	//HL
__asm
	ld   A,L
	di
	out  (VDPSTATUS),A
	ld   A,H
	and  #0x3F          //bit6 = 0 --> read access
	out  (VDPSTATUS),A 
	ei
	ret
__endasm;
}



/* =============================================================================
SetVDPtoWRITE
Description: 
		Enable VDP to write and indicates the VRAM address where the writing 
		will be performed.
Input:	[unsigned int] VRAM address
Output:	-
Regs:	A             
============================================================================= */
void SetVDPtoWRITE(unsigned int vaddr) __naked
{
vaddr;	//HL
__asm
	ld    A,L             //first 8bits from VRAM ADDR
	di
	out   (VDPSTATUS),A
	ld    A,H             //6 bits from VRAM ADDR 
	and   #0x3F
	or    #0x40           //bit6 = 1 --> write access
	out   (VDPSTATUS),A
	ei
	ret
__endasm;
}





/* #############################################################################
##                                                         SPRITE functions   ##
################################################################################ */


/* =============================================================================
ClearSprites
Description: 
		Initialises the sprite attribute table (OAM). 
		The vertical location of the sprite is set to 209.
Input:	-
Output:	-
============================================================================= */
void ClearSprites(void) __naked
{
__asm
	ld   HL,#BASE8
	call _SetVDPtoWRITE 

	ld   B,#32
	ld   C,#SPRITES_YHIDDEN
TMS_ClearOAMloop$:
	ld   A,C			//( 5ts) 12+14+5= 31ts
	out  (VDPVRAM),A	//(12ts) attr Y (time for write 29 T-states)
	xor  A				//( 8ts)
	nop					//( 5ts)
	nop					//( 5ts)
	out  (VDPVRAM),A	//(12ts) attr X
//	xor  A				//( 8ts) 12+8+5+5 = 30ts - 
	inc  BC				//( 7ts) 12+7+5+5 = 29ts This instruction is added to create a delay up to 29 T-states (need for write to TMS9918A VRAM)
	nop
	nop
	out  (VDPVRAM),A	//attr pattern number
//	xor  A
	dec  BC				//This instruction is added to create a delay up to 29 T-states (need for write to TMS9918A VRAM)
	nop
	nop
	out  (VDPVRAM),A	//attr color
	djnz TMS_ClearOAMloop$   //(14ts or 9ts if B=0)
	
	ret  
__endasm;
} 



/* =============================================================================
SetSpritesSize
Description: 
		Set size type for the sprites.
Input:	[char] size: 0=8x8; 1=16x16
Output:	-
============================================================================= */  
void SetSpritesSize(char size) __naked
{
size;	//A	
__asm

	ld   C,A
	ld   A,(#RG0SAV+1)	//read vdp(1) from mem

	bit  0,C	//IF size = 0 (8x8)?
	jr   Z,TMS_SPRsize8$	

//	set  1,A	//16x16
	or   #0b00000010
	jr   TMS_setREG1$
  
TMS_SPRsize8$:
//	res  1,A	//8x8
	and  #0b11111101
	jr   TMS_setREG1$  
  
__endasm;
} 



/* =============================================================================
SetSpritesZoom
Description: 
		Set zoom type for the sprites.
Input:	[char] or [boolean]/[switcher] zoom: 0/false/OFF = x1; 1/true/ON = x2
Output:	-
============================================================================= */
void SetSpritesZoom(char zoom) __naked
{
zoom;	//A	
__asm
  
	ld   C,A
	ld   A,(#RG0SAV+1)	//read vdp(1) from mem

	bit  0,C	//IF zoom enable?
	jr   Z,TMS_SPRnoZoom$

	or   #0b00000001		//(8c)enable zoom	
	jr   TMS_setREG1$
  
TMS_SPRnoZoom$:
	and   #0b11111110 		//(8c)disable zoom	

TMS_setREG1$:  
	ld   C,#0x01
	jp   writeVDP

__endasm;
} 



/* =============================================================================
PUTSPRITE
Description: 
		Displays a Sprite on the screen.
Input:	[char] sprite plane (0-31) 
		[char] X coordinate 
		[char] Y coordinate
		[char] color (0-15)
		[char] pattern number
Output:	-
============================================================================= */
void PUTSPRITE(char plane, char x, char y, char color, char pattern)
{
plane;		//A
x;			//L
y;			//Stack
color;		//Stack
pattern;	//Stack
__asm
	push IX
	ld   IX,#0
	add  IX,SP
  
	ld   C,L		//x

	call _GetSPRattrVADDR	//Input:A-plane; Output:HL-VRAM addr
//	call _SetVDPtoWRITE		//VDP ready to write to VRAM

	ld   A,4(IX)	//y
	call WriteByteToVRAM

	ld   A,C		//x
	call _FastVPOKE


	ld   E,6(IX)	//Sprite pattern
	call GetSpritePattern	//Input:E; Output:E pattern position according to sprite size
	ld   A,E
	call _FastVPOKE

	ld   A,5(IX)	//color
	call _FastVPOKE

	pop  IX
	
__endasm;
}



/* =============================================================================
GetSPRattrVADDR
Description: 
		Gets the VRAM address of the Sprite attributes of the specified plane
Input:	[char] sprite plane (0-31) 
Output:	[unsigned int] VRAM address
============================================================================= */
unsigned int GetSPRattrVADDR(char plane) __naked
{
plane;		//A
__asm
	SLA  A    //*2
	SLA  A    //*2
	ld   E,A
	ld   D,#0
	ld   HL,#SPR_OAM	//base 8/13/18 sprite attribute table
	add  HL,DE
	ret
	
/* =============================================================================
GetSpritePattern
Description: 
		Returns the pattern value according to the Sprite size 
		(multiplied by 4 when its 16x16).
Input:	[E] sprite pattern 
Output: [E] pattern position
Regs:	A
============================================================================= */
GetSpritePattern::

	ld   A,(#RG0SAV+1)	// read vdp(1) from mem

	bit  1,A			//Sprite size; 1=16x16
	ret  Z				//same value

//if spritesize = 16x16 then E*4
	SLA  E
	SLA  E ;multiplica x4  
	ret	
	
__endasm;
}