/*
 * font.c
 *
 *  Created on: Mar 15, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "font.h"

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
#include "bitmap.h"
#include "text.h"


/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/



/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

//! Get the total width, in pixels, for the specified character, including any whitespace
uint8_t Font_GetCharWidth(Font* the_font, unsigned char the_char);

//! Get the total height, in pixels, of one line of text, including any leading
uint8_t Font_GetRowHeight(Font* the_font);

//! Get the width of characters, in pixels, for fixed width fonts (all fonts will report one)
uint8_t Font_GetFixedWidth(Font* the_font);


// **** Debug functions *****

void Font_Print(Font* the_font);

/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

// **** NOTE: all functions in private section REQUIRE pre-validated parameters. 
// **** NEVER call these from your own functions. Always use the public interface. You have been warned!


//! Get the total width, in pixels, for the specified character, including any whitespace
uint8_t Font_GetCharWidth(Font* the_font, unsigned char the_char)
{
	int16_t			offset_width_value;
	int8_t			width_value;			//!< the total width of the character including any whitespace to left/right

	offset_width_value = the_font->width_table_[the_char];
	
	if (offset_width_value == -1)
	{
		// offset/width table says this char does not exist in the font		
		// switch to the "missing glyph" character, which is the last one in the font.
		the_char = the_font->lastChar + 1;
		offset_width_value = the_font->width_table_[the_char];
	}

	width_value = offset_width_value & 0xFF;
	
	return width_value;
}


//! Get the total height, in pixels, of one line of text, including any leading
uint8_t Font_GetRowHeight(Font* the_font)
{
	return the_font->leading + the_font->fRectHeight;
}

//! Get the width of characters, in pixels, for fixed width fonts (all fonts will report one)
uint8_t Font_GetFixedWidth(Font* the_font)
{
	return the_font->leading + the_font->widMax;
}


// **** Debug functions *****

void Font_Print(Font* the_font)
{
	DEBUG_OUT(("Font print out:"));
	DEBUG_OUT(("  fontType: %i", the_font->fontType));
	DEBUG_OUT(("  firstChar: %i", the_font->firstChar));
	DEBUG_OUT(("  lastChar: %i", the_font->lastChar));
	DEBUG_OUT(("  widMax: %i", the_font->widMax));
	DEBUG_OUT(("  kernMax: %i", the_font->kernMax));
	DEBUG_OUT(("  nDescent: %i", the_font->nDescent));
	DEBUG_OUT(("  fRectWidth: %i", the_font->fRectWidth));
	DEBUG_OUT(("  fRectHeight: %i", the_font->fRectHeight));
	DEBUG_OUT(("  owTLoc: %u", the_font->owTLoc));
	DEBUG_OUT(("  ascent: %i", the_font->ascent));
	DEBUG_OUT(("  descent: %i", the_font->descent));
	DEBUG_OUT(("  leading: %i", the_font->leading));
	DEBUG_OUT(("  rowWords: %i", the_font->rowWords));	
}




/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Create a Font object, and populate it from the passed buffer.
//! NOTE: it is not possible to create a Font object without having a valid 'FONT' data chunk already in memory.
//! NOTE: this allocates new memory for the font, and copies the font data to it from the passed buffer. It is not dependent on the data in the buffer after returning.
//! @param	the_data: Must contain a valid Mac 'FONT' resource data hunk. 
Font* Font_New(unsigned char* the_data)
{
	Font*			the_font;
	uint16_t		image_table_count;
	uint16_t		loc_table_count;
	uint16_t		width_table_count;
	uint16_t		height_table_count;
	bool			has_height_table;
	
	// LOGIC:
	//   the font data will contain 26 bytes of font record, followed by data
	//   the data is: 
	//     font bits. size is 2 * rowWords * fRectHeight
	//     location table. size is 2 * (lastChar-firstChar+3)
	//     glyph offset/width table. size is 2 * (lastChar-firstChar+3)
	//
	//   process is allocate space for font rec, copy over its 20 bytes
	//   use the data now in the font rec to allocate space for the other 3 tables and copy them over. 
	
	if ( (the_font = (Font*)calloc(1, sizeof(Font)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new font record", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_font	%p	size	%i", __func__ , __LINE__, the_font, sizeof(Font)));

	// copy in the font record
	memcpy(the_font, the_data, FONT_RECORD_SIZE);
	the_data += FONT_RECORD_SIZE;
	
	// set up font data tables
	
	// LOGIC: 
	//   Mac fonts will have 3 tables immediately after the rowWords table: bitmap image table, bitmap offset table, and width/offset table
	//   Mac fonts can optionally have a separate (higher-precision) character width table after the width offset table
	//   Mac fonts can optionally have a separate image height table after the width offset table (or height table if present)
	//   Both tables are presumed to be same size as the width offset table
	//   We have no use for the higher precision character width table, but the optional height table, if present, can speed up drawing
	//   Flags for both these are stored in bits 0 and 1 of the font type field. 
	

	// font image table -- holds the pixels for each glyph, packed into 16 bit ints

	image_table_count = the_font->rowWords * the_font->fRectHeight;
	
	if ( (the_font->image_table_ = (uint16_t*)calloc(image_table_count, sizeof(uint16_t)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new font image data", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_font->image_table_	%p	size	%i", __func__ , __LINE__, the_font->image_table_, image_table_count * sizeof(uint16_t)));

	memcpy((char*)the_font->image_table_, (char*)the_data, image_table_count * sizeof(uint16_t));
	the_data += image_table_count * sizeof(uint16_t);

	
	// location table -- holds offsets to each glyph, in BITS, from the start of the image table

	loc_table_count = the_font->lastChar - the_font->firstChar + 3;

	if ( (the_font->loc_table_ = (uint16_t*)calloc(loc_table_count, sizeof(uint16_t)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new font location data", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_font->loc_table_	%p	size	%i", __func__ , __LINE__, the_font->loc_table_, loc_table_count * sizeof(uint16_t)));

	memcpy((char*)the_font->loc_table_, (char*)the_data, loc_table_count * sizeof(uint16_t));
	the_data += loc_table_count * sizeof(uint16_t);

	
	// width/offset table -- holds the horizontal offset to first pixel and total render width
	//   the low byte will be the h offset (eg, 1, if you need to start drawing 1 pixel to the right of the pen location)
	//   the high byte will contain the total width needed to render the character (including any whitespace to left or right of pixels)

	width_table_count = the_font->lastChar - the_font->firstChar + 3;
	
	if ( (the_font->width_table_ = (uint16_t*)calloc(width_table_count, sizeof(uint16_t)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new font width data", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_font->width_table_	%p	size	%i", __func__ , __LINE__, the_font->width_table_, width_table_count * sizeof(uint16_t)));
	
	memcpy((char*)the_font->width_table_, (char*)the_data, width_table_count * sizeof(uint16_t));
	the_data += width_table_count * sizeof(uint16_t);


	// get the optional height table, if any - this can be used to optimize drawing speed
	//   the low byte will be the v offset from top of glyph rect (eg, 3, if the first pixel is in the 4th row down)
	//   the high byte will contain the total rows of visible pixels (eg, 3 for say a comma, but 9 for a capital letter)

	has_height_table = (bool)((the_font->fontType >> 0) & 0x01);
// 	DEBUG_OUT(("%s %d: has_height_table=%u, has_detailed_width_table=%u", __func__, __LINE__, has_height_table, (bool)((the_font->fontType >> 1) & 0x01)));	
		
	if (has_height_table)
	{
		//DEBUG_OUT(("%s %d: this font has the optional height table", __func__, __LINE__));	
		
		height_table_count = the_font->lastChar - the_font->firstChar + 3;
		
		if ( (the_font->height_table_ = (uint16_t*)calloc(height_table_count, sizeof(uint16_t)) ) == NULL)
		{
			LOG_ERR(("%s %d: could not allocate memory to create new font height data", __func__ , __LINE__));
			goto error;
		}
		LOG_ALLOC(("%s %d:	__ALLOC__	the_font->height_table_	%p	size	%i", __func__ , __LINE__, the_font->height_table_, height_table_count * sizeof(uint16_t)));

		memcpy((char*)the_font->height_table_, (char*)the_data, height_table_count * sizeof(uint16_t));
	}
	else
	{
		//DEBUG_OUT(("%s %d: this font does NOT have the optional height table", __func__, __LINE__));
		
		height_table_count = 0;
		the_font->height_table_ = NULL;
	}

	//DEBUG_OUT(("%s %d: image_table_count=%u, loc_table_count=%u, width_table_count=%u", __func__, __LINE__, image_table_count, loc_table_count, width_table_count));
	
	// DEBUG
 	//Font_Print(the_font);
//	General_PrintBufferCharacters((char*)the_font->image_table_, image_table_count * sizeof(uint16_t));
// 	General_PrintBufferCharacters((char*)the_font->loc_table_, loc_table_count * sizeof(uint16_t));
// 	General_PrintBufferCharacters((char*)the_font->width_table_, width_table_count * sizeof(uint16_t));
		
	return the_font;
	
error:
	if (the_font)					Font_Destroy(&the_font);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Font_Destroy(Font** the_font)
{
	if (*the_font == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	if ((*the_font)->image_table_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	(*the_font)->image_table_	%p	size	?", __func__ , __LINE__, (*the_font)->image_table_));
		free((*the_font)->image_table_);
	}
	
	if ((*the_font)->loc_table_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	(*the_font)->loc_table_	%p	size	?", __func__ , __LINE__, (*the_font)->loc_table_));
		free((*the_font)->loc_table_);
	}
	
	if ((*the_font)->width_table_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	(*the_font)->width_table_	%p	size	?", __func__ , __LINE__, (*the_font)->width_table_));
		free((*the_font)->width_table_);
	}
	
	if ((*the_font)->height_table_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	(*the_font)->height_table_	%p	size	?", __func__ , __LINE__, (*the_font)->height_table_));
		free((*the_font)->height_table_);
	}

	LOG_ALLOC(("%s %d:	__FREE__	*the_font	%p	size	%i", __func__ , __LINE__, *the_font, sizeof(Font)));
	free(*the_font);
	*the_font = NULL;
	
	return true;
}





// **** xxx functions *****

//! Load a font into memory from disk, and create a font record from it
//! NOTE: this allocates new memory for the font, and copies the font data to it from the passed buffer. It is not dependent on the data in the buffer after returning.
//! This preliminary version is just a shell that does not read from disk, because no disk functionality available yet in f68/mcp as far as I know. Will switch to taking a path or something once disk is available. 
Font* Font_LoadFontData(unsigned char* the_data)
{
	Font*			the_font;
// 	uint16_t		image_table_count;
// 	uint16_t		loc_table_count;
// 	uint16_t		width_table_count;
// 	uint16_t		height_table_count;
// 	bool			has_height_table;
	
	if ( (the_font = Font_New(the_data)) == NULL)
	{
		LOG_ERR(("%s %d: Could not create a Font object", __func__ , __LINE__));
		return NULL;
	}
	
	return the_font;

}



// **** Set xxx functions *****





// **** Get xxx functions *****





// **** Draw string functions *****


// Draw a string at the current "pen" location, using the current pen color of the bitmap
// Truncate, but still draw the string if it is too long to display on the line it started.
// No word wrap is performed. 
// If max_chars is less than the string length, only that many characters will be drawn (as space allows)
// If max_chars is -1, then the full string length will be drawn, as space allows.
bool Font_DrawString(Bitmap* the_bitmap, char* the_string, int16_t max_chars)
{
	int16_t			fit_count;
	int16_t			i;
	int16_t			num_chars;
	int16_t			available_width;
	int16_t			draw_result = 0;
	int16_t			pixels_used;
	
	// LOGIC:
	//   Determine how many characters of the string will fit in one line on the bitmap and draw that many
	
	num_chars = strlen(the_string);
	
	if (num_chars > max_chars && max_chars != -1)
	{
		num_chars = max_chars;
	}
	
	available_width = the_bitmap->width_ - the_bitmap->x_;
	
	//DEBUG_OUT(("%s %d: the_bitmap->width_=%i, the_bitmap->x_=%i, max_chars=%i, num_chars=%i", __func__, __LINE__, the_bitmap->width_, the_bitmap->x_, max_chars, num_chars));
	//DEBUG_OUT(("%s %d: the_string='%s'", __func__, __LINE__, the_string));
	
	fit_count = Font_MeasureStringWidth(the_bitmap->font_, the_string, num_chars, available_width, 0, &pixels_used);
	
	if (fit_count == -1)
	{
		LOG_ERR(("%s %d: Could not measure string width", __func__, __LINE__));
		return false;
	}
	
	for (i = 0; i < fit_count && draw_result != -1; i++)
	{
		unsigned char	the_char;
		
		the_char = the_string[i];
		draw_result = Font_DrawChar(the_bitmap, the_char, NULL);
	}

	return true;
}


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
char* Font_DrawStringInBox(Bitmap* the_bitmap, int16_t width, int16_t height, char* the_string, int16_t num_chars, char** wrap_buffer, bool (* continue_function)(void))
{
	Font*			the_font;
	char*			needs_formatting;
	char*			needed_formatting_last_round;
	char*			formatted_string;
	int16_t			this_line_len;
	bool			do_another_round = false;
	int16_t			row_height;
	int16_t			fixed_char_width;	
	int16_t			x;
	int16_t			y;
	
	if (the_bitmap == NULL)
	{
		LOG_ERR(("%s %d: passed bitmap was NULL", __func__, __LINE__));
		return NULL;
	}

	x = Bitmap_GetX(the_bitmap);
	y = Bitmap_GetY(the_bitmap);
	
	//DEBUG_OUT(("%s %d: x=%i, y=%i, num_chars=%i", __func__, __LINE__, x, y, num_chars));
	
	if (width < 1 || height < 1)
	{
		LOG_ERR(("%s %d: illegal width or height (%i, %i)", __func__, __LINE__, width, height));
		return NULL;
	}
	
	if (x + width > the_bitmap->width_ || y + height > the_bitmap->height_)
	{
		LOG_ERR(("%s %d: illegal box size (%i, %i, %i, %i)", __func__, __LINE__, x, y, width, height));
		return NULL;
	}
	
	// num_chars will be GEN_NO_STRLEN_CAP (-1) if the calling method wants us to display the entire string. 
	if (num_chars == GEN_NO_STRLEN_CAP)
	{
		num_chars = General_Strnlen(the_string, WORD_WRAP_MAX_LEN);
	}
	
	formatted_string = *wrap_buffer;
	needs_formatting = the_string;
	needed_formatting_last_round = needs_formatting;
	the_font = the_bitmap->font_;
	row_height = Font_GetRowHeight(the_font);
	fixed_char_width = Font_GetFixedWidth(the_font);
// 	DEBUG_OUT(("%s %d: row_height=%i, fixed_char_width=%i", __func__, __LINE__, row_height, fixed_char_width));
	
	// outer loop: iterate on one-box worth of text until all text is displayed, or calling function no longer wants to proceed
	do
	{	
		char*			remaining_string;
		int16_t			remaining_len;
		int16_t			orig_len;
		int16_t			v_pixels;
		int16_t			num_rows;
		int16_t			the_row;

		remaining_len = General_Strnlen(needs_formatting, num_chars + 1); // +1 for terminator. 
		orig_len = remaining_len;

		// clear out the word wrap buffer in case anything had been there before. Shouldn't be necessary, but something weird happening in some cases with 2nd+ wrap, and this does prevent it.
		memset(formatted_string, 0, num_chars + 1);
		
		// format the string into chunks that will fit in the width specified, with line breaks on each line
		v_pixels = General_WrapAndTrimTextToFit(&needs_formatting, &formatted_string, orig_len, width, height, fixed_char_width, row_height, the_font, &Font_MeasureStringWidth);
		num_rows = v_pixels / row_height;
		
		// LOGIC:
		//   needs_formatting is now either pointing at next char after cutoff (it not all text fit), or at itself still (if all text fit). 
		//   we can detect if all chars fit by comparing needs_formatting to needed_formatting_last_round
	
		//DEBUG_OUT(("%s %d: v_pixels=%i, num_rows=%i, remaining_len=%i, needs_formatting=%p, the_string=%p", __func__, __LINE__, v_pixels, num_rows, remaining_len, needs_formatting, the_string));
		
		remaining_string = formatted_string;
	
		// clear the target box area on the screen -- if fail to do this, when we draw page 2, etc, it will be messy.
		//Text_FillBox(the_screen, x1, y1, x2, y2, ' ', fore_color, back_color);

		// set up char and attribute memory initial loc
// 		the_char_loc = Text_GetMemLocForXY(the_screen, x1, y1, SCREEN_FOR_TEXT_CHAR);
// 		the_attr_loc = the_char_loc + (the_screen->text_attr_ram_ - the_screen->text_ram_);

		// draw the string, one line at a time, until string is done or no more lines available
	
		the_row = 0;
	
		do
		{
			int16_t		this_write_len;
		
			this_line_len = General_StrFindNextLineBreak(remaining_string, orig_len); // we don't know how many chars could fit on a line so have to just use remaining string len

			if (this_line_len == 1)
			{
				// next/first character is a line break char
				this_write_len = 0;
// 				*(the_attr_loc) = the_attribute_value;
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
		
// 				memcpy(the_char_loc, remaining_string, this_write_len);
// 				memset(the_attr_loc, the_attribute_value, this_write_len);
				Font_DrawString(the_bitmap, remaining_string, this_write_len);
			}

			remaining_string += this_write_len + 1; // skip past the actual \n char.
			remaining_len -= (this_write_len + 1);
			//DEBUG_OUT(("%s %d: remaining_len=%i, remaining='%s'", __func__ , __LINE__, remaining_len, remaining_string));
			
			y += row_height;
			Bitmap_SetXY(the_bitmap, x, y);
// 			the_char_loc += the_screen->text_mem_cols_;
// 			the_attr_loc += the_screen->text_mem_cols_;		
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
//! The current font of the bitmap will be used as the basis for calculating fit.
//! @param	the_font: reference to a complete, loaded Font object.
//! @param	the_string: the null-terminated string to be measured.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed. Passing GEN_NO_STRLEN_CAP will mean it will attempt to measure the entire string.
//! @param	available_width: the width, in pixels, of the space the string is to be measured against.
//! @param	fixed_char_width: the width, in pixels, of one character. This value will be ignored. It exists to keep text-mode text-wrapping compatible with bitmap-font text-wrapping.
//! @param	measured_width: the number of pixels needed to display the characters that fit into the available_width. If the entire string fit, this is the width in pixels of that string. If only X characters fit, it is the pixel width of those X characters.
//! @return	returns -1 in any error condition, or the number of characters that fit. If the entire string fits, the passed len will be returned.
int16_t Font_MeasureStringWidth(Font* the_font, char* the_string, int16_t num_chars, int16_t available_width, int16_t fixed_char_width, int16_t* measured_width)
{
	int16_t			required_width = 0;
	int16_t			i;
	uint8_t			this_width;
		
	if (the_font == NULL)
	{
		LOG_ERR(("%s %d: passed font was NULL", __func__, __LINE__));
		return -1;
	}

	//DEBUG_OUT(("%s %d: num_chars=%i, available_width=%i, str='%s'", __func__, __LINE__, num_chars, available_width, the_string));

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
	//   The Font object contains a table with the total width (including white space) of every character in the font
	//   The Font object also contains a height pixel count and a leading pixel count. Combination of those is required V space per line.
	//	 The Mac fonts are in macRoman encoding, so a future translation process might be necessary. 
	//   Each glyph needs to be examined individually to get the width. 
	
	for (i=0; i < num_chars && required_width <= available_width; i++)
	{
		unsigned char	the_char;
		
		the_char = the_string[i];
		this_width = Font_GetCharWidth(the_font, the_char);
		required_width += this_width;
		//DEBUG_OUT(("%s %d: the_char=%u, this_width=%u, required_width=%i, available_width=%i, i=%i, num_chars=%i", __func__, __LINE__, the_char, this_width, required_width, available_width, i, num_chars));
	}
	
	if (required_width <= available_width)
	{
		*measured_width = required_width;
		//DEBUG_OUT(("%s %d: required_width=%i, measured_width=%i, available_width=%i, i=%i, num_chars=%i", __func__, __LINE__, required_width, *measured_width, available_width, i, num_chars));
		return num_chars;
	}
	else
	{
		*measured_width = required_width - this_width; // take back that last measurement, as it went over the limit
		//DEBUG_OUT(("%s %d: required_width=%i, measured_width=%i, available_width=%i, i=%i, num_chars=%i", __func__, __LINE__, required_width, *measured_width, available_width, i, num_chars));
		return i - 1;
	}
}


//! Draw one character on the bitmap, at the current bitmap pen coordinates
//! NOTE: if the draw action is successful, the bitmaps current pen position will be updated in preparation for the next character draw.
//! TODO: stop passing Font, and have the concept of a current font for a given bitmap. and maybe a default system font. 
//! return Returns number of horizontal pixels used, including left/right offsets, or -1 on any error condition.
int16_t Font_DrawChar(Bitmap* the_bitmap, unsigned char the_char, Font* the_font)
{
	uint8_t			next_char;
	int16_t			loc_offset;
	int16_t			next_loc_offset;
	int16_t			pixel_only_width;		//!< the width of the character's actual pixels at max width
	int16_t			image_offset_index;
	int16_t			image_offset_index_rem;
	int16_t			offset_width_value;
	int8_t			h_offset_value;			//!< the horizontal offset from pen position before the first pixel should be drawn
	int8_t			width_value;			//!< the total width of the character including any whitespace to left/right
	uint16_t*		start_read_addr;
	int32_t			row;
	uint8_t			this_bit;
	uint8_t			the_color;				// shortcut to bitmap->color_
	unsigned char*	start_write_addr;
	int32_t			pixels_moved;
	uint8_t			first_row;				// if no height table available, this is 0. otherwise it's first row to start drawing.
	uint8_t			max_row;				// if no height table available, this is height of font rec. otherwise it's 1 past the last vis row
	
	if (the_bitmap == NULL)
	{
		LOG_ERR(("%s %d: passed bitmap was NULL", __func__, __LINE__));
		return -1;
	}

	// use the passed font record, unless NULL, and in that case, use the bitmap's font record if not NULL
	if (the_font == NULL)
	{
		the_font = the_bitmap->font_;
		
		if (the_bitmap->font_ == NULL)
		{
			LOG_ERR(("%s %d: passed font was NULL and the bitmap's font was also NULL", __func__, __LINE__));
			return -1;
		}
	}
	
	// LOGIC:
	//   Some Mac fonts have an optional height offset/num rows table. 
	//   If present, it will contain row of first visible pixel, and count of rows with pixels
	//   We can use that to reduce reads/write loops and draw faster. 
	//     Characters that are not in the font (missing/-1 glyphs) will have 0 for height/offset. 
	//   If no height table present, we read/write through every row in the Font
	
	if ( ((the_font->fontType >> 0) & 0x01) )
	{
		uint16_t		v_offset_height;

		v_offset_height = the_font->height_table_[the_char];
		
		if (v_offset_height == 0)
		{
			// if char is missing, height value seems to get set to 0. Use the one for the "missing glyph" char, which is one past last real char in font
			v_offset_height = the_font->height_table_[the_font->lastChar + 1];
		}
		
		first_row = v_offset_height >> 8;
		max_row = first_row + v_offset_height & 0xFF;		
	}
	else
	{
		first_row = 0;
		max_row = the_font->fRectHeight;
	}
	
	//DEBUG_OUT(("%s %d: glyph char=%u, height/offset value=%i, first_row=%i, max_row=%i", __func__, __LINE__, the_char, v_offset_height, first_row, max_row));
	
	//   the low byte will be the v offset from top of glyph rect (eg, 3, if the first pixel is in the 4th row down)
	//   the high byte will contain the total rows of visible pixels (eg, 3 for say a comma, but 9 for a capital letter)

	
	// LOGIC:
	//   The pixel-only width (max width of character excluding any whitespace to left or right) is determined by subtracting the location offset of the the char to draw, from that of the following character. All the characters are packed in "shoulder to shoulder, so basically you are just comparing start of this char vs start of next char. 
	
	next_char = the_char + 1;

	// LOGIC:
	//   The offset/width table contains -1 if the char does not exist in this font
	//   If the char does exist:
	//     the low byte will be the horizontal offset (eg, 1, if you need to start drawing 1 pixel to the right of the pen location)if the value is -1
	//     the high byte will contain the total width needed to render the character (including any whitespace to left or right of pixels)
	
	offset_width_value = the_font->width_table_[the_char];
	
	if (offset_width_value == -1)
	{
		//DEBUG_OUT(("%s %d: offset/width table says this char (%u) does not exist in the font", __func__, __LINE__, the_char));
		
		// switch to the "missing glyph" character, which is the last one in the font.
		the_char = the_font->lastChar + 1;
		next_char = the_char+1;
		offset_width_value = the_font->width_table_[the_char];
	}
	else
	{
		//DEBUG_OUT(("%s %d: this char (%u) has a glyph in the font. Width/offset value=%i", __func__, __LINE__, the_char, offset_width_value));		
	}

	h_offset_value = offset_width_value >> 8;
	width_value = offset_width_value & 0xFF;
	
	loc_offset = the_font->loc_table_[the_char];
	next_loc_offset = the_font->loc_table_[next_char];
	pixel_only_width = next_loc_offset - loc_offset;
	
	//DEBUG_OUT(("%s %d: glyph char=%u, pixelwidth=%i, Width/offset value=%i, width_value=%i, h_offset_value=%i", __func__, __LINE__, the_char, pixel_only_width, offset_width_value, width_value, h_offset_value));
				
	// for A in Chicago, I get "227". this is apparently the BIT offset from the start of the image data table. 
	// so 227/16=14.1875=image_table_[13] + 3 bits. 
	
	image_offset_index = loc_offset / 16;
	image_offset_index_rem = loc_offset % 16;
	//DEBUG_OUT(("%s %d: loc_offset=%i, image_offset_index=%i, image_offset_index_rem=%i", __func__, __LINE__, loc_offset, image_offset_index, image_offset_index_rem));

	the_color = Bitmap_GetColor(the_bitmap);
	
	start_read_addr = the_font->image_table_ + image_offset_index;
	
	start_write_addr = Bitmap_GetMemLoc(the_bitmap);
	//DEBUG_OUT(("%s %d: start_write_addr=%p", __func__, __LINE__, start_write_addr));

	for (row = 0; row < the_font->fRectHeight; row++)
	{
		uint16_t*		read_addr;
		unsigned char*	write_addr;
		
		read_addr = start_read_addr;
		write_addr = start_write_addr;
		
		// LOGIC: 
		//   we have one or more 16 bit words to parse
		//   each bit in the word represents one horizontal pixel on or off
		//   a glyph may start on one word, and end on the next
		//   of the 16 bits, we likely only need a subset: 
		//     the bits from image_offset_index_rem to image_offset_index_rem +  pixel_only_width
		
		if (row >= first_row && row < max_row)
		{
			int16_t	pixels_written = 0;

			// for each row, account for any H offset specified for the glyph
			write_addr += h_offset_value;
			
			pixels_moved = 0;

			do
			{
				int16_t	i;
				
				for (i = 15; i >= 0 && pixels_written < pixel_only_width; i--)
				{
					//DEBUG_OUT(("font_data=%u, bit=%u, i=%i, row=%i", *font_data, (unsigned char)((*font_data >> i) & 0x01), i, row));
			
					if (pixels_moved >= image_offset_index_rem)
					{
						this_bit = (unsigned char)((*read_addr >> i) & 0x01);
					
						if (this_bit)
						{
							*write_addr = the_color;
						}
					
						write_addr++;
						pixels_written++;
					}
			
					pixels_moved++;
				}
			
				read_addr++; // move to next word in the font data
			
			} while (pixels_written < pixel_only_width);
		}		
		
		// move read pointer in font to next row; move write pointer in bitmap to next row
		start_read_addr += the_font->rowWords;
		start_write_addr += the_bitmap->width_;
	}
	
	// finished writing visible pixels, but need to move pen further right if char's overall width was greater than amount moved so far
	DEBUG_OUT(("%s %d: before: pixels_moved=%i, width_value=%i", __func__, __LINE__, pixels_moved, width_value));
	//pixels_moved += width_value - pixels_moved;
	pixels_moved = width_value;
// 	DEBUG_OUT(("%s %d: after: pixels_moved=%i", __func__, __LINE__, pixels_moved));
	
	//DEBUG_OUT(("%s %d: before: pixels_moved=%i, the_bitmap->x_=%i", __func__, __LINE__, pixels_moved, the_bitmap->x_));
// 	int16_t	temp = the_bitmap->x_;
	the_bitmap->x_ += pixels_moved;
	//DEBUG_OUT(("%s %d: after: the_bitmap->x_=%i", __func__, __LINE__, the_bitmap->x_));
// 	the_bitmap->x_ = temp;
// 	the_bitmap->x_ += (int16_t)pixels_moved;
// 	DEBUG_OUT(("%s %d: after cast: the_bitmap->x_=%i", __func__, __LINE__, the_bitmap->x_));
	
	return pixels_moved;
}



