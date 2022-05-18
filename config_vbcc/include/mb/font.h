//! @file font.h

/*
 * font.h
 *
*  Created on: Mar 15, 2022
 *      Author: micahbly
 */

#ifndef LIB_FONT_H_
#define LIB_FONT_H_


/* about this class: Font
 *
 * This provides functionality for loading and drawing bitmapped proportional and fixed-width fonts
 *
 *** things this library needs to be able to do
 * 
 *
 * STRETCH GOALS
 * 
 *
 * SUPER STRETCH GOALS
 * 
 * 
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes


// C includes
#include <stdbool.h>


// A2560 includes
#include <mcp/syscalls.h>
#include "a2560_platform.h"
#include "general.h"
#include "text.h"
#include "bitmap.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define FONT_RECORD_SIZE		26	//!< size, in bytes, of the font record (minus tables) as stored in Mac FONT resources

#define FONT_CHAR_FOENIX_OUTLINE	0x11	//!< the outlined Foenix character
#define FONT_CHAR_CHECKMARK			0x12	//!< the checkmark character
#define FONT_CHAR_DIAMOND			0x13	//!< the diamond character
#define FONT_CHAR_FOENIX_REG		0x14	//!< the non-outlined Foenix character
#define FONT_CHAR_MENU_RIGHT		0x15	//!< the '>' style character for use in showing submenus
#define FONT_CHAR_MENU_RIGHT_WIDTH	7		//!< the width, in pixels, of the standard '>' for menu sub-menus. Every font is to use the same width, regardless of style or size.


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

//! This Font object is essentially the Mac "fontRecord" struct, with added pointers for the data tables. 
//! It is designed to allow a Mac 'FONT' resource to be loaded into memory to populate this struct. 
struct Font {
	int16_t				fontType;		//!< Only bit we care about is the first. See https://developer.apple.com/library/archive/documentation/mac/Text/Text-251.html#MARKER-9-442
	int16_t				firstChar;		//!< ASCII code of first character
	int16_t				lastChar;		//!< ASCII code of last character
	int16_t				widMax;			//!< maximum character width -- Could be used if font is fixed-width
	int16_t				kernMax;		//!< negative of maximum character kern
	int16_t				nDescent;		//!< negative of descent
	int16_t				fRectWidth;		//!< width of font rectangle
	int16_t				fRectHeight;	//!< height of font rectangle
	uint16_t			owTLoc;			//!< offset to offset/width table
	int16_t				ascent;			//!< ascent
	int16_t				descent;		//!< descent
	int16_t				leading;		//!< leading
	int16_t				rowWords;		//!< row width of bit image / 2
	uint16_t*			image_table_;	//!< The font image data
	uint16_t*			loc_table_;		//!< The location table
	uint16_t*			width_table_;	//!< Table containing h offset and widths for each glyph
	uint16_t*			height_table_;	//!< Table containing starting v offset and active v pixel count for each glyph
};






/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Create a Font object, and populate it from the passed buffer.
//! NOTE: it is not possible to create a Font object without having a valid 'FONT' data chunk already in memory.
//! NOTE: this allocates new memory for the font, and copies the font data to it from the passed buffer. It is not dependent on the data in the buffer after returning.
//! @param	the_data: Must contain a valid Mac 'FONT' resource data hunk. 
Font* Font_New(unsigned char* the_data);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Font_Destroy(Font** the_font);






// **** xxx functions *****

//! Load a font into memory from disk, and create a font record from it
//! NOTE: this allocates new memory for the font, and copies the font data to it from the passed buffer. It is not dependent on the data in the buffer after returning.
//! This preliminary version is just a shell that does not read from disk, because no disk functionality available yet in f68/mcp as far as I know. Will switch to taking a path or something once disk is available. 
Font* Font_LoadFontData(unsigned char* the_data);



// **** Set xxx functions *****





// **** Get xxx functions *****





// **** Draw string functions *****


// Draw a string at the current "pen" location, using the current pen color of the bitmap
// Truncate, but still draw the string if it is too long to display on the line it started.
// No word wrap is performed. 
// If max_chars is less than the string length, only that many characters will be drawn (as space allows)
// If max_chars is -1, then the full string length will be drawn, as space allows.
bool Font_DrawString(Bitmap* the_bitmap, char* the_string, int16_t max_chars);

//! Draw a string in a rectangular block on the screen, with wrap.
//! The current font, pen location, and pen color of the bitmap will be used
//! If a word can't be wrapped, it will break the word and move on to the next line. So if you pass a rect with 1 char of width, it will draw a vertical line of chars down the screen.
//! @param	the_bitmap: a valid Bitmap object, with a valid font_ property
//! @param	width: the horizontal size of the text wrap box, in pixels. The total of 'width' and the current X coord of the bitmap must not be greater than width of the bitmap.
//! @param	height: the vertical size of the text wrap box, in pixels. The total of 'height' and the current Y coord of the bitmap must not be greater than height of the bitmap.
//! @param	the_string: the null-terminated string to be displayed.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed. Passing GEN_NO_STRLEN_CAP will mean it will attempt to display the entire string if it fits.
//! @param	wrap_buffer: pointer to a pointer to a temporary text buffer that can be used to hold the wrapped ('formatted') characters. The buffer must be large enough to hold num_chars of incoming text, plus additional line break characters where necessary. 
//! @param	continue_function: optional hook to a function that will be called if the provided text cannot fit into the specified box. If provided, the function will be called each time text exceeds available space. If the function returns true, another chunk of text will be displayed, replacing the first. If the function returns false, processing will stop. If no function is provided, processing will stop at the point text exceeds the available space.
//! @return	returns a pointer to the first character in the string after which it stopped processing (if string is too long to be displayed in its entirety). Returns the original string if the entire string was processed successfully. Returns NULL in the event of any error.
char* Font_DrawStringInBox(Bitmap* the_bitmap, int16_t width, int16_t height, char* the_string, int16_t num_chars, char** wrap_buffer, bool (* continue_function)(void));

//! Calculates how many characters of the passed string will fit into the passed pixel width.
//! The current font of the bitmap will be used as the basis for calculating fit.
//! @param	the_font: reference to a complete, loaded Font object.
//! @param	the_string: the null-terminated string to be measured.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed. Passing GEN_NO_STRLEN_CAP will mean it will attempt to measure the entire string.
//! @param	available_width: the width, in pixels, of the space the string is to be measured against.
//! @param	fixed_char_width: the width, in pixels, of one character. This value will be ignored. It exists to keep text-mode text-wrapping compatible with bitmap-font text-wrapping.
//! @param	measured_width: the number of pixels needed to display the characters that fit into the available_width. If the entire string fit, this is the width in pixels of that string. If only X characters fit, it is the pixel width of those X characters.
//! @return	returns -1 in any error condition, or the number of characters that fit. If the entire string fits, the passed len will be returned.
int16_t Font_MeasureStringWidth(Font* the_font, char* the_string, int16_t num_chars, int16_t available_width, int16_t fixed_char_width, int16_t* measured_width);


//! Draw one character on the bitmap, at the current bitmap pen coordinates
//! NOTE: if the draw action is successful, the bitmaps current pen position will be updated in preparation for the next character draw.
//! TODO: stop passing Font, and have the concept of a current font for a given bitmap. and maybe a default system font. 
//! @return:	Returns number of horizontal pixels used, including left/right offsets, or -1 on any error condition.
int16_t Font_DrawChar(Bitmap* the_bitmap, unsigned char the_char, Font* the_font);



// **** xxx functions *****




#endif /* LIB_FONT_H_ */


