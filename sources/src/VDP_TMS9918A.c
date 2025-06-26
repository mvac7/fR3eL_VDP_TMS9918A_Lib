/* ==============================================================================                                                                            
# VDP_TMS9918A MSX Library (fR3eL Project)

- Version: 1.5 (11/12/2023)
- Author: mvac7/303bcn
- Architecture: MSX
- Environment: ROM, MSXBASIC or MSX-DOS
- Format: C object (SDCC .rel)
- Programming language: C and Z80 assembler
- Compiler: SDCC 4.3 or newer 

## Description:                                                              
Opensource library for acces to VDP TMS9918A/28A/29A

Features:
- Not use the BIOS 
- It does not copy the tileset with the MSX system font
  Requires your own tileset or copying the one from the BIOS
- Using the ports 0x98 and 0x99 from MSX computers.
- Uses the memory settings of the display modes set in MSX computers System
- Save VDP registers values in MSX System variables	
 
## History of versions:
- v1.5 (11/12/2023) Update to SDCC (4.1.12) Z80 calling conventions
					Added SetVDPtoREAD and SetVDPtoWRITE functions
					Added FastVPOKE and FastVPEEK functions
					Added initialization of MULTICOLOR mode (in SCREEN function) 
					with sorted map.
					Convert Assembler source code to C
- v1.4 (16/08/2022) Bug#2 (init VRAM addr in V9938) and code optimization 
- v1.3 (23/07/2019) COLOR function improvements
- v1.2 (04/05/2019)
- v1.1 (25/04/2019) 
- v1.0 (14/02/2014)                                                                             
============================================================================== */

#include "../include/msxSystemVariables.h"

#include "../include/VDP_TMS9918A.h"


#define YHIDDEN 0xD1  // position to hide sprites by placing them outside the screen boundaries in TMS9918A modes



/* =============================================================================
SCREEN
Description:
		Initializes the display to one of the four standardized modes on the MSX.
		- T1 and G1 modes are initialized the map (Pattern Name Table) with 
		  value 0. 
		- In Graphic2 and MultiColor modes are initialized in an orderly manner 
		  (as in MSX BASIC) to be able to display an image directly.
		- In graphics modes, the sprite attributes (OAM) are initialized. 
		
Input:	[char] number of screen mode
			0 = TextMode1
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

;  ld   A,4(IX)
	cp   #1
	jr   Z,$screen1
	cp   #2
	jr   Z,$screen2
	cp   #3
	jr   Z,$screen3
  
;screen 0  
	call ClearT1mode

	ld   HL,#mode_TXT1
	;screen 0 > 40 columns mode
	ld   A,#39  ;default value
	ld   (#LINL40),A 

	jr  $setREGs 

$screen1:
	call initG1
	call ClearG1G2
	call _ClearSprites    
	ld   HL,#mode_GFX1
	jr  $setREGs
  
$screen3:
	;call _SortMCmap	;Not in this universe
	call ClearMC
	call _ClearSprites  
	ld   HL,#mode_MC
	jr  $setREGs  

$screen2:
	;call _SortG2map	;Not in this universe
	call ClearG1G2
	call _ClearSprites      
	ld   HL,#mode_GFX2


$setREGs:
	ld   B,#7
	ld   C,#0
loopREGs:
	ld   A,(HL)
	call writeVDP
	inc  HL
	inc  C
	djnz  loopREGs
  
;initialize VRAM access on MSX2 or higher (V9938)
	LD    HL,#MSXID3
	LD    A,(#EXPTBL)            ;EXPTBL=main BIOS-ROM slot address
	CALL  0x000C                 ;RDSLTReads the value of an address in another slot
	EI
	or   A   
	jr   Z,EXIT_SCR
  
;clear upper bits (A14,A15,A16) from VRAM address for only acces to first 16k
	xor  A
	out  (VDPSTATUS),A   ;clear three upper bits for 16bit VRAM ADDR (128K)
	ld   A,#14+128       ;V9938 reg 14 - Control Register
	out  (VDPSTATUS),A
  
EXIT_SCR: 
	pop ix
	ret



/* --------------------------------------------------------------
Writes in the color table, with the last values of the ink and
background indicated with the COLOR function
-------------------------------------------------------------- */
initG1:
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
-------------------------------------------------------------- */
;Reg/Bit  7     6     5     4     3     2     1     0
;0        -     -     -     -     -     -     M3    EXTVID
;1        4/16K	BLK   GINT	M1    M2    -     SIZE  MAG

