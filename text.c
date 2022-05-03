/*
 * text.c
 *
 *  Created on: Feb 19, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "text.h"

// C includes
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A2560 includes
#include <mcp/syscalls.h>
#include "a2560_platform.h"
#include "general.h"
#include "lib_sys.h"


/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern System*			global_system;


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

//! \cond PRIVATE

//! Validate screen id, x, y, and colors
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position to validate. Must be between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position to validate. Must be between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15).
//! @param	back_color: Index to the desired background color (0-15).
//! @return	Returns false on any error/invalid input.
bool Text_ValidateAll(Screen* the_screen, int16_t x, int16_t y, uint8_t fore_color, uint8_t back_color);

//! Validate the coordinates are within the bounds of the specified screen. 
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position to validate. Must be between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position to validate. Must be between 0 and the screen's text_rows_vis_ - 1
bool Text_ValidateXY(Screen* the_screen, int16_t x, int16_t y);

//! Calculate the VRAM location of the specified coordinate
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
char* Text_GetMemLocForXY(Screen* the_screen, int16_t x, int16_t y, bool for_attr);

// Fill attribute or text char memory. Writes to char memory if for_attr is false.
// calling function must validate the screen ID before passing!
//! @return	Returns false on any error/invalid input.
bool Text_FillMemory(Screen* the_screen, bool for_attr, uint8_t the_fill);

//! Fill character and attribute memory for a specific box area
//! calling function must validate screen id, coords, attribute value before passing!
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	width: width, in character cells, of the rectangle to be filled
//! @param	height: height, in character cells, of the rectangle to be filled
//! @param	the_attribute_value: a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillMemoryBoxBoth(Screen* the_screen, int16_t x, int16_t y, int16_t width, int16_t height, unsigned char the_char, uint8_t the_attribute_value);

//! Fill character OR attribute memory for a specific box area
//! calling function must validate screen id, coords, attribute value before passing!
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	width: width, in character cells, of the rectangle to be filled
//! @param	height: height, in character cells, of the rectangle to be filled
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
//! @param	the_fill: either a 1-byte character code, or a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillMemoryBox(Screen* the_screen, int16_t x, int16_t y, int16_t width, int16_t height, bool for_attr, uint8_t the_fill);

//! \endcond


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

// **** NOTE: all functions in private section REQUIRE pre-validated parameters. 
// **** NEVER call these from your own functions. Always use the public interface. You have been warned!


//! \cond PRIVATE

//! Validate screen id, x, y, and colors
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position to validate. Must be between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position to validate. Must be between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15).
//! @param	back_color: Index to the desired background color (0-15).
//! @return	Returns false on any error/invalid input.
bool Text_ValidateAll(Screen* the_screen, int16_t x, int16_t y, uint8_t fore_color, uint8_t back_color)
{
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
			
	if (Text_ValidateXY(the_screen, x, y) == false)
	{
		LOG_ERR(("%s %d: illegal coordinates %li, %li", __func__, __LINE__, x, y));
		return false;
	}

	if (fore_color > 15 || back_color > 15)
	{
		LOG_INFO(("%s %d: illegal foreground (%u) or background (%u) color", __func__, __LINE__, fore_color, back_color));
		return false;
	}
	
	return true;
}

//! Validate the coordinates are within the bounds of the specified screen. 
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position to validate. Must be between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position to validate. Must be between 0 and the screen's text_rows_vis_ - 1
bool Text_ValidateXY(Screen* the_screen, int16_t x, int16_t y)
{
	int16_t			max_row;
	int16_t			max_col;
	
	max_col = the_screen->text_cols_vis_ - 1;
	max_row = the_screen->text_rows_vis_ - 1;
	
	if (x < 0 || x > max_col || y < 0 || y > max_row)
	{
		return false;
	}

	return true;
}


//! Calculate the VRAM location of the specified coordinate
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
char* Text_GetMemLocForXY(Screen* the_screen, int16_t x, int16_t y, bool for_attr)
{
	char*			the_write_loc;
	
	// LOGIC:
	//   For plotting the VRAM, A2560 uses the full width, regardless of borders. 
	//   So even if only 72 are showing, the screen is arranged from 0-71 for row 1, then 80-151 for row 2, etc. 
	
	if (for_attr)
	{
		the_write_loc = the_screen->text_attr_ram_ + (the_screen->text_mem_cols_ * y) + x;
	}
	else
	{
		the_write_loc = the_screen->text_ram_ + (the_screen->text_mem_cols_ * y) + x;
	}
	
	//DEBUG_OUT(("%s %d: screen=%i, x=%i, y=%i, for-attr=%i, calc=%i, loc=%p", __func__, __LINE__, (int16_t)the_screen_id, x, y, for_attr, (the_screen->text_mem_cols_ * y) + x, the_write_loc));
	
	return the_write_loc;
}


//! Fill attribute or text char memory. 
//! calling function must validate the screen ID before passing!
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
//! @param	the_fill: either a 1-byte character code, or a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillMemory(Screen* the_screen, bool for_attr, uint8_t the_fill)
{
	char*			the_write_loc;
	unsigned long	the_write_len;
	
	if (for_attr)
	{
		the_write_loc = the_screen->text_attr_ram_;
	}
	else
	{
		the_write_loc = the_screen->text_ram_;
	}

	the_write_len = the_screen->text_mem_cols_ * the_screen->text_mem_rows_;
	
	memset(the_write_loc, the_fill, the_write_len);

	return true;
}


//! Fill character and attribute memory for a specific box area
//! calling function must validate screen id, coords, attribute value before passing!
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	width: width, in character cells, of the rectangle to be filled
//! @param	height: height, in character cells, of the rectangle to be filled
//! @param	the_attribute_value: a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillMemoryBoxBoth(Screen* the_screen, int16_t x, int16_t y, int16_t width, int16_t height, uint8_t the_char, uint8_t the_attribute_value)
{
	char*			the_char_loc;
	char*			the_attr_loc;
	int16_t			max_row;
	
	// set up char and attribute memory initial loc
	the_char_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_CHAR);
	the_attr_loc = the_char_loc + (the_screen->text_attr_ram_ - the_screen->text_ram_);
	
	max_row = y + height;
	
	for (; y <= max_row; y++)
	{
		memset(the_char_loc, the_char, width);
		memset(the_attr_loc, the_attribute_value, width);
		the_char_loc += the_screen->text_mem_cols_;
		the_attr_loc += the_screen->text_mem_cols_;
	}
			
	return true;
}


//! Fill character OR attribute memory for a specific box area
//! calling function must validate screen id, coords, attribute value before passing!
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the starting horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the starting vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	width: width, in character cells, of the rectangle to be filled
//! @param	height: height, in character cells, of the rectangle to be filled
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
//! @param	the_fill: either a 1-byte character code, or a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillMemoryBox(Screen* the_screen, int16_t x, int16_t y, int16_t width, int16_t height, bool for_attr, uint8_t the_fill)
{
	char*			the_write_loc;
	int16_t			max_row;
	
	// set up initial loc
	the_write_loc = Text_GetMemLocForXY(the_screen, x, y, for_attr);
	
	max_row = y + height;
	
	for (; y <= max_row; y++)
	{
		memset(the_write_loc, the_fill, width);
		the_write_loc += the_screen->text_mem_cols_;
	}
			
	return true;
}

//! \endcond



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// ** NOTE: there is no destructor or constructor for this library, as it does not track any allocated memory. It works on the basis of a screen ID, which corresponds to the text memory for Vicky's Channel A and Channel B video memory.


// **** Block copy functions ****

//! Copy a full screen of attr from an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_source_buffer: valid pointer to attribute value data that will be copied to the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyAttrMemToScreen(Screen* the_screen, char* the_source_buffer)
{
	char*			the_vram_loc;
	unsigned long	the_write_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
	
	the_vram_loc = the_screen->text_attr_ram_;
	the_write_len = the_screen->text_cols_vis_ * the_screen->text_rows_vis_;
	
	memcpy(the_vram_loc, the_source_buffer, the_write_len);

	return true;
}

//! Copy a full screen of text attributes to an off-screen buffer.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_target_buffer: valid pointer to a block of memory big enough to hold the attribute data copied from the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyAttrMemFromScreen(Screen* the_screen, char* the_target_buffer)
{
	char*			the_vram_loc;
	unsigned long	the_write_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
	
	the_vram_loc = the_screen->text_attr_ram_;
	the_write_len = the_screen->text_cols_vis_ * the_screen->text_rows_vis_;
	
	memcpy(the_target_buffer, the_vram_loc, the_write_len);

	return true;
}

//! Copy a full screen of text from an off-screen buffer.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_source_buffer: valid pointer to character data that will be copied to the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyCharMemToScreen(Screen* the_screen, char* the_source_buffer)
{
	char*			the_vram_loc;
	unsigned long	the_write_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
	
	the_vram_loc = the_screen->text_ram_;
	the_write_len = the_screen->text_cols_vis_ * the_screen->text_rows_vis_;
	
	memcpy(the_vram_loc, the_source_buffer, the_write_len);

	return true;
}

//! Copy a full screen of text to an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_target_buffer: valid pointer to a block of memory big enough to hold the character data copied from the screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @return	Returns false on any error/invalid input.
bool Text_CopyCharMemFromScreen(Screen* the_screen, char* the_target_buffer)
{
	char*			the_vram_loc;
	unsigned long	the_write_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
	
	the_vram_loc = the_screen->text_ram_;
	the_write_len = the_screen->text_cols_vis_ * the_screen->text_rows_vis_;
	
	memcpy(the_target_buffer, the_vram_loc, the_write_len);

	return true;
}


//! Copy a full screen worth of EITHER text or attribute data to OR from an off-screen buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_buffer: valid pointer to a block of memory big enough to store (or alternatively act as the source of) the character or attribute data for the specified screen. It must be at least as large as the visible rows X visible columns of the screen.
//! @param	to_screen: true to copy to the screen from the buffer, false to copy from the screen to the buffer. Recommend using SCREEN_COPY_TO_SCREEN/SCREEN_COPY_FROM_SCREEN.
//! @param	for_attr: true to work with attribute data, false to work character data. Recommend using SCREEN_FOR_TEXT_ATTR/SCREEN_FOR_TEXT_CHAR.
//! @return	Returns false on any error/invalid input.
bool Text_CopyScreen(Screen* the_screen, char* the_buffer, bool to_screen, bool for_attr)
{
	char*			the_vram_loc;
	int16_t			the_write_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
		
	if (the_buffer == NULL)
	{
		LOG_ERR(("%s %d: passed off-screen buffer was NULL", __func__, __LINE__));
		return false;
	}

	if (for_attr)
	{
		the_vram_loc = the_screen->text_attr_ram_;
	}
	else
	{
		the_vram_loc = the_screen->text_ram_;
	}
	
	//the_write_len = the_screen->text_cols_vis_ * the_screen->text_rows_vis_;
	the_write_len = the_screen->text_mem_cols_ * the_screen->text_mem_rows_;

	if (to_screen)
	{
		memcpy(the_vram_loc, the_buffer, the_write_len);
	}
	else
	{
		memcpy(the_buffer, the_vram_loc, the_write_len);
	}

	return true;
}


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
bool Text_CopyMemBox(Screen* the_screen, char* the_buffer, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool to_screen, bool for_attr)
{
	char*			the_vram_loc;
	char*			the_buffer_loc;
	int16_t			the_write_len;
	int16_t			initial_offset;
	
	if (!Text_ValidateAll(the_screen, x1, y1, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}
	
	if (the_buffer == NULL)
	{
		LOG_ERR(("%s %d: passed off-screen buffer was NULL", __func__, __LINE__));
		return false;
	}

	// get initial read/write locs
	initial_offset = (the_screen->text_mem_cols_ * y1) + x1;
	the_buffer_loc = the_buffer + initial_offset;

	if (for_attr)
	{
		the_vram_loc = the_screen->text_attr_ram_ + initial_offset;
	}
	else
	{
		the_vram_loc = the_screen->text_ram_ + initial_offset;
	}
	
	// do copy one line at a time
	
	the_write_len = x2 - x1 + 1;

//	DEBUG_OUT(("%s %d: vramloc=%p, buffer=%p, bufferloc=%p, to_screen=%i, the_write_len=%i", the_vram_loc, the_buffer, the_buffer_loc, to_screen, the_write_len));

	for (; y1 <= y2; y1++)
	{
		if (to_screen)
		{
			memcpy(the_vram_loc, the_buffer_loc, the_write_len);
		}
		else
		{
			memcpy(the_buffer_loc, the_vram_loc, the_write_len);
		}
		
		the_buffer_loc += the_screen->text_mem_cols_;
		the_vram_loc += the_screen->text_mem_cols_;
	}

	return true;
}



// **** Block fill functions ****

//! Fill the entire attribute memory of the passed screen with the specified fore- and back-color
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_FillAttrMemForeBack(Screen* the_screen, uint8_t fore_color, uint8_t back_color)
{
	uint8_t			the_attribute_value;

	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);
	
	return Text_FillAttrMem(the_screen, the_attribute_value);
}


//! Fill the entire attribute memory of the passed screen
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_fill: a 1-byte attribute code (foreground in high nibble, background in low nibble)
//! @return	Returns false on any error/invalid input.
bool Text_FillAttrMem(Screen* the_screen, uint8_t the_fill)
{
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
	
 	return Text_FillMemory(the_screen, SCREEN_FOR_TEXT_ATTR, the_fill);
}


//! Fill the entire character memory of the passed screen
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	the_fill: a 1-byte character code
//! @return	Returns false on any error/invalid input.
bool Text_FillCharMem(Screen* the_screen, unsigned char the_fill)
{
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}
	
	return Text_FillMemory(the_screen, SCREEN_FOR_TEXT_CHAR, the_fill);
}


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
bool Text_FillBoxSlow(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice)
{
	int16_t		dx;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1,fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// add 1 to line len, because desired behavior is a box that connects fully to the passed coords
	dx = x2 - x1 + 1;
	
	for (; y1 <= y2; y1++)
	{
		if (!Text_DrawHLine(the_screen, x1, y1, dx, the_char, fore_color, back_color, the_draw_choice))
		{
			LOG_ERR(("%s %d: fill failed", __func__, __LINE__));
			return false;
		}
	}
			
	return true;
}


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
bool Text_FillBox(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char, uint8_t fore_color, uint8_t back_color)
{
	int16_t		dy;
	int16_t		dx;
	uint8_t		the_attribute_value;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// add 1 to line len, because desired behavior is a box that connects fully to the passed coords
	dx = x2 - x1 + 1;
	dy = y2 - y1 + 0;

	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);

	return Text_FillMemoryBoxBoth(the_screen, x1, y1, dx, dy, the_char, the_attribute_value);
}


//! Fill character memory for a specific box area
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used for the fill operation
//! @return	Returns false on any error/invalid input.
bool Text_FillBoxCharOnly(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char)
{
	int16_t		dy;
	int16_t		dx;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// add 1 to line len, because desired behavior is a box that connects fully to the passed coords
	dx = x2 - x1 + 1;
	dy = y2 - y1 + 1;

	return Text_FillMemoryBox(the_screen, x1, y1, dx, dy, SCREEN_FOR_TEXT_CHAR, the_char);
}


//! Fill attribute memory for a specific box area
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_FillBoxAttrOnly(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fore_color, uint8_t back_color)
{
	int16_t			dy;
	int16_t			dx;
	uint8_t			the_attribute_value;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// add 1 to line len, because desired behavior is a box that connects fully to the passed coords
	dx = x2 - x1 + 1;
	dy = y2 - y1 + 1;

	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);

	return Text_FillMemoryBox(the_screen, x1, y1, dx, dy, SCREEN_FOR_TEXT_ATTR, the_attribute_value);
}


//! Invert the colors of a rectangular block.
//! As this requires sampling each character cell, it is no faster (per cell) to do for entire screen as opposed to a subset box
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns false on any error/invalid input.
bool Text_InvertBox(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	uint8_t			the_attribute_value;
	uint8_t			the_inversed_value;
	char*			the_write_loc;
	int16_t			the_col;
	int16_t			skip_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// get initial read/write loc
	the_write_loc = Text_GetMemLocForXY(the_screen, x1, y1, SCREEN_FOR_TEXT_ATTR);	
	
	// amount of cells to skip past once we have written the specified line len
	skip_len = the_screen->text_mem_cols_ - (x2 - x1) - 1;

	for (; y1 <= y2; y1++)
	{
		for (the_col = x1; the_col <= x2; the_col++)
		{
			the_attribute_value = (unsigned char)*the_write_loc;
			
			// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
			the_inversed_value = (((the_attribute_value & 0x0F) << 4) | ((the_attribute_value & 0xF0) >> 4));
			
			*the_write_loc++ = the_inversed_value;
		}

		the_write_loc += skip_len;
	}

	return true;
}




// **** FONT RELATED *****

//! replace the current font data with the data at the passed memory buffer
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	new_font_data: Pointer to 2K (256 characters x 8 lines/bytes each) of font data. Each byte represents one line of an 8x8 font glyph.
//! @return	Returns false on any error/invalid input.
bool Text_UpdateFontData(Screen* the_screen, char* new_font_data)
{
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (new_font_data == NULL)
	{
		LOG_ERR(("%s %d: passed font data was NULL", __func__, __LINE__));
		return false;
	}

	memcpy(the_screen->text_font_ram_, new_font_data, 8*256*1);

	return true;
}


//! Test function to display all 256 font characters.
//! Characters are rendered in 8 rows of 32 characters.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	y: the starting row to draw the font characters.
//! @return	Returns false on any error/invalid input.
bool Text_ShowFontChars(Screen* the_screen, int16_t y)
{
	unsigned char	the_char = 0;
	char*			the_write_loc;
	int16_t			i;
	int16_t			j;

	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	the_write_loc = the_screen->text_ram_ + (the_screen->text_mem_cols_ * y);
	
	// print rows of 32 characters at a time
	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 32; i++)
		{
			*the_write_loc++ = the_char++;
		}
		
		the_write_loc += the_screen->text_mem_cols_ - i;
	}
	
	return true;
}



// **** Set char/attr functions *****


//! Set a char at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used
//! @return	Returns false on any error/invalid input.
bool Text_SetCharAtXY(Screen* the_screen, int16_t x, int16_t y, unsigned char the_char)
{
	char*	the_write_loc;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id or coordinate", __func__, __LINE__));
		return false;
	}
	
	the_write_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_CHAR);	
 	*the_write_loc = the_char;
	
	return true;
}


//! Set the attribute value at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_SetAttrAtXY(Screen* the_screen, int16_t x, int16_t y, uint8_t fore_color, uint8_t back_color)
{
	char*			the_write_loc;
	uint8_t			the_attribute_value;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}

	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);
	
	the_write_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_ATTR);	
 	*the_write_loc = the_attribute_value;
	
	return true;
}


//! Draw a char at a specified x, y coord, also setting the color attributes
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	the_char: the character to be used
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_SetCharAndColorAtXY(Screen* the_screen, int16_t x, int16_t y, unsigned char the_char, uint8_t fore_color, uint8_t back_color)
{
	char*			the_write_loc;
	uint8_t			the_attribute_value;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
		
	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);
	
	// write character memory
	the_write_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_CHAR);	
	*the_write_loc = the_char;
	
	// write attribute memory (reuse same calc, just add attr ram delta)
	the_write_loc += the_screen->text_attr_ram_ - the_screen->text_ram_;
	*the_write_loc = the_attribute_value;
	
	return true;
}


// **** Get char/attr functions *****


//! Get the char at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns a character code
unsigned char Text_GetCharAtXY(Screen* the_screen, int16_t x, int16_t y)
{
	char*			the_read_loc;
	unsigned char	the_char;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id or coordinate", __func__, __LINE__));
		return false;
	}
	
	the_read_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_CHAR);	
 	the_char = (unsigned char)*the_read_loc;
	
	return the_char;
}


//! Get the attribute value at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns a 1-byte attribute code (foreground in high nibble, background in low nibble)
uint8_t Text_GetAttrAtXY(Screen* the_screen, int16_t x, int16_t y)
{
	char*			the_read_loc;
	unsigned char	the_value;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id or coordinate", __func__, __LINE__));
		return false;
	}
	
	the_read_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_ATTR);	
 	the_value = (unsigned char)*the_read_loc;
	
	return the_value;
}


//! Get the foreground color at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns an index (0-15) to the foreground color LUT
uint8_t Text_GetForeColorAtXY(Screen* the_screen, int16_t x, int16_t y)
{
	char*			the_read_loc;
	unsigned char	the_value;
	unsigned char	the_color;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id or coordinate", __func__, __LINE__));
		return false;
	}
	
	the_read_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_ATTR);	
 	the_value = (unsigned char)*the_read_loc;
	the_color = (the_value & 0xF0) >> 4;
	
	return the_color;
}


//! Get the background color at a specified x, y coord
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x: the horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y: the vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @return	Returns an index (0-15) to the background color LUT
uint8_t Text_GetBackColorAtXY(Screen* the_screen, int16_t x, int16_t y)
{
	char*			the_read_loc;
	unsigned char	the_value;
	unsigned char	the_color;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, 0, 0))
	{
		LOG_ERR(("%s %d: illegal screen id or coordinate", __func__, __LINE__));
		return false;
	}
	
	the_read_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_ATTR);	
 	the_value = (unsigned char)*the_read_loc;
	the_color = (the_value & 0x0F);
	
	return the_color;
}



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
bool Text_DrawHLine(Screen* the_screen, int16_t x, int16_t y, int16_t the_line_len, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice)
{
	bool			result;
	
	// LOGIC: 
	//   an H line is just a box with 1 row, so we can re-use Text_FillMemoryBox(Both)(). These routines use memset, so are quicker than for loops. 
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}

	if (the_draw_choice == CHAR_ONLY)
	{
		result = Text_FillMemoryBox(the_screen, x, y, the_line_len, 0, SCREEN_FOR_TEXT_CHAR, the_char);
	}
	else
	{
		// calculate attribute value from passed fore and back colors
		// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground

		uint8_t			the_attribute_value;

		the_attribute_value = ((fore_color << 4) | back_color);
	
		if (the_draw_choice == ATTR_ONLY)
		{
			result = Text_FillMemoryBox(the_screen, x, y, the_line_len, 0, SCREEN_FOR_TEXT_ATTR, the_attribute_value);
		}
		else
		{
			result = Text_FillMemoryBoxBoth(the_screen, x, y, the_line_len, 0, the_char, the_attribute_value);
		}
	}

	return result;
}


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
bool Text_DrawHLineSlow(Screen* the_screen, int16_t x, int16_t y, int16_t the_line_len, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice)
{
	int16_t		dx;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}

	switch (the_draw_choice)
	{
		case CHAR_ONLY:
			for (dx = 0; dx < the_line_len; dx++)
			{
				Text_SetCharAtXY(the_screen, x + dx, y, the_char);
			}
			break;
			
		case ATTR_ONLY:
			for (dx = 0; dx < the_line_len; dx++)
			{
				Text_SetAttrAtXY(the_screen, x + dx, y, fore_color, back_color);
			}
			break;
			
		case CHAR_AND_ATTR:
		default:
			for (dx = 0; dx < the_line_len; dx++)
			{
				Text_SetCharAndColorAtXY(the_screen, x + dx, y, the_char, fore_color, back_color);		
			}
			break;			
	}

	return true;
}


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
bool Text_DrawVLine(Screen* the_screen, int16_t x, int16_t y, int16_t the_line_len, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice)
{
	int16_t		dy;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}

	switch (the_draw_choice)
	{
		case CHAR_ONLY:
			for (dy = 0; dy < the_line_len; dy++)
			{
				Text_SetCharAtXY(the_screen, x, y + dy, the_char);
			}
			break;
			
		case ATTR_ONLY:
			for (dy = 0; dy < the_line_len; dy++)
			{
				Text_SetAttrAtXY(the_screen, x, y + dy, fore_color, back_color);
			}
			break;
			
		case CHAR_AND_ATTR:
		default:
			for (dy = 0; dy < the_line_len; dy++)
			{
				Text_SetCharAndColorAtXY(the_screen, x, y + dy, the_char, fore_color, back_color);		
			}
			break;			
	}
	
	return true;
}


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
bool Text_DrawBoxCoords(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice)
{
	int16_t		dy;
	int16_t		dx;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1,fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// add 1 to line len, because desired behavior is a box that connects fully to the passed coords
	dx = x2 - x1 + 1;
	dy = y2 - y1 + 1;
	
	if (!Text_DrawHLine(the_screen, x1, y1, dx, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawVLine(the_screen, x2, y1, dy, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawHLine(the_screen, x1, y2, dx, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawVLine(the_screen, x1, y1, dy, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
		
	return true;
}


//! Draws a box based on 2 sets of coords, using the predetermined line and corner "graphics", and the passed colors
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	x1: the leftmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y1: the uppermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	x2: the rightmost horizontal position, between 0 and the screen's text_cols_vis_ - 1
//! @param	y2: the lowermost vertical position, between 0 and the screen's text_rows_vis_ - 1
//! @param	fore_color: Index to the desired foreground color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @param	back_color: Index to the desired background color (0-15). The predefined macro constants may be used (COLOR_DK_RED, etc.), but be aware that the colors are not fixed, and may not correspond to the names if the LUT in RAM has been modified.
//! @return	Returns false on any error/invalid input.
bool Text_DrawBoxCoordsFancy(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fore_color, uint8_t back_color)
{
	int16_t		dy;
	int16_t		dx;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x1, y1, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return false;
	}

	// add 1 to line len, because desired behavior is a box that connects fully to the passed coords
	dx = x2 - x1;
	dy = y2 - y1;
	
	// draw all lines one char shorter on each end so that we don't overdraw when we do corners
	
	if (!Text_DrawHLine(the_screen, x1+1, y1, dx, CH_WALL_H, fore_color, back_color, CHAR_AND_ATTR))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawHLine(the_screen, x1+1, y2, dx, CH_WALL_H, fore_color, back_color, CHAR_AND_ATTR))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawVLine(the_screen, x2, y1+1, dy, CH_WALL_V, fore_color, back_color, CHAR_AND_ATTR))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawVLine(the_screen, x1, y1+1, dy, CH_WALL_V, fore_color, back_color, CHAR_AND_ATTR))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	// draw the 4 corners with dedicated corner pieces
	Text_SetCharAndColorAtXY(the_screen, x1, y1, CH_WALL_UL, fore_color, back_color);		
	Text_SetCharAndColorAtXY(the_screen, x2, y1, CH_WALL_UR, fore_color, back_color);		
	Text_SetCharAndColorAtXY(the_screen, x2, y2, CH_WALL_LR, fore_color, back_color);		
	Text_SetCharAndColorAtXY(the_screen, x1, y2, CH_WALL_LL, fore_color, back_color);		
	
	return true;
}


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
bool Text_DrawBox(Screen* the_screen, int16_t x, int16_t y, int16_t width, int16_t height, unsigned char the_char, uint8_t fore_color, uint8_t back_color, text_draw_choice the_draw_choice)
{	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y,fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	if (!Text_ValidateXY(the_screen, x + width - 1, y + height - 1))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return false;
	}

	if (!Text_DrawHLine(the_screen, x, y, width, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawVLine(the_screen, x + width - 1, y, height, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawHLine(the_screen, x, y + height - 1, width, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
	
	if (!Text_DrawVLine(the_screen, x, y, height, the_char, fore_color, back_color, the_draw_choice))
	{
		LOG_ERR(("%s %d: draw box failed", __func__, __LINE__));
		return false;
	}
		
	return true;
}



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
bool Text_DrawStringAtXY(Screen* the_screen, int16_t x, int16_t y, char* the_string, uint8_t fore_color, uint8_t back_color)
{
	char*			the_char_loc;
	char*			the_attr_loc;
	uint8_t			the_attribute_value;
	int16_t			i;
	int16_t			max_col;
	int16_t			draw_len;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return false;
	}

	if (!Text_ValidateAll(the_screen, x, y, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return false;
	}
	
	draw_len = General_Strnlen(the_string, the_screen->text_mem_cols_); // can't be wider than the screen anyway
	max_col = the_screen->text_cols_vis_ - 1;
	
	if ( x + draw_len > max_col)
	{
		draw_len = (max_col - x) + 1;
	}
	
	//DEBUG_OUT(("%s %d: draw_len=%i, max_col=%i, x=%i", __func__, __LINE__, draw_len, max_col, x));
	
	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);

	// set up char and attribute memory initial loc
	the_char_loc = Text_GetMemLocForXY(the_screen, x, y, SCREEN_FOR_TEXT_CHAR);
	the_attr_loc = the_char_loc + (the_screen->text_attr_ram_ - the_screen->text_ram_);
	
	// draw the string
	for (i = 0; i < draw_len; i++)
	{
		*the_char_loc++ = the_string[i];
		*the_attr_loc++ = the_attribute_value;
	}
	
	return true;
}


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
char* Text_DrawStringInBox(Screen* the_screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2, char* the_string, uint8_t fore_color, uint8_t back_color, bool (* continue_function)(void))
{
	char*			the_char_loc;
	char*			needs_formatting;
	char*			needed_formatting_last_round;
	char*			formatted_string;
	uint8_t			the_attribute_value;
	int16_t			max_col;
	int16_t			max_pix_width;
	int16_t			max_pix_height;
	int16_t			this_line_len;
	bool			do_another_round = false;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		return NULL;
	}

	if (!Text_ValidateAll(the_screen, x1, y1, fore_color, back_color))
	{
		LOG_ERR(("%s %d: illegal screen id, coordinate, or color", __func__, __LINE__));
		return NULL;
	}
	
	if (!Text_ValidateXY(the_screen, x2, y2))
	{
		LOG_ERR(("%s %d: illegal coordinate", __func__, __LINE__));
		return NULL;
	}

	if (x1 > x2 || y1 > y2)
	{
		LOG_ERR(("%s %d: illegal coordinates", __func__, __LINE__));
		return NULL;
	}
	
	max_col = x2 - x1 + 1;
	max_pix_width = (x2 - x1 + 1) * the_screen->text_font_width_;
	max_pix_height = (y2 - y1 + 1) * the_screen->text_font_height_;
	
	//DEBUG_OUT(("%s %d: draw_len=%i, max_col=%i, x=%i", __func__, __LINE__, draw_len, max_col, x));
	
	// calculate attribute value from passed fore and back colors
	// LOGIC: text mode only supports 16 colors. lower 4 bits are back, upper 4 bits are foreground
	the_attribute_value = ((fore_color << 4) | back_color);

	formatted_string = the_screen->text_temp_buffer_2_;
	needs_formatting = the_string;
	needed_formatting_last_round = needs_formatting;

	// outer loop: iterate on one-box worth of text until all text is displayed, or calling function no longer wants to proceed
	do
	{	
		char*			the_attr_loc;
		char*			remaining_string;
		int16_t			remaining_len;
		int16_t			orig_len;
		int16_t			v_pixels;
		int16_t			num_rows;
		int16_t			the_row;

		remaining_len = General_Strnlen(needs_formatting, the_screen->text_mem_cols_ * the_screen->text_mem_rows_ + 1); // can't be bigger than the screen (80x60=4800). +1 for terminator. 
		orig_len = remaining_len;

		// clear out the word wrap buffer in case anything had been there before. Shouldn't be necessary, but something weird happening in some cases with 2nd+ wrap, and this does prevent it.
		memset(formatted_string, 0, TEXT_COL_COUNT_FOR_PLOTTING_A2560K * TEXT_ROW_COUNT_FOR_PLOTTING_A2560K + 1);
		
		// format the string into chunks that will fit in the width specified, with line breaks on each line
		v_pixels = General_WrapAndTrimTextToFit(&needs_formatting, &formatted_string, orig_len, max_pix_width, max_pix_height, the_screen->text_font_width_, the_screen->text_font_height_, NULL, &Text_MeasureStringWidth);
		num_rows = v_pixels / the_screen->text_font_height_;
	
		// LOGIC:
		//   needs_formatting is now either pointing at next char after cutoff (it not all text fit), or at itself still (if all text fit). 
		//   we can detect if all chars fit by comparing needs_formatting to needed_formatting_last_round
	
		//DEBUG_OUT(("%s %d: v_pixels=%i, num_rows=%i, remaining_len=%i, needs_formatting=%p, the_string=%p", __func__, __LINE__, v_pixels, num_rows, remaining_len, needs_formatting, the_string));
		remaining_string = formatted_string;
	
		// clear the target box area on the screen -- if fail to do this, when we draw page 2, etc, it will be messy.
		Text_FillBox(the_screen, x1, y1, x2, y2, ' ', fore_color, back_color);

		// set up char and attribute memory initial loc
		the_char_loc = Text_GetMemLocForXY(the_screen, x1, y1, SCREEN_FOR_TEXT_CHAR);
		the_attr_loc = the_char_loc + (the_screen->text_attr_ram_ - the_screen->text_ram_);

		// draw the string, one line at a time, until string is done or no more lines available
	
		the_row = 0;
	
		do
		{
			int16_t		this_write_len;
		
			this_line_len = General_StrFindNextLineBreak(remaining_string, max_col);

			if (this_line_len == 1)
			{
				// next/first character is a line break char
				this_write_len = 0;
				*(the_attr_loc) = the_attribute_value;
			}
			else
			{
				if (this_line_len == 0)
				{
					// there is no other line break char left in the string.
					this_write_len = General_Strnlen(remaining_string, remaining_len) - 0;
					//DEBUG_OUT(("%s %d: this_write_len=%i, remaining_len=%i, remaining='%s'", __func__ , __LINE__, this_write_len, remaining_len, remaining_string));
				}
				else
				{
					// there is a line break character, but some other chars come first. write up to be not including the line break
					this_write_len = this_line_len - 1; // stop short of the the actual \n char.
					//DEBUG_OUT(("%s %d: this_line_len=%i, remaining_len=%i, remaining='%s'", __func__ , __LINE__, this_write_len, this_line_len, remaining_string));
				}
		
				memcpy(the_char_loc, remaining_string, this_write_len);
				memset(the_attr_loc, the_attribute_value, this_write_len);
			}

			remaining_string += this_write_len + 1; // skip past the actual \n char.
			remaining_len -= (this_write_len + 1);
			//DEBUG_OUT(("%s %d: remaining_len=%i, remaining='%s'", __func__ , __LINE__, remaining_len, remaining_string));
			
			the_char_loc += the_screen->text_mem_cols_;
			the_attr_loc += the_screen->text_mem_cols_;		
			the_row++;
			//DEBUG_OUT(("%s %d: the_row=%i, num_rows=%i, this_line_len=%i", __func__ , __LINE__, the_row, num_rows, this_line_len));
		} while (the_row < num_rows && this_line_len > 0);
	
		// any more text still to format?
		if (needs_formatting == needed_formatting_last_round)
		{
			// all chars fit
			return the_string;
		}
		else
		{
			// some chars didn't fit - let calling function determine if it wants to display more
			if (continue_function == NULL)
			{
				// no hook provided, just return
				return needs_formatting;
			}
			else
			{
				if ((*continue_function)() == true)
				{
					// show next portion
					do_another_round = true;
					needed_formatting_last_round = needs_formatting; // reset to end results of this round so can check what happens next round
				}
				else
				{
					// calling function indicated it didn't want to display next portion
					return needs_formatting;
				}
			}		
		}
	} while (do_another_round == true);
	
	return needs_formatting;
}


//! Calculates how many characters of the passed string will fit into the passed pixel width.
//! In Text Mode, all characters have the same fixed width, so this is measuring against the font width described in the screen object.
//! @param	the_font: this is for consistency with the graphical font code. Pass a NULL here, the result will not be used.
//! @param	the_string: the null-terminated string to be measured.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed. Passing GEN_NO_STRLEN_CAP will mean it will attempt to measure the entire string.
//! @param	available_width: the width, in pixels, of the space the string is to be measured against.
//! @param	fixed_char_width: the width, in pixels, of one character.
//! @param	measured_width: the number of pixels needed to display the characters that fit into the available_width. If the entire string fit, this is the width in pixels of that string. If only X characters fit, it is the pixel width of those X characters.
//! @return	Returns -1 in any error condition, or the number of characters that fit. If the entire string fits, the passed len will be returned.
int16_t Text_MeasureStringWidth(Font* the_font, char* the_string, int16_t num_chars, int16_t available_width, int16_t fixed_char_width, int16_t* measured_width)
{
	int16_t			fit_count;
	int16_t			required_width;
	
	if (num_chars == 0)
	{
		return -1;
	}
	
	// num_chars will be GEN_NO_STRLEN_CAP (-1) if the calling method wants us to display the entire string. 
	if (num_chars == GEN_NO_STRLEN_CAP)
	{
		num_chars = General_Strnlen(the_string, WORD_WRAP_MAX_LEN);
	}
	
	// LOGIC:
	//   in a system that had proportionally spaced fonts, we would examine each character in the string, and get the width of that char
	//   the foenix computers currently only offer fixed with fonts in their text mode
	//   if foenix or users add a way to do proportionally spaced fonts in the future, this will need a helper routine a la Amiga's TextFit()
	//   for now, we can just multiply chars * char width
	
	required_width = num_chars * fixed_char_width;
	
	if (available_width >= required_width)
	{
		fit_count = num_chars;
		*measured_width = required_width;
	}
	else
	{
		fit_count = available_width / fixed_char_width;
		*measured_width = available_width;
	}
	
	return fit_count;	
}




