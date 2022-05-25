//! @file text.h

/*
 * text.h
 *
*  Created on: Feb 19, 2022
 *      Author: micahbly
 */

#ifndef LIB_TEXT_H_
#define LIB_TEXT_H_


/* about this library: TextDisplay
 *
 * This handles writing and reading information to/from the VICKY's text mode memory
 *
 *** things this library needs to be able to do
 * work with either channel A or channel B
 * clear / fill an entire screen of text characters
 * clear / fill an entire screen of text attributes
 * invert the colors of a screen
 * clear / fill a smaller-than-screen rectangular area of text/attrs
 * Draw a char to a specified x, y coord
 * Get the currently displayed character at the specified coord
 * Set the foreground and background colors at the specified coord
 * Set the attribute value at the specified coord
 * Get the attribute value at the specified coord
 * Get the foreground or background color at the specified coord
 * draw a line using "graphic" characters
 * draw a box using "graphic" characters
 * copy a full screen of text or attr from an off-screen buffer
 * copy a full screen of text or attr TO an off-screen buffer
 * copy a full screen of text and attr between channel A and B
 * copy a rectangular area of text or attr TO/FROM an off-screen buffer
 * display a string at a specified x, y coord (no wrap)
 * display a pre-formatted string in a rectangular block on the screen, breaking on \n characters
 * display a string in a rectangular block on the screen, with wrap
 * display a string in a rectangular block on the screen, with wrap, taking a hook for a "display more" event, and scrolling text vertically up after hook func returns 'continue' (or exit, returning control to calling func, if hook returns 'stop')
 * replace current text font with another, loading from specified ram loc.
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes

// C includes
#include <stdbool.h>

// A2560 includes
#include "a2560_platform.h"
#include "general.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define SCREEN_FOR_TEXT_ATTR	true	///< param for functions with for_attr
#define SCREEN_FOR_TEXT_CHAR	false	// param for functions with for_attr

#define SCREEN_COPY_TO_SCREEN	true	// param for functions doing block copy to/from screen / off-screen buffer
#define SCREEN_COPY_FROM_SCREEN	false	// param for functions doing block copy to/from screen / off-screen buffer

// based on observations in f68 with names matched to ANSI colors
// https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit
#define COLOR_BLACK				(unsigned char)0x00
#define COLOR_RED				(unsigned char)0x01
#define COLOR_GREEN				(unsigned char)0x02
#define COLOR_YELLOW			(unsigned char)0x03
#define COLOR_BLUE				(unsigned char)0x04
#define COLOR_MAGENTA			(unsigned char)0x05
#define COLOR_CYAN				(unsigned char)0x06
#define COLOR_WHITE				(unsigned char)0x07
#define COLOR_GRAY				(unsigned char)0x08
#define COLOR_BRIGHT_RED		(unsigned char)0x09
#define COLOR_BRIGHT_GREEN		(unsigned char)0x0A
#define COLOR_BRIGHT_YELLOW		(unsigned char)0x0B
#define COLOR_BRIGHT_BLUE		(unsigned char)0x0C
#define COLOR_BRIGHT_MAGENTA	(unsigned char)0x0D
#define COLOR_BRIGHT_CYAN		(unsigned char)0x0E
#define COLOR_BRIGHT_WHITE		(unsigned char)0x0F

// I believe foreground and background colors can be defined differently, but from testing on morfe, they seem to be the same at least by default.
#define FG_COLOR_BLACK			(unsigned char)0x00
#define FG_COLOR_RED			(unsigned char)0x01
#define FG_COLOR_GREEN			(unsigned char)0x02
#define FG_COLOR_YELLOW			(unsigned char)0x03
#define FG_COLOR_BLUE			(unsigned char)0x04
#define FG_COLOR_MAGENTA		(unsigned char)0x05
#define FG_COLOR_CYAN			(unsigned char)0x06
#define FG_COLOR_WHITE			(unsigned char)0x07
#define FG_COLOR_GRAY			(unsigned char)0x08
#define FG_COLOR_BRIGHT_RED		(unsigned char)0x09
#define FG_COLOR_BRIGHT_GREEN	(unsigned char)0x0A
#define FG_COLOR_BRIGHT_YELLOW	(unsigned char)0x0B
#define FG_COLOR_BRIGHT_BLUE	(unsigned char)0x0C
#define FG_COLOR_BRIGHT_MAGENTA	(unsigned char)0x0D
#define FG_COLOR_BRIGHT_CYAN	(unsigned char)0x0E
#define FG_COLOR_BRIGHT_WHITE	(unsigned char)0x0F

#define BG_COLOR_BLACK			(unsigned char)0x00
#define BG_COLOR_RED			(unsigned char)0x01
#define BG_COLOR_GREEN			(unsigned char)0x02
#define BG_COLOR_YELLOW			(unsigned char)0x03
#define BG_COLOR_BLUE			(unsigned char)0x04
#define BG_COLOR_MAGENTA		(unsigned char)0x05
#define BG_COLOR_CYAN			(unsigned char)0x06
#define BG_COLOR_WHITE			(unsigned char)0x07
#define BG_COLOR_GRAY			(unsigned char)0x08
#define BG_COLOR_BRIGHT_RED		(unsigned char)0x09
#define BG_COLOR_BRIGHT_GREEN	(unsigned char)0x0A
#define BG_COLOR_BRIGHT_YELLOW	(unsigned char)0x0B
#define BG_COLOR_BRIGHT_BLUE	(unsigned char)0x0C
#define BG_COLOR_BRIGHT_MAGENTA	(unsigned char)0x0D
#define BG_COLOR_BRIGHT_CYAN	(unsigned char)0x0E
#define BG_COLOR_BRIGHT_WHITE	(unsigned char)0x0F

// update: the numbers shown in vicky2 file in morfe don't match up to what's shown on screen, at least with a2560 config. eg, 20/00/00 is not a super dark blue, it's some totally bright thing. need to spend some time mapping these out better. But since user configurable, will wait until real machine comes and I can make sure of what's in flash rom. 

/*****************************************************************************/
/*                  Character-codes (IBM Page 437 charset)                   */
/*****************************************************************************/
// https://en.wikipedia.org/wiki/Code_page_437