; M1=1; M2=0; M3=0  
mode_TXT1:
 .db 0B00000000 ;reg0 $00 
 .db 0B11110000 ;reg1 $F0 
 .db 0x00		;reg2 Name Table              (0000h)
 .db 0x00		;reg3 --
 .db 0x01		;reg4 Pattern Table           (0800h)
 .db 0x00		;reg5 --
 .db 0x00		;reg6 --

; M1=0; M2=0; M3=0  
mode_GFX1:
 .db 0B00000000 ;reg0 $00
 .db 0B11100000 ;reg1 Default sprites 8x8 No Zoom
 .db 0x06		;reg2 Name Table             (1800h)
 .db 0x80		;reg3 Color Table            (2000h)
 .db 0x00		;reg4 Pattern Table          (0000h)
 .db 0x36		;reg5 Sprite Attribute Table (1B00h)
 .db 0x07		;reg6 Sprite Pattern Table   (3800h)

; M1=0; M2=0; M3=1  
mode_GFX2:
 .db 0B00000010 ;reg0
 .db 0B11100000 ;reg1 Default sprites 8x8 No Zoom
 .db 0x06  ;reg2 Name Table             (1800h)
 .db 0xFF  ;reg3 Color Table            (2000h)
 .db 0x03  ;reg4 Pattern Table          (0000h)
 .db 0x36  ;reg5 Sprite Attribute Table (1B00h)
 .db 0x07  ;reg6 Sprite Pattern Table   (3800h)

; M1=0; M2=1; M3=0
mode_MC:
 .db 0B00000000	;reg0 $00
 .db 0B11101000	;reg1 $E8 Default sprites 8x8 No Zoom
 .db 0x02		;reg2 Name Table             (0800h)
 .db 0x00		;reg3 Pattern Table          (0000h)
 .db 0x00		;reg4 --
 .db 0x36		;reg5 Sprite Attribute Table (1B00h)
 .db 0x07		;reg6 Sprite Pattern Table   (3800h)  
// --------------------------------------------------------------
__endasm;
} 




/* =============================================================================
SortG2map 
Description: 
		Initializes the pattern name table, with sorted values. 
		Designed to be able to display a G2 (256x192px) image.
Input:	-
Output:	-
============================================================================= */
void SortG2map(void) __naked
{
__asm
	ld   HL,#G2_MAP
	call _SetVDPtoWRITE
	ld	 DE,#0x300
	ld   C,#VDPVRAM
	ld   B,#0
$initG2_loop:
	out  (C),B
	dec  DE
	inc  B
	ld   A,D
	or   E
	jr   NZ,$initG2_loop
	ret	
__endasm;	
}



/* =============================================================================
SortMCmap 
Description: 
		Initializes the pattern name table, with sorted values. 
		Designed to be able to display a MC (64x48 blocks) image.
Input:	-
Output:	-
============================================================================= */
void SortMCmap(void) __naked
{
__asm
	ld   HL,#MC_MAP
	call _SetVDPtoWRITE  ;init VDP to Write to BASE15 add ress
	
	ld   B,#24
	ld   C,#0  ;row
$loopRow:	
	push BC
	ld   A,C
	
	SRL  A 
	SRL  A	;row/4
	SLA  A
	SLA  A
	SLA  A
	SLA  A
	SLA  A	;row*32
	
	ld   B,#32
$loopPrintRow:
	out  (VDPVRAM),A
	inc  A
	djnz $loopPrintRow
	
	pop  BC
	inc  C
	djnz $loopRow
		
	ret
__endasm;	
}


  


