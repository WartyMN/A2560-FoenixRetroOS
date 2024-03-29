/*
 * a2560_platform.h
 *
 *  Created on: Feb 19, 2022
 *      Author: micahbly
 */

#ifndef A2560_PLATFORM_H_
#define A2560_PLATFORM_H_


/* about this class
 *
 *
 *
 *** things this class needs to be able to do
 * nothing - this is not a functional class, but a set of useful headers providing structs and defines useful for developing on the A2560 platform
 *
 *** things objects of this class have
 *
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

#include <stdint.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

// 
// // eliminate __huge extension keyword unless we are building with Calypsi for 65816
// #if defined(__VBCC__)
// 	#define __huge  
// #endif

#define ID_CHANNEL_A				0	// for use in text() calls, etc. 
#define ID_CHANNEL_B				1	// for use in text() calls, etc.

// A2560 OTHER
#define EA_MCP						(char*)0x010000	// start of MCP kernel
#define EA_USER						(char*)0x020000	// start of user space. ie, put your program here.

// adapted from vinz67
#define R32(x)						*((volatile unsigned long* const)(x))	// make sure we read a 32 bit long; for VICKY registers, etc.
#define P32(x)						(volatile unsigned long* const)(x)		// make sure we read a 32 bit long; for VICKY registers, etc.
#define R16(x)						*((volatile unsigned short* const)(x))	// make sure we read an 16 bit short; for VICKY registers, etc.
#define P16(x)						(volatile unsigned short* const)(x)		// make sure we read an 16 bit short; for VICKY registers, etc.
#define R8(x)						*((volatile uint8_t* const)(x))			// make sure we read an 8 bit byte; for VICKY registers, etc.
#define P8(x)						(volatile uint8_t* const)(x)			// make sure we read an 8 bit byte; for VICKY registers, etc.

// ** believe to be common to all A2560 platforms... 
#define TEXTA_NUM_COLS_BORDER		4	// need to measure.... 
#define TEXTB_NUM_COLS_BORDER		4	// going on what f68 shows

#define TEXT_COL_COUNT_FOR_PLOTTING_A2560K		100	// regardless of visible cols (between borders), VRAM seems to be fixed at 80 cols across.
#define TEXT_ROW_COUNT_FOR_PLOTTING_A2560K		75	// regardless of visible rows (between borders), VRAM seems to be fixed at 60 rows up/down.
#define TEXT_COL_COUNT_FOR_PLOTTING		TEXT_COL_COUNT_FOR_PLOTTING_A2560K	// regardless of visible cols (between borders), VRAM seems to be fixed at 80 cols across.
#define TEXT_ROW_COUNT_FOR_PLOTTING		TEXT_ROW_COUNT_FOR_PLOTTING_A2560K	// regardless of visible rows (between borders), VRAM seems to be fixed at 60 rows up/down.

#define TEXT_FONT_WIDTH_A2560	8	// for text mode, the width of the fixed-sized font chars
#define TEXT_FONT_HEIGHT_A2560	8	// for text mode, the height of the fixed-sized font chars. I believe this is supposed to be 16, but its 8 in morfe at the moment.

#define VIDEO_MODE_MASK				0xFFFF00FF	//!> the mask for the system control register that holds the video mode bits
#define VIDEO_MODE_BYTE				0x01	//!> the byte offset from system control register that holds the video mode bits
#define VIDEO_MODE_BIT1				0x01	//!> the bits in the 2nd byte of the system control register that define video mode (resolution). if this bit is set, resolution is 800x600 (bit 2 not set) or 640x400 (bit 2 set) or 1024x768 (if bit 11 is set)
// from Stefany on 2022/03/06 discord:
// Channel B bit 8 and 9 are the bit that select the video mode. 640x480, 800x600 and 640x400. No 1024x768 
// Channel A bit 8 and 9 are ignored
// It is bit 11 that gives you the choice between 800x600 or 1024x768.
#define VIDEO_MODE_BIT2				0x02	//!> the bits in the 2nd byte of the system control register that define video mode (resolution). if set on Chan B, you get 640x400. (also requires bit1 to be set?)
#define GAMMA_MODE_DIPSWITCH_BIT	0x20	//!>the bits in the 2nd byte of the system control register reflect dip switch setting for control gamma correction on/off
#define GAMMA_MODE_MASK				0xFF00FFFF	//!> the mask for the system control register that controls gamma (in some way not clear to me)
#define GAMMA_MODE_ONOFF_BITS		0x03	//!>the bits in the 3rd byte of the system control register control gamma correction on/off

	#define BORDER_X_MASK				0xFFFF00FF	//!> the mask for the Border control register (0x0004) long, for the X border
	#define BORDER_Y_MASK				0xFF00FFFF	//!> the mask for the Border control register (0x0004) long, for the Y border
	#define BORDER_CTRL_OFFSET_L		0x01		//!> the (long) offset from the VICKY control register to the border control register		
	#define BORDER_COLOR_OFFSET_L		0x02		//!> the (long) offset from the VICKY control register to the border color register		
	#define BACKGROUND_COLOR_OFFSET_L	0x03		//!> the (long) offset from the VICKY control register to the background color register		
	#define CURSOR_CTRL_OFFSET_L		0x04		//!> the (long) offset from the VICKY control register to the cursor control register		
	#define CURSOR_POS_OFFSET_L			0x04		//!> the (long) offset from the VICKY control register to the cursor position register		
	#define LN_INTERRUPT_01_OFFSET_L	0x05		//!> the (long) offset from the VICKY control register to the line interrupts 0 and 1 registers		
	#define BITMAP_L0_CTRL_L			0x40		//!> the (long) offset from the VICKY control register to the bitmap layer0 control register (foreground layer)		
	#define BITMAP_L0_VRAM_ADDR_OFFSET_L	0x41		//!> the (long) offset from the VICKY control register to the bitmap layer0 VRAM address pointer)		
	#define BITMAP_L1_CTRL_L			0x42		//!> the (long) offset from the VICKY control register to the bitmap layer1 control register (background layer)		
	#define BITMAP_L1_VRAM_ADDR_OFFSET_L	0x43		//!> the (long) offset from the VICKY control register to the bitmap layer1 VRAM address pointer)		
	#define CLUT0_OFFSET_L				0x800		//!> the (long) offset from the VICKY control register to the first CLUT RAM space
	#define CLUT1_OFFSET_L				0x900		//!> the (long) offset from the VICKY control register to the 2nd CLUT RAM space
	#define CLUT2_OFFSET_L				0xA00		//!> the (long) offset from the VICKY control register to the 3rd CLUT RAM space
	#define CLUT3_OFFSET_L				0xB00		//!> the (long) offset from the VICKY control register to the 4th CLUT RAM space
	#define CLUT4_OFFSET_L				0xC00		//!> the (long) offset from the VICKY control register to the 5th CLUT RAM space
	#define CLUT5_OFFSET_L				0xD00		//!> the (long) offset from the VICKY control register to the 6th CLUT RAM space
	#define CLUT6_OFFSET_L				0xE00		//!> the (long) offset from the VICKY control register to the 7th CLUT RAM space
	#define CLUT7_OFFSET_L				0xF00		//!> the (long) offset from the VICKY control register to the 8th CLUT RAM space


	// ** C256 (VICKY II)
	
	// byte swapper: 68000 is big endian; 65816 is little endian
	#define BSWAP(word)	( (((word) >> 8) & 0x00FF) | (((word) << 8) & 0xFF00) )
	
	#define VICKY_C256					0x00af0000		// Vicky II offset/first register
	#define VICKY_II_MASTER_CTRL_REG_L	0x00af0000		// Vicky II Master Control Register - low - graphic mode/text mode/etc.
	#define VICKY_II_MASTER_CTRL_REG_H	0x00af0001		// Vicky II Master Control Register - high - screen res, etc.
	#define VICKY_II_GAMMA_CTRL_REG		0x00af0002		// Vicky II Gamma Control Register
	#define VICKY_II_BORDER_CTRL_REG	0x00af0004		// Vicky II Border Control Register
	#define VICKY_II_BORDER_COLOR_B		0x00af0005		// Vicky II Border Color Blue
	#define VICKY_II_BORDER_COLOR_G		0x00af0006		// Vicky II Border Color Green
	#define VICKY_II_BORDER_COLOR_R		0x00af0007		// Vicky II Border Color Red
	#define VICKY_II_BORDER_X_SIZE		0x00af0008		// Vicky II Border X size in pixels
	#define VICKY_II_BORDER_Y_SIZE		0x00af0009		// Vicky II Border Y size in pixels
	
	#define VICKY_II_BACKGROUND_COLOR_B		0x00af000D		// Vicky II background color Blue
	#define VICKY_II_BACKGROUND_COLOR_G		0x00af000E		// Vicky II background color Green
	#define VICKY_II_BACKGROUND_COLOR_R		0x00af000F		// Vicky II background color Red

	#define BITMAP_L0_CTRL				0x00af0100		//!> bitmap layer0 control register (background layer)		
	#define BITMAP_L0_VRAM_ADDR_L		0x00af0101		//!> bitmap layer0 VRAM address pointer)		
	#define BITMAP_L0_VRAM_ADDR_M		0x00af0102		//!> bitmap layer0 VRAM address pointer)		
	#define BITMAP_L0_VRAM_ADDR_H		0x00af0103		//!> bitmap layer0 VRAM address pointer)		
	#define BITMAP_L1_CTRL				0x00af0108		//!> bitmap layer1 control register (foreground layer)		
	#define BITMAP_L1_VRAM_ADDR_L		0x00af0109		//!> bitmap layer1 VRAM address pointer)		
	#define BITMAP_L1_VRAM_ADDR_M		0x00af010A		//!> bitmap layer1 VRAM address pointer)		
	#define BITMAP_L1_VRAM_ADDR_H		0x00af010B		//!> bitmap layer1 VRAM address pointer)		

	#define TEXT_RAM_C256			(char*)0xafa000			// CS_TEXT_MEM_PTR	Text Memory Block
	#define TEXT_ATTR_C256			(char*)0xafc000			// CS_COLOR_MEM_PTR	Color Text Memory Block
	#define TEXT_FORE_LUT_C256		(char*)0xaf1f40			// FG_CHAR_LUT_PTR	Text Foreground Look-Up Table
	#define TEXT_BACK_LUT_C256		(char*)0xaf1f80			// BG_CHAR_LUT_PTR	Text Background Look-Up Table
	#define FONT_MEMORY_BANK_C256	(char*)0xaf8000			// FONT_MEMORY_BANK0	FONT Character Graphic Mem
	#define VICKY_II_CLUT0				0xaf2000				// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT1				VICKY_II_CLUT0 + 0x400	// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT2				VICKY_II_CLUT1 + 0x400	// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT3				VICKY_II_CLUT2 + 0x400	// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT4				VICKY_II_CLUT3 + 0x400	// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT5				VICKY_II_CLUT4 + 0x400	// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT6				VICKY_II_CLUT5 + 0x400	// each addition LUT is 400 offset from here
	#define VICKY_II_CLUT7				VICKY_II_CLUT6 + 0x400	// each addition LUT is 400 offset from here


	#define GABE_SYS_STAT				0x00AFE887	//!> The gabe register holding the machine ID. Machine ID is stored in 4 least significant bits
	
	#define VIDEO_MODE_REG_OFFSET_B		0x01		//!> the (byte) offset from the VICKY control register to the Video mode reg (MASTER_CTRL_REG_H)
	#define GAMMA_CTRL_REG_OFFSET_B		0x02		//!> the (byte) offset from the VICKY control register to the Gamma control register
	#define VICKY_RESERVED_OFFSET_B		0x03		//!> the (byte) offset from the VICKY control register to the VKY_RESERVED_O1 byte
	#define BORDER_CTRL_OFFSET_B		0x04		//!> the (byte) offset from the VICKY control register to the border control register
	#define BORDER_COLOR_OFFSET_B_B		0x05		//!> the (byte) offset from the VICKY control register to the border color register	for Blue
	#define BORDER_COLOR_OFFSET_G_B		0x06		//!> the (byte) offset from the VICKY control register to the border color register	for Green
	#define BORDER_COLOR_OFFSET_R_B		0x07		//!> the (byte) offset from the VICKY control register to the border color register	for Red
	#define BORDER_X_SIZE_B				0x08		//!> the (byte) offset from the VICKY control register to the border X-size
	#define BORDER_Y_SIZE_B				0x09		//!> the (byte) offset from the VICKY control register to the border Y-size
	#define BACKGROUND_COLOR_OFFSET_B_B	0x0D		//!> the (byte) offset from the VICKY control register to the background color register	for Blue
	#define BACKGROUND_COLOR_OFFSET_G_B	0x0E		//!> the (byte) offset from the VICKY control register to the background color register	for Green
	#define BACKGROUND_COLOR_OFFSET_R_B	0x0F		//!> the (byte) offset from the VICKY control register to the background color register	for Red
	#define CURSOR_CTRL_OFFSET_B		0x10		//!> the (byte) offset from the VICKY control register to the cursor control register		
	#define START_ADDR_OFFSET_B			0x11		//!> the (byte) offset from the VICKY control register to change the starting address of the text mode buffer (in x)
	#define CURSOR_CHAR_OFFSET_B		0x12		//!> the (byte) offset from the VICKY control register to the Char used as the Cursor	
	#define CURSOR_COLOR_OFFSET_B		0x13		//!> the (byte) offset from the VICKY control register to the color used for the Cursor	
	#define LN_INTERRUPT_01_OFFSET_B	0x1B		//!> the (byte) offset from the VICKY control register to the line interrupts 0 and 1 registers		
	#define BITMAP_L0_CTRL_B			0x100		//!> the (byte) offset from the VICKY control register to the bitmap layer0 control register (foreground layer)		
	#define BITMAP_L0_VRAM_ADDR_L_B		0x101		//!> the (byte) offset from the VICKY control register to the bitmap layer0 VRAM address pointer)		
	#define BITMAP_L0_VRAM_ADDR_M_B		0x102		//!> the (byte) offset from the VICKY control register to the bitmap layer0 VRAM address pointer)		
	#define BITMAP_L0_VRAM_ADDR_H_B		0x103		//!> the (byte) offset from the VICKY control register to the bitmap layer0 VRAM address pointer)		
	#define BITMAP_L1_CTRL_B			0x108		//!> the (byte) offset from the VICKY control register to the bitmap layer1 control register (foreground layer)		
	#define BITMAP_L1_VRAM_ADDR_L_B		0x109		//!> the (byte) offset from the VICKY control register to the bitmap layer1 VRAM address pointer)		
	#define BITMAP_L1_VRAM_ADDR_M_B		0x10A		//!> the (byte) offset from the VICKY control register to the bitmap layer1 VRAM address pointer)		
	#define BITMAP_L1_VRAM_ADDR_H_B		0x10B		//!> the (byte) offset from the VICKY control register to the bitmap layer1 VRAM address pointer)		
	#define CLUT0_OFFSET_B				0x2000		//!> the (byte) offset from the VICKY control register to the first CLUT RAM space
	#define CLUT1_OFFSET_B				0x2400		//!> the (byte) offset from the VICKY control register to the 2nd CLUT RAM space
	#define CLUT2_OFFSET_B				0x2800		//!> the (byte) offset from the VICKY control register to the 3rd CLUT RAM space
	#define CLUT3_OFFSET_B				0x2C00		//!> the (byte) offset from the VICKY control register to the 4th CLUT RAM space
	#define CLUT4_OFFSET_B				0x3000		//!> the (byte) offset from the VICKY control register to the 5th CLUT RAM space
	#define CLUT5_OFFSET_B				0x3400		//!> the (byte) offset from the VICKY control register to the 6th CLUT RAM space
	#define CLUT6_OFFSET_B				0x3800		//!> the (byte) offset from the VICKY control register to the 7th CLUT RAM space
	#define CLUT7_OFFSET_B				0x3C00		//!> the (byte) offset from the VICKY control register to the 8th CLUT RAM space
	#define FONT_MEMORY_BANK0_OFFSET_B	0x8000		//!> the (byte) offset from the VICKY control register to the font memory for bank0

#define GRAPHICS_MODE_MASK		0xFFFFFF00	//!> the mask for the system control register that holds the graphics/bitmap/text/sprite mode bits
#define GRAPHICS_MODE_TEXT		0x01	// 0b00000001	Enable the Text Mode
#define GRAPHICS_MODE_TEXT_OVER	0x02	// 0b00000010	Enable the Overlay of the text mode on top of Graphic Mode (the Background Color is ignored)
#define GRAPHICS_MODE_GRAPHICS	0x04	// 0b00000100	Enable the Graphic Mode
#define GRAPHICS_MODE_EN_BITMAP	0x08	// 0b00001000	Enable the Bitmap Module In Vicky
#define GRAPHICS_MODE_EN_TILE	0x10	// 0b00010000	Enable the Tile Module in Vicky
#define GRAPHICS_MODE_EN_SPRITE	0x20	// 0b00100000	Enable the Sprite Module in Vicky
#define GRAPHICS_MODE_EN_GAMMA	0x40	// 0b01000000	Enable the GAMMA correction - The Analog and DVI have different color values, the GAMMA is great to correct the difference
#define GRAPHICS_MODE_DIS_VIDEO	0x80	// 0b10000000	This will disable the Scanning of the Video information in the 4Meg of VideoRAM hence giving 100% bandwidth to the CPU


// VICKY RESOLUTION FLAGS Per A2560K_UM_Rev0.0.1.pdf and A2560U_UM_Rev0.0.2.pdf
// VICKY II / VICKY III Chan B
// 640x480  @ 60FPS > 0 0
// 800x600  @ 60FPS > 0 1
// reserved         > 1 0
// 640x400  @ 70FPS > 1 1

// VICKY III Chan A
// 800x600  @ 60FPS > 0 0
// 1024x768 @ 60FPS > 0 1
// reserved         > 1 0
// reserved         > 1 1

#define VICKY_II_RES_640X480_FLAGS		0x00	// 0b00000000
#define VICKY_II_RES_800X600_FLAGS		0x01	// 0b00000001
#define VICKY_II_PIX_DOUBLER_FLAGS		0x02	// 0b00000001
#define VICKY_II_RES_640X400_FLAGS		0x03	// 0b00000011

#define VICKY_IIIB_RES_640X480_FLAGS	0x00	// 0b00000000
#define VICKY_IIIB_RES_800X600_FLAGS	0x01	// 0b00000001
#define VICKY_IIIB_RES_640X400_FLAGS	0x03	// 0b00000011

#define VICKY_IIIA_RES_800X600_FLAGS	0x00	// 0b00000000
#define VICKY_IIIA_RES_1024X768_FLAGS	0x08	// 0b00000100


// ** A2560K and A2560X
#define VICKY_A2560K_A				0xfec40000				// vicky III channel A control register
#define VICKYA_CURSOR_CTRL_A2560K	VICKY_A2560K_A + 0x10	// vicky III channel A cursor control register
#define VICKYA_CURSOR_POS_A2560K	VICKY_A2560K_A + 0x14	// vicky III channel A cursor position register (x pos is lower word, y pos is upper word)
#define VICKY_A2560K_B				0xfec80000				// vicky III channel B control register
#define VICKYB_BORDER_CTRL_A2560K	VICKY_A2560K_B + 0x04	// vicky III channel B border control register
#define VICKYB_BORDER_COLOR_A2560K	0xfec80008				// vicky III channel B border color register
#define VICKYB_BACK_COLOR_A2560K	0xfec8000C				// vicky III channel B background color register
#define VICKYB_CURSOR_CTRL_A2560K	VICKY_A2560K_B + 0x10	// vicky III channel B cursor control register
#define VICKYB_CURSOR_POS_A2560K	VICKY_A2560K_B + 0x14	// vicky III channel B cursor position register
#define VICKYB_BITMAP_L0_CTRL		0xfec80100				// vicky III channel B bitmap layer 0 control register (1=enable, +2=LUT0, +4=LUT1, +8=LUT2
#define VICKYB_MOUSE_GRAPHIC_A2560K	0xfec80400				// vicky III channel B mouse pointer graphic stored here (16x16)
#define VICKYB_MOUSE_CTRL_A2560K	0xfec80c00				// vicky III channel B mouse pointer control register. set to 1 to enable mouse. +2 to do whatever "pointer choice" does.
#define VICKYB_MOUSE_PTR_POS_A2560K	0xfec80c04				// vicky III channel B mouse pointer position (Y pos in upper 16 bits, x in lower)
#define TEXTA_RAM_A2560K			(char*)0xfec60000		// channel A text
#define TEXTA_ATTR_A2560K			(char*)0xfec68000		// channel A attr
#define TEXTA_FORE_LUT_A2560K		(char*)0xfec6c400		// FG_CHAR_LUT_PTR	Text Foreground Look-Up Table
#define TEXTA_BACK_LUT_A2560K		(char*)0xfec6c440		// BG_CHAR_LUT_PTR	Text Background Look-Up Table
#define TEXTB_RAM_A2560K			(char*)0xfeca0000		// channel B text
#define TEXTB_ATTR_A2560K			(char*)0xfeca8000		// channel B attr
#define TEXTB_FORE_LUT_A2560K		(char*)0xfecac400		// FG_CHAR_LUT_PTR	Text Foreground Look-Up Table
#define TEXTB_BACK_LUT_A2560K		(char*)0xfecac440		// BG_CHAR_LUT_PTR	Text Background Look-Up Table
#define FONT_MEMORY_BANKA_A2560K	(char*)0xfec48000		// chan A
#define FONT_MEMORY_BANKB_A2560K	(char*)0xfec88000		// chan B
#define VICKY_IIIB_CLUT0			0xfec82000				// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT1			VICKY_IIIB_CLUT0 + 0x400	// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT2			VICKY_IIIB_CLUT1 + 0x400	// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT3			VICKY_IIIB_CLUT2 + 0x400	// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT4			VICKY_IIIB_CLUT3 + 0x400	// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT5			VICKY_IIIB_CLUT4 + 0x400	// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT6			VICKY_IIIB_CLUT5 + 0x400	// each addition LUT is 400 offset from here
#define VICKY_IIIB_CLUT7			VICKY_IIIB_CLUT6 + 0x400	// each addition LUT is 400 offset from here

#define default_start_a2560k_vram	0x00011000	// offset against vicky I think though. add to VICKY_A2560K_B? based on doing peek32 in f68. 
#define BITMAP_CTRL_REG_A2560_0		0xfec80100	//! Bitmap Layer0 Control Register (Foreground Layer)
#define BITMAP_VRAM_ADDR_A2560_0	0xfec80104	//! Bitmap Layer0 VRAM Address Pointer. Offset within the VRAM memory from VICKY’s perspective. VRAM Address begins @ $00:0000 and ends @ $1FFFFF
#define BITMAP_CTRL_REG_A2560_1		0xfec80108	//! Bitmap Layer1 Control Register (Background Layer)
#define BITMAP_VRAM_ADDR_A2560_1	0xfec8010C	//! Bitmap Layer0 VRAM Address Pointer. Offset within the VRAM memory from VICKY’s perspective. VRAM Address begins @ $00:0000 and ends @ $1FFFFF


// ** A2560U and A2560U+
#define VICKY_A2560U				0xb40000				// Vicky II offset/first register
#define VICKY_CURSOR_CTRL_A2560U	VICKY_A2560U + 0x10		// vicky II channel A cursor control register
#define VICKY_CURSOR_POS_A2560U		VICKY_A2560U + 0x14		// vicky II channel A cursor position register (x pos is lower word, y pos is upper word)
#define TEXT_RAM_A2560U				(char*)0xb60000			// text (A2560U only has one video channel)
#define TEXT_ATTR_A2560U			(char*)0xb68000			// attr (A2560U only has one video channel)
#define TEXT_FORE_LUT_A2560U		(char*)0xb6c400			// FG_CHAR_LUT_PTR	Text Foreground Look-Up Table
#define TEXT_BACK_LUT_A2560U		(char*)0xb6c440			// BG_CHAR_LUT_PTR	Text Background Look-Up Table
#define FONT_MEMORY_BANK_A2560U		(char*)0xb48000			// only 1 channel


// ** VRAM locations per machine

// NOTE: VRAM is currently only available in VRAM Buffer A on A2560K, Buffer B is not implemented (2022/03/21)

#if defined _C256_FMX_
	#define VRAM_START				0x00B00000
	#define VRAM_LEN				0x00200000
#elif defined _A2560U_
	#define VRAM_START				0x00C00000
	#define VRAM_LEN				0x00200000
#elif defined _A2560K_
	#define VRAM_START				0x00800000
	#define VRAM_LEN				0x00400000
#endif

#define VRAM_OFFSET_TO_NEXT_SCREEN	0x75300		// 800x600 - 480,000 -- number of bytes needed to cover maximum screen resolution for one bitmap layer


// subtract 0xfe000000 from the UM map for Vicky (to get the old/morfe addresses)
// size of some areas changed too:
//   channel-A text went 0xc6:0000 -> 0xFEC6:0000, but channel-A color went 0xc6:8000 -> 0xFEC6:4000
//   channel-B text went 0xca:0000-> 0xFECA:0000, channel-B color went 0xca:8000 -> 0xFECA:4000
// btw, one thing to keep in mind is device-mem access granularity -- while in morfe you can do 8-32bit accesses, on the actual hw you will need to adhere to the area access granularity
//  see user manual, the "SIZE" columns

// c256foenix on 2/27:
// I will have to look into that. Suffice to say that Channel A, has 2 Video Modes, 800x600 and 1024x768 with no doubling. Channel B has 6 modes, 640x480@60, 800x600@60 and 640x400@70 and with the equivalent Pixel Doubling, 320x240, 400x300 and 320x200. 
// Now, in the K, for Channel A, to not be confusing (although I might have created what I was trying to avoid) and to not have competing Regiters bit with different function (from Channel B and A), I moved the Resolution selection bit to bit# (Something I have to check), but it is farther down the Control Register.
// the Video mode bit for Channel A (that I call Auxiliary) would be bit 11 of the Control Register
// assign Mstr_Ctrl_Video_Mode_PLL_Aux_o = VICKY_MASTER_REG1_RESYNC_AUX[2][11];
// So, that is the bit that selects either 800x600 or 1024x768 and bit[9:8] are ignored.

//    so: 1024x768 = 128x96
//    800x600 = 100x75
//    640x480 = 80x60
//    640x400 = 80x50
//    400x300 = 50x37.5
//    320x240 = 40x30
//    320x200 = 40x25
//    (these are all maximums, as borders can be configured, which reduces the number of usable rows/cols.)
//
// c256 foenix on 2/27:
// To answer, in the traditional Text Mode (Channel B). When you are double pixel mode, everything is reajusted automatically. The Channel A doesn't have a text doubling mode (anymore). And the text matrix and FONT dimension are all manual (needs to be programmed). This is to allow the usage of different sizes of FONT.

// fonts
// gadget:
// If it's the same as on the C256 line, each character consists of 8 bytes.  Upper left hand corner is the high-bit of byte zero, upper right is the low bit of byte zero, lower left is the high bit of byte 7, lower right is the low bit of byte 7.  The bytes are placed in memory from character zero to character 255, 0..7, 0..7, 0..7, etc.

// pjw:
// Yeah, C256 and A2560U fonts are the same layout. The A2560K is a little different. The mostly-text-mode screen supports an 8x16 font layout, where the structure is essentially the same, but each character is 16 bytes rather than 8.

// beethead:
// The 2nd font was removed when the U line came in since it was not being used.  Atleast on the C256's.

// PJW — 2022/03/06 at 9:10 AM
// The A2560K has two DVI ports: one is channel A (which is a text only channel), and channel B is a text and graphics channel equivalent to the main screen on the A2560U. The boot up image on the K shows up on channel B. Currently, the MCP uses channel A as the main interaction channel, but I think I'm going to change that soon, since some people may have just the one screen. The "CTX Switch" key on the A2560K was originally intended as a way to switch which screen you were using for text input, and I may finally implement that.

#define SYS_TICKS_PER_SEC		60	// per syscalls.h in MCP, "a jiffie is 1/60 of a second."

// machine model numbers - for decoding s_sys_info.model
#define MACHINE_C256_FMX		0	///< for s_sys_info.model
#define MACHINE_C256_U			1	///< for s_sys_info.model
#define MACHINE_C256_GENX		4	///< for s_sys_info.model
#define MACHINE_C256_UPLUS		5	///< for s_sys_info.model
#define MACHINE_A2560U_PLUS		6	///< for s_sys_info.model
#define MACHINE_A2560X			7	///< for s_sys_info.model
#define MACHINE_A2560U			9	///< for s_sys_info.model
#define MACHINE_A2560K			13	///< for s_sys_info.model



// amiga rawkey codes
#define KEYCODE_CURSOR_LEFT		79
#define KEYCODE_CURSOR_RIGHT	78
#define KEYCODE_CURSOR_UP		76
#define KEYCODE_CURSOR_DOWN		77

#define KEYCODE_RETURN			0x44					// rawkey code
#define KEYCODE_RETURN_UP		KEY_RAW_RETURN & 0x80
#define KEY_RETURN			13	// vanilla key
#define KEY_ESC				27	// vanilla key

#define CHAR_UMLAUT			0xA8	// ¨ char -- will use in place of ellipsis when truncating strings
#define CHAR_ELLIPSIS		CHAR_UMLAUT

#define KEY_BUFFER_SIZE		16	// unlikely anyone would ever want to type 15 chars to select a file

#define ALERT_MAX_MESSAGE_LEN		256	// 255 chars + terminator. seems long, but with formatting chars, not crazy.

#define ALERT_DIALOG_SHOW_AS_ERROR	true	// parameter for General_ShowAlert()
#define ALERT_DIALOG_SHOW_AS_INFO	false	// parameter for General_ShowAlert()

#define ALERT_DIALOG_INCLUDE_CANCEL	true	// parameter for General_ShowAlert()
#define ALERT_DIALOG_NO_CANCEL_BTN	false	// parameter for General_ShowAlert()

#define ALERT_DIALOG_1ST_BUTTON	0	// return value for General_ShowAlert()
#define ALERT_DIALOG_2ND_BUTTON	1	// return value for General_ShowAlert()
#define ALERT_DIALOG_3RD_BUTTON	2	// return value for General_ShowAlert()
// NOTE: can't define "OK" as 1 or 0, because it depends on passing ok, then cancel (eg). Buttons appear to be numbered from RIGHT to LEFT!


// System Default Colors - correspond to the CLUT loaded by sys lib

#define SYS_COLOR_WHITE		244						// = 0xF4
#define SYS_COLOR_GRAY1		SYS_COLOR_WHITE + 1
#define SYS_COLOR_GRAY2		SYS_COLOR_WHITE + 2
#define SYS_COLOR_GRAY3		SYS_COLOR_WHITE + 3
#define SYS_COLOR_GRAY4		SYS_COLOR_WHITE + 4
#define SYS_COLOR_GRAY5		SYS_COLOR_WHITE + 5
#define SYS_COLOR_GRAY6		SYS_COLOR_WHITE + 6
#define SYS_COLOR_GRAY7		SYS_COLOR_WHITE + 7
#define SYS_COLOR_GRAY8		SYS_COLOR_WHITE + 8
#define SYS_COLOR_GRAY9		SYS_COLOR_WHITE + 9
#define SYS_COLOR_GRAY10	SYS_COLOR_WHITE + 10
#define SYS_COLOR_BLACK		255

#define SYS_COLOR_RED1		35 // = 0x23
#define SYS_COLOR_RED2		SYS_COLOR_RED1 + 36*5+1	// 3rd lightest red
#define SYS_COLOR_RED3		SYS_COLOR_RED2 + 3

#define SYS_COLOR_GREEN1	23*8+1
#define SYS_COLOR_GREEN2	SYS_COLOR_GREEN1 + 2
#define SYS_COLOR_GREEN3	SYS_COLOR_GREEN2 + 2

#define SYS_COLOR_BLUE1		26*8+3	// = 211 = 0xd3
#define SYS_COLOR_BLUE2		SYS_COLOR_BLUE1 + 2
#define SYS_COLOR_BLUE3		SYS_COLOR_BLUE2 + 2

#define SYS_COLOR_PURPLEBLUE		15*8+7	// purplish blue = 127 = 0x7F
#define SYS_COLOR_PURPLEBLUEINACT	SYS_COLOR_GRAY3	// light gray inactive accent color for purplish blue = 85 = 0x55
#define SYS_COLOR_PURPLEBLUEHL		6*7+1	// light active accent color for purplish blue = 37 = 0x2B
#define SYS_COLOR_TETRA_1			14 * 4 + 0 // wine color (CC6699) that is tetradic for SYS_COLOR_PURPLEBLUE = 56 = 0x38
#define SYS_COLOR_TETRA_2			11 * 4 + 1 // yellowish (CCCC66) color that is tetradic for SYS_COLOR_PURPLEBLUE = 44 = 0x2C
#define SYS_COLOR_TETRA_3			29 * 4 + 0 // teal/greenish color (66CC99) that is tetradic for SYS_COLOR_PURPLEBLUE = 116 = 0x74

#define SYS_DEF_COLOR_WINFRAME			SYS_COLOR_BLACK
#define SYS_DEF_COLOR_WINTITLE_BACK		SYS_COLOR_PURPLEBLUE
#define SYS_DEF_COLOR_WINTITLE_TEXT		SYS_COLOR_WHITE
#define SYS_DEF_COLOR_ICONBAR_BACK		SYS_COLOR_PURPLEBLUE
#define SYS_DEF_COLOR_CONTENT_BACK		SYS_COLOR_WHITE
#define SYS_DEF_COLOR_BUTTON_BACK		SYS_COLOR_GRAY7
#define SYS_DEF_COLOR_BUTTON_PUSH		SYS_COLOR_PURPLEBLUE
#define SYS_DEF_COLOR_BUTTON_TEXT		SYS_COLOR_WHITE
#define SYS_DEF_COLOR_BUTTON_TEXT_DIS	SYS_COLOR_GRAY3
#define SYS_DEF_COLOR_DESKTOP			SYS_COLOR_GRAY4



typedef uint8_t	ColorIdx;

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum
{
	back_layer = 0,
	fore_layer = 1,
} bitmap_layer;

// device-independent resolution flags
typedef enum
{
	RES_640X400 = 0,
	RES_640X480,
	RES_800X600,
	RES_1024X768,
} screen_resolution;

/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

// forward declarations
typedef struct Font Font;						// defined in font.h
typedef struct Window Window;					// defined in window.h
typedef struct ClipRect ClipRect;				// defined in window.h
typedef struct NewWinTemplate NewWinTemplate;	// defined in window.h
typedef struct Theme Theme;						// defined in theme.h
typedef struct ControlBackdrop ControlBackdrop;	// defined in theme.h
typedef struct Control Control;					// defined in control.h
typedef struct ControlTemplate ControlTemplate;	// defined in control.h
typedef struct System System;					// defined in lib_sys.h
typedef struct Bitmap Bitmap;					// defined in bitmap.h
typedef struct List List;						// defined in list.h
typedef struct EventRecord EventRecord;			// defined in event.h
typedef struct EventManager EventManager;		// defined in event.h
typedef struct MouseTracker MouseTracker;		// defined in mouse.h
typedef struct MenuItem MenuItem;				// defined in menu.h
typedef struct MenuGroup MenuGroup;				// defined in menu.h
typedef struct Menu Menu;						// defined in menu.h

//typedef enum event_modifiers event_modifiers;	// defined in event.h


typedef struct Coordinate
{
    int16_t x;
    int16_t y;
} Coordinate;

typedef struct Rectangle
{
	int16_t		MinX, MinY;
	int16_t		MaxX, MaxY;
} Rectangle;

typedef struct Screen
{
	int16_t			id_;				// 0 for channel A, 1 for channel B. not all foenix's have 2 channels.
	volatile unsigned long*	vicky_;		// VICKY primary register RAM loc. See VICKY_A2560K_A, VICKY_A2560K_B, VICKY_A2560U, etc.
	Rectangle		rect_;				// the x1, y1, > x2, y2 coordinates of the screen, taking into account any borders. 
	int16_t			width_;				// for the current resolution, the max horizontal pixel count 
	int16_t			height_;			// for the current resolution, the max vertical pixel count 
	int16_t			text_cols_vis_;		// accounting for borders, the number of visible columns on screen
	int16_t			text_rows_vis_;		// accounting for borders, the number of visible rows on screen
	int16_t			text_mem_cols_;		// for the current resolution, the total number of columns per row in VRAM. Use for plotting x,y 
	int16_t			text_mem_rows_;		// for the current resolution, the total number of rows per row in VRAM. Use for plotting x,y 
	char*			text_ram_;
	char*			text_attr_ram_;
	char*			text_font_ram_;		// 2K of memory holding font definitions.
	char*			text_color_fore_ram_;	// 64b of memory holding foreground color LUTs for text mode, in BGRA order
	char*			text_color_back_ram_;	// 64b of memory holding background color LUTs for text mode, in BGRA order
	int16_t			text_font_height_;	// in text mode, the height in pixels for the fixed width font. Should be either 8 or 16, depending on which Foenix. used for calculating text fit.
	int16_t			text_font_width_;	// in text mode, the width in pixels for the fixed width font. Unlikely to be other than '8' with Foenix machines. used for calculating text fit.
	char			text_temp_buffer_1_[TEXT_COL_COUNT_FOR_PLOTTING_A2560K * TEXT_ROW_COUNT_FOR_PLOTTING_A2560K + 1];	// todo: replace with pointer, and allocate space on resolution switch. general use temp buffer - do NOT use for real storage - any utility function clobber it
	char			text_temp_buffer_2_[TEXT_COL_COUNT_FOR_PLOTTING_A2560K * TEXT_ROW_COUNT_FOR_PLOTTING_A2560K + 1];	// todo: replace with pointer, and allocate space on resolution switch. general use temp buffer - do NOT use for real storage - any utility function clobber it
	Bitmap*			bitmap_[2];			//! The foreground (layer0=0) and background (layer1=1) bitmaps associated with this screen, if any. (Text only screens do not have bitmaps available)
} Screen;



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/





#endif /* A2560_PLATFORM_H_ */