#define CH_CHECKERED1	(unsigned char)0xB0
#define CH_CHECKERED2	(unsigned char)0xB1
#define CH_CHECKERED3	(unsigned char)0xB2
#define CH_SPACE		(unsigned char)0x20	// regular space
#define CH_SOLID		(unsigned char)0xDB	// inverse space
#define CH_WALL_H		(unsigned char)0xC4
#define CH_WALL_V		(unsigned char)0xB3
#define CH_WALL_UL		(unsigned char)0xDA
#define CH_WALL_UR		(unsigned char)0xBF
#define CH_WALL_LL		(unsigned char)0xC0
#define CH_WALL_LR		(unsigned char)0xD9
#define CH_INTERSECT	(unsigned char)0xC5
#define CH_T_DOWN		(unsigned char)0xC2 // T-shape pointing down
#define CH_T_UP			(unsigned char)0xC1 // T-shape pointing up
#define CH_T_LEFT		(unsigned char)0xB4 // T-shape pointing left
#define CH_T_RIGHT		(unsigned char)0xC3 // T-shape pointing right
#define CH_SMILEY1		(unsigned char)0x01 // 
#define CH_SMILEY2		(unsigned char)0x02 // 
#define CH_HEART		(unsigned char)0x03 // 
#define CH_DIAMOND		(unsigned char)0x04 // 
#define CH_CLUB			(unsigned char)0x05 // 
#define CH_SPADE		(unsigned char)0x06 // 
#define CH_MIDDOT		(unsigned char)0x07 // 
#define CH_RIGHT		(unsigned char)0x10 // Triangle pointing right
#define CH_LEFT			(unsigned char)0x11 // Triangle pointing left
#define CH_UP			(unsigned char)0x1E // Triangle pointing up
#define CH_DOWN			(unsigned char)0x1F // Triangle pointing down


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum text_draw_choice
{
	CHAR_ONLY		= 0,
	ATTR_ONLY 		= 1,
	CHAR_AND_ATTR	= 2,
} text_draw_choice;


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// ** NOTE: there is no destructor or constructor for this library, as it does not track any allocated memory.


// **** Block copy functions ****

//! Copy a full screen of attr from an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_source_buffer: valid pointer to attribute value data that will be copied to the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyAttrMemToScreen(Screen* the_screen, char* the_source_buffer);

//! Copy a full screen of text attributes to an off-screen buffer.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_target_buffer: valid pointer to a block of memory big enough to hold the attribute data copied from the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyAttrMemFromScreen(Screen* the_screen, char* the_target_buffer);

//! Copy a full screen of text from an off-screen buffer.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_source_buffer: valid pointer to character data that will be copied to the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyCharMemToScreen(Screen* the_screen, char* the_source_buffer);

//! Copy a full screen of text to an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_target_buffer: valid pointer to a block of memory big enough to hold the character data copied from the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyCharMemFromScreen(Screen* the_screen, char* the_target_buffer);

//! Copy a full screen worth of EITHER text or attribute data to OR from an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_buffer: valid pointer to a block of memory big enough to store (or alternatively act as the source of) the character or attribute data for the specified screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @param	to_screen: true to copy to the screen from the buffer, false to copy from the screen to the buffer. Recommend using SCREEN_COPY_TO_SCREEN/SCREEN_COPY_FROM_SCREEN.
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
//! @return	Returns false on any error/invalid input.
bool Text_CopyScreen(Screen* the_screen, char* the_buffer, bool to_screen, bool for_attr);