/* =============================================================================
CLS 
Description: 
		 Clear Screen
		 Fill the Name Table with the value 0
		 Does not clear the sprite attribute table (OAM)
		 
Input:	-
Output:	-
============================================================================= */
void CLS(void) __naked
{
__asm

	ld   A,(#RG0SAV+1)	;reg1
	bit  4,A			;M1=1 Text-1 (T1)
	jr   NZ,ClearT1mode
	bit  3,A			;M2=1 Multi Colour (MC)
	jr   NZ,ClearMC

;	ld   A,(#RG0SAV)	;reg0  
;	bit  1,A			;M3=1 G2
;	jr   NZ,ClearG2

ClearG1G2:
	xor  A
	ld   DE,#0x300		;32*24
	ld   HL,#G1_MAP
	jp   fillVR
  
ClearMC:
	xor  A
	ld   DE,#0x300		;32*24 (2x2 blocks)
	ld   HL,#MC_MAP		;Name Table
	jp   fillVR
  
ClearT1mode:
	xor  A
	ld   DE,#0x3C0		;40*24
	ld   HL,#T1_MAP
	jp   fillVR
  
__endasm;
} 



/* =============================================================================
COLOR
Description:
		Specifies the ink, foreground and background colors.
		This function has different behaviors depending on the screen mode.
		In T1 (text) mode, the color change is instantaneous except the 
		border color which has no effect.
		In G1, G2 and MC modes, only the border color has an instant effect. 
		Ink and background colors are only used when starting the screen with 
		the SCREEN() function.

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
  ld   (#FORCLR),A		;ink color (foreground) 
  ld   A,L
  ld   (#BAKCLR),A		;background color
  ld   A,4(IX)  
  ld   (#BDRCLR),A		;border color

  ld   A,(#RG0SAV+1)	;reads the last value of register 1 of the VDP in the system variables
  bit  4,A				;M1=1   IF screen0?
  jr   NZ,colorMode0
  
  ld   A,(#BDRCLR)
  ld   B,A  
  ld   A,(#BAKCLR)
  jr   SAVEcolorREG

colorMode0:
  ld   A,(#BAKCLR)
  ld   B,A 
  ld   A,(#FORCLR)

SAVEcolorREG:
  sla  A
  sla  A
  sla  A
  sla  A
  or   B

  
;(info by Portar Doc)
;Register 7: colour register.
;  Bit  Name  Expl.
;  0-3  TC0-3 Background colour in SCREEN 0 (also border colour in SCREEN 1-3)
;  4-7  BD0-3 Foreground colour in SCREEN 0      
  
  ld   C,#0x07	;VDP reg 7
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

  call _SetVDPtoREAD
  
/* =============================================================================
FastVPEEK                                
Description:
		Reads the next video RAM value.
		Requires the VDP to be in read mode, either by previously 
		using VPEEK or SetVDPtoREAD functions.
Input    : --
Output   : A - value
============================================================================= */ 
_FastVPEEK::  
  in   A,(VDPVRAM)

  ret 




/* =============================================================================
WriteByte2VRAM                                
Description:
		Writes a value to the video RAM. 
Input:	HL - VRAM address
		A - value
Output:	-
Regs:	A'
============================================================================= */
WriteByte2VRAM::
	ex   AF,AF
	call _SetVDPtoWRITE
	ex   AF,AF
/* =============================================================================
FastVPOKE                                
Description:
		Writes a value to the next video RAM position. 
		Requires the VDP to be in write mode, either by previously 
		using VPOKE or SetVDPtoWRITE functions.
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
		Fill a large area of the VRAM of the same byte.
Input:	[unsigned int] VRAM address
		[unsigned int] blocklength
		[char] Value to fill.
Output:	- 
============================================================================= */
void FillVRAM(unsigned int vaddr, unsigned int length, char value)
{
vaddr;  //HL
length; //DE
value;  //STack
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
		[unsigned int] blocklength
Output:	- 
============================================================================= */
void CopyToVRAM(unsigned int addr, unsigned int vaddr, unsigned int length)
{
addr;	//HL
vaddr;	//DE
length;	//STack
__asm
  push IX
  ld   IX,#0
  add  IX,SP  

  ex   DE,HL
  
  ld   C,4(IX) ;length
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
		[unsigned int] blocklength
Output:	-
============================================================================= */
void CopyFromVRAM(unsigned int vaddr, unsigned int addr, unsigned int length)
{
vaddr;	//HL
addr;	//DE
length;	//STack
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
		Provides the mirror value of a VDP register stored in system 
		variables.
Input:	[char] VDP register              
Output:	[char] Value
============================================================================= */
char GetVDP(char reg) __naked
{
reg;	//A
__asm 
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
		Writes a value to a VDP register
Input:	[char] VDP register (0-7)                    
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
  
;  jr   writeVDP
;----------------- END  


/* =============================================================================
writeVDP
Description:
		write data in the VDP-register  
Input:	A  - value to write
        C  - number of the register
Output:	-
Registers: IY, DE
============================================================================= */
writeVDP:

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
		Fill a large area of the VRAM of the same byte.
Input:	HL - VRAM address
		DE - Size
		A - value
Output:	-
Regs:	BC
============================================================================= */
fillVR::
  
	ld   C,A

	call _SetVDPtoWRITE  
      
;VFILL_loop:
;	out  (C),B			;(14ts)  14+7+5+5+13=44ts  (time for write 29 T-states)
;	dec  DE				;(7ts)
;	ld   A,D			;(5ts)
;	or   E				;(5ts)
;	jr   nz,VFILL_loop	;(13ts)
;	ret
	
	ld   A,C 
	ld   C,#VDPVRAM
	
    ld   B,E			; Number of loops is in DE
    dec  DE				; Calculate DB value (destroys B, D and E)
    inc  D	
VFILL_loop:
	nop					;( 5ts)
    out  (VDPVRAM),A	;(12ts)   14+5+12 = 31ts
    djnz VFILL_loop		;(14/9ts)
    dec  D
    jr   NZ,VFILL_loop
	ret


/* =============================================================================
LDIR2VRAM
Description:
		Block transfer from memory to VRAM 
Input:	BC - blocklength
		DE - source Memory address
		HL - target VRAM address
Output:	-
Regs:	A
============================================================================= */
LDIR2VRAM::

	call _SetVDPtoWRITE

	ex   DE,HL

	ld   D,B
	ld   E,C
		
	ld   C,#VDPVRAM
/*    
VWRITE_loop:
	outi         //out [c],[HL] + INC HL + dec B

	dec  DE
	ld   A,D
	or   E
	jr   nz,VWRITE_loop    

	ret*/
  
    ld   B,E			// Number of loops is in DE
    dec  DE				// Calculate DB value (destroys B, D and E)
    inc  D	
VWRITE_loop:
	outi				//(18ts) out [c],[HL] + INC HL + dec B
    jp   NZ,VWRITE_loop	//(11ts) 29 T-States (18 + 11)
	
    dec  D
    jr   NZ,VWRITE_loop
	ret
    

        
/* =============================================================================
GetBLOCKfromVRAM
Description: 
		Block transfer from VRAM to memory.  
Input:	BC - blocklength
		HL - source VRAM address                     
		DE - target RAM address
Output:	-
Regs:	A        
============================================================================= */
GetBLOCKfromVRAM::
	call _SetVDPtoREAD

	ex   DE,HL

	ld   D,B
	ld   E,C

	ld   C,#VDPVRAM

    ld   B,E			// Number of loops is in DE
    dec  DE				// Calculate DB value (destroys B, D and E)
    inc  D	
VREAD_loop:
	ini           		//read value from C port, write in [HL] and INC HL
    jp   NZ,VREAD_loop	//(11ts) 29 T-States (18 + 11)
	
    dec  D
    jr   NZ,VREAD_loop
	ret


    /*
VREAD_loop:
	ini           //read value from C port, write in [HL] and INC HL

	dec  DE
	ld   A,D
	or   E
	jr   NZ,VREAD_loop    

	ret*/
__endasm;
}   



/* =============================================================================
SetVDPtoREAD
Description:
		Enable VDP to read (Similar to BIOS SETRD)
Input:	[char] VRAM address
Output:	-
Regs:	A
============================================================================= */
void SetVDPtoREAD(unsigned int vaddr) __naked
{
vaddr;	//HL
__asm
//	push  AF
	ld   A,L
	di
	out  (VDPSTATUS),A
	ld   A,H
	and  #0x3F          ;bit6 = 0 --> read access
	out  (VDPSTATUS),A 
	ei
//	pop   AF
	ret
__endasm;
}



/* =============================================================================
SetVDPtoWRITE
Description: 
		Enable VDP to write (Similar to BIOS SETWRT)
Input:	[char] VRAM address
Output:	-
Regs:	A             
============================================================================= */
void SetVDPtoWRITE(unsigned int vaddr) __naked
{
vaddr;	//HL
__asm
//	push  AF
	ld    A,L             ;first 8bits from VRAM ADDR
	di
	out   (VDPSTATUS),A
	ld    A,H             ;6 bits from VRAM ADDR 
	and   #0x3F
	or    #0x40           ;bit6 = 1 --> write access
	out   (VDPSTATUS),A
	ei
//	pop   AF
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
	ld   C,#YHIDDEN
loop_ClearOAM:
	ld   A,C		   ;(5ts) 12+14+5= 31ts
	out  (VDPVRAM),A   ;(12ts) attr Y (time for write 29 T-states)
	xor  A             ;(8ts)
	nop                ;(5ts)
	nop                ;(5ts)
	out  (VDPVRAM),A   ;(12ts) attr X
	xor  A             ;(8ts) 12+8+5+5 = 30ts - It is not necessary but I add an Xor as a delay to reach 29 T-states (for TMS9918A)
//inc  HL            ;(7ts) 12+7+5+5 = 29ts ???
	nop
	nop
	out  (VDPVRAM),A   ;attr pattern number
	xor  A
	nop
	nop
	out  (VDPVRAM),A   ;attr color
	djnz loop_ClearOAM   ;(14ts or 9ts if B=0)

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
	jr   Z,SPRsize8	

//	set  1,A	//16x16
	or   #0b00000010
	jr   SPRsetREG1
  
SPRsize8:
//	res  1,A	//8x8
	and  #0b11111101
	jr   SPRsetREG1  
  
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
	jr   Z,SPRnozoom

	or   #0b00000001		//(8c)enable zoom	
	jr   SPRsetREG1
  
SPRnozoom:
	and   #0b11111110 		//(8c)disable zoom	

SPRsetREG1:  
	ld   C,#0x01
	jp   writeVDP

__endasm;
} 


/*void SetSpritesZoom(char zoom) __naked
{
zoom;	//A	
__asm
  
	ld   HL,#RG0SAV+1	//read vdp(1) from mem
	ld   B,(HL)

	or   A
	jr   Z,SPRnozoom

	set  0,B	//zoom
	jr   SPRsetREG1
  
SPRnozoom:
	res  0,B	//(10c)no zoom

SPRsetREG1:  
	ld   C,#0x01
	ld   A,B  
	jp   writeVDP

__endasm;
}*/




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
	call WriteByte2VRAM

	ld   A,C		//x
	call _FastVPOKE


	ld   E,6(IX)	//pattern
	call GetSpritePattern	//Input:E -->Sprite pattern; Output:E
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
		Same as MSX BIOS CALATR
Input:	[char] [A] sprite plane (0-31) 
Output:	[unsigned int] [HL] VRAM address
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
Output: [E] new pattern value
============================================================================= */
GetSpritePattern::

	ld   A,(#RG0SAV+1) ; --- read vdp(1) from mem

	bit  1,A        //Sprite size; 1=16x16
	ret  Z			//same value

//if spritesize = 16x16 then E*4
	SLA  E
	SLA  E ;multiplica x4  
	ret	
	
__endasm;
}