//! Copy a rectangular area of text or attr to or from an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_buffer: valid pointer to a block of memory big enough to store (or alternatively act as the source of) the character or attribute data for the specified rectangle of screen memory.
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	to_screen: true to copy to the screen from the buffer, false to copy from the screen to the buffer. Recommend using SCREEN_COPY_TO_SCREEN/SCREEN_COPY_FROM_SCREEN.
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
//! @return	Returns false on any error/invalid input.
bool Text_CopyMemBox(Screen* the_screen, char* the_buffer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool to_screen, bool for_attr);


// **** Block fill functions ****


//! Clear the text screen and reset foreground and background colors
void Text_ClearScreen(Screen* the_screen, uint8_t fore_color, uint8_t back_color);

//! Fill the entire attribute memory of the passed screen with the specified fore- and back-color
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_FillAttrMemForeBack(Screen* the_screen, uint8_t fore_color, uint8_t back_color);

//! Fill the entire attribute memory of the passed screen
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_fill: a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillAttrMem(Screen* the_screen, uint8_t the_fill);

//! Fill the entire character memory of the passed screen
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_fill: a 1-byte character code
//! @return	Returns false on any error/invalid input.
bool Text_FillCharMem(Screen* the_screen, unsigned char the_fill);

//! Fill a box-shaped section of character and/or attribute memory
//! This version uses char-by-char functions, so it is very slow. It will be removed before release. 
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used for the fill operation
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	the_draw_choice: controls the scope of the action, and is one of CHAR_ONLY, ATTR_ONLY, or CHAR_AND_ATTR. See the text_draw_choice enum.
//! @return	Returns false on any error/invalid input.
bool Text_FillBoxSlow(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice);

//! Fill character and attribute memory for a specific box area
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used for the fill operation
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_FillBox(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char, uint8_t fore_color, uint8_t back_color);

//! Fill character memory for a specific box area
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used for the fill operation
//! @return	Returns false on any error/invalid input.
bool Text_FillBoxCharOnly(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char);

//! Fill attribute memory for a specific box area
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_FillBoxAttrOnly(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fore_color, uint8_t back_color);

//! Invert the colors of a rectangular block.
//! As this requires sampling each character cell, it is no faster (per cell) to do for entire screen as opposed to a subset box
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns false on any error/invalid input.
bool Text_InvertBox(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2);




// **** FONT RELATED *****


//! replace the current font data with the data at the passed memory buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	new_font_data: Pointer to 2K (256 characters x 8 lines/bytes each) of font data. Each byte represents one line of an 8x8 font glyph.
//! @return	Returns false on any error/invalid input.
bool Text_UpdateFontData(Screen* the_screen, char* new_font_data);

//! Test function to display all 256 font characters.
//! Characters are rendered in 8 rows of 32 characters.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	y: the starting row to draw the font characters.
//! @return	Returns false on any error/invalid input.
bool Text_ShowFontChars(Screen* the_screen, int16_t y);




// **** Set char/attr functions *****


//! Set a char at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used
//! @return	Returns false on any error/invalid input.
bool Text_SetCharAtXY(Screen* the_screen, int16_t x, int16_t y, unsigned char the_char);

//! Set the attribute value at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_SetAttrAtXY(Screen* the_screen, int16_t x, int16_t y, uint8_t fore_color, uint8_t back_color);

//! Draw a char at a specified x, y coord, also setting the color attributes
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_SetCharAndColorAtXY(Screen* the_screen, int16_t x, int16_t y, unsigned char the_char, uint8_t fore_color, uint8_t back_color);




// **** Get char/attr functions *****


//! Get the char at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns a character code
unsigned char Text_GetCharAtXY(Screen* the_screen, int16_t x, int16_t y);

//! Get the attribute value at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns a 1-byte attribute code (foreground in high nibble, background in low nibble)
uint8_t Text_GetAttrAtXY(Screen* the_screen, int16_t x, int16_t y);

//! Get the foreground color at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns an index (0-15) to the foreground color LUT
uint8_t Text_GetForeColorAtXY(Screen* the_screen, int16_t x, int16_t y);

//! Get the background color at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns an index (0-15) to the background color LUT
uint8_t Text_GetBackColorAtXY(Screen* the_screen, int16_t x, int16_t y);



// **** Drawing functions *****


//! Draws a horizontal line from specified coords, for n characters, using the specified char and/or attribute
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_line_len: The total length of the line, in characters, including the start and end character.
//! @param	the_char: the character to be used when drawing
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	the_draw_choice: controls the scope of the action, and is one of CHAR_ONLY, ATTR_ONLY, or CHAR_AND_ATTR. See the text_draw_choice enum.
//! @return	Returns false on any error/invalid input.
bool Text_DrawHLine(Screen* the_screen, int16_t x, int16_t y, int16_t the_line_len, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice);

//! Draws a horizontal line from specified coords, for n characters, using the specified char and/or attribute
//! This version uses char-by-char functions, so it is very slow. It will be removed before release. 
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_line_len: The total length of the line, in characters, including the start and end character.
//! @param	the_char: the character to be used when drawing
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	the_draw_choice: controls the scope of the action, and is one of CHAR_ONLY, ATTR_ONLY, or CHAR_AND_ATTR. See the text_draw_choice enum.
//! @return	Returns false on any error/invalid input.
bool Text_DrawHLineSlow(Screen* the_screen, int16_t x, int16_t y, int16_t the_line_len, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice);

//! Draws a vertical line from specified coords, for n characters, using the specified char and/or attribute
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_line_len: The total length of the line, in characters, including the start and end character.
//! @param	the_char: the character to be used when drawing
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	the_draw_choice: controls the scope of the action, and is one of CHAR_ONLY, ATTR_ONLY, or CHAR_AND_ATTR. See the text_draw_choice enum.
//! @return	Returns false on any error/invalid input.
bool Text_DrawVLine(Screen* the_screen, int16_t x, int16_t y, int16_t the_line_len, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice);

//! Draws a basic box based on 2 sets of coords, using the specified char and/or attribute for all cells
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used when drawing the box
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	the_draw_choice: controls the scope of the action, and is one of CHAR_ONLY, ATTR_ONLY, or CHAR_AND_ATTR. See the text_draw_choice enum.
//! @return	Returns false on any error/invalid input.
bool Text_DrawBoxCoords(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice);

//! Draws a box based on 2 sets of coords, using the predetermined line and corner "graphics", and the passed colors
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_DrawBoxCoordsFancy(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fore_color, uint8_t back_color);

//! Draws a basic box based on start coords and width/height, using the specified char and/or attribute for all cells
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	width: width, in character cells, of the rectangle to be drawn
//! @param	height: height, in character cells, of the rectangle to be drawn
//! @param	the_char: the character to be used when drawing the box
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	the_draw_choice: controls the scope of the action, and is one of CHAR_ONLY, ATTR_ONLY, or CHAR_AND_ATTR. See the text_draw_choice enum.
//! @return	Returns false on any error/invalid input.
bool Text_DrawBox(Screen* the_screen, int16_t x, int16_t y, int16_t width, int16_t height, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice);



// **** Draw string functions *****


//! Draw a string at a specified x, y coord, also setting the color attributes.
//! If it is too long to display on the line it started, it will be truncated at the right edge of the screen.
//! No word wrap is performed. 
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_string: the null-terminated string to be drawn
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_DrawStringAtXY(Screen* the_screen, int16_t x, int16_t y, char* the_string, uint8_t fore_color, uint8_t back_color);

//! Draw a string in a rectangular block on the screen, with wrap.
//! If a word can't be wrapped, it will break the word and move on to the next line. So if you pass a rect with 1 char of width, it will draw a vertical line of chars down the screen.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_string: the null-terminated string to be displayed.
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	continue_function: optional hook to a function that will be called if the provided text cannot fit into the specified box. If provided, the function will be called each time text exceeds available space. If the function returns true, another chunk of text will be displayed, replacing the first. If the function returns false, processing will stop. If no function is provided, processing will stop at the point text exceeds the available space.
//! @return	Returns a pointer to the first character in the string after which it stopped processing (if string is too long to be displayed in its entirety). Returns the original string if the entire string was processed successfully. Returns NULL in the event of any error.
char* Text_DrawStringInBox(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, char* the_string, uint8_t fore_color, uint8_t back_color, bool (* continue_function)(void));

//! Calculates how many characters of the passed string will fit into the passed pixel width.
//! In Text Mode, all characters have the same fixed width, so this is measuring against the font width described in the screen object.
//! @param	the_font: this is for consistency with the graphical font code. Pass a NULL here, the result will not be used.
//! @param	the_string: the null-terminated string to be measured.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed. Passing GEN_NO_STRLEN_CAP will mean it will attempt to measure the entire string.
//! @param	available_width: the width, in pixels, of the space the string is to be measured against.
//! @param	fixed_char_width: the width, in pixels, of one character.
//! @param	measured_width: the number of pixels needed to display the characters that fit into the available_width. If the entire string fit, this is the width in pixels of that string. If only X characters fit, it is the pixel width of those X characters.
//! @return	Returns -1 in any error condition, or the number of characters that fit. If the entire string fits, the passed len will be returned.
int16_t Text_MeasureStringWidth(Font* the_font, char* the_string, int16_t num_chars, int16_t available_width, int16_t fixed_char_width, int16_t* measured_width);






#endif /* LIB_TEXT_H_ */
