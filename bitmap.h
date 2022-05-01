//! @file bitmap.h

/*
 * bitmap.h
 *
*  Created on: Mar 10, 2022
 *      Author: micahbly
 */

#ifndef LIB_BITMAP_H_
#define LIB_BITMAP_H_


/* about this library: Bitmap
 *
 * This provides functionality for accessing the VICKY's bitmaps, sprites, etc.
 *
 *** things this library needs to be able to do
 * initialize/enter graphics mode
 * exit graphics mode
 * fill an entire screen of bitmap
 * fill a rect
 * draw a rect
 * allocate a bitmap
 * copy a bitmap
 * load a bitmap from disk
 * draw a circle
 * fill an enclosed area
 * draw a round rect
 * draw a line
 * get the value of a pixel, from specified x/y coords
 * set the value of a pixel, from specified x/y coords
 * copy a rect of pixel mem from one bitmap to another
 * copy a rect of pixel mem from place to place within the same bitmap
 *
 * STRETCH GOALS
 * load a graphical (proportional width or fixed width) font from disk or memory
 * draw string using graphical font on screen, at specified x/y
 * draw string using graphical font on screen, wrapping and fitting to specified rectangle
 *
 * SUPER STRETCH GOALS
 * have a clipping system that prevents drawing to non-clipped parts of the screen
 * have a layers system that prevents drawing to portions of layers that are under other layers
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
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define PARAM_DRAW_NE		true	//!< for Bitmap_DrawCircleQuadrants, do draw NE quadrant
#define PARAM_DRAW_SE		true	//!< for Bitmap_DrawCircleQuadrants, do draw SE quadrant
#define PARAM_DRAW_SW		true	//!< for Bitmap_DrawCircleQuadrants, do draw SW quadrant
#define PARAM_DRAW_NW		true	//!< for Bitmap_DrawCircleQuadrants, do draw NW quadrant
#define PARAM_SKIP_NE		false	//!< for Bitmap_DrawCircleQuadrants, do NOT draw NE quadrant
#define PARAM_SKIP_SE		false	//!< for Bitmap_DrawCircleQuadrants, do NOT draw SE quadrant
#define PARAM_SKIP_SW		false	//!< for Bitmap_DrawCircleQuadrants, do NOT draw SW quadrant
#define PARAM_SKIP_NW		false	//!< for Bitmap_DrawCircleQuadrants, do NOT draw NW quadrant

#define PARAM_DO_FILL		true	//!< for various graphic routines
#define PARAM_DO_NOT_FILL	false	//!< for various graphic routines

#define PARAM_IN_VRAM		true	//!< for Bitmap_New
#define PARAM_NOT_IN_VRAM	false	//!< for Bitmap_New


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

struct Bitmap
{
	int16_t			width_;		//!< width of the bitmap in pixels
	int16_t			height_;	//!< height of the bitmap in pixels
	int16_t			x_;			//!< H position within this bitmap, of the "pen", for functions that draw from that point
	int16_t			y_;			//!< V position within this bitmap, of the "pen", for functions that draw from that point
	uint8_t			color_;		//!< color value to use for next "pen" based operation in this bitmap
	uint8_t			reserved_;	//!< future use
	Font*			font_;		//!< the currently selected font. All text drawing activities will use this font face.
	unsigned char*	addr_;		//!< address of the start of the bitmap, within the machine's global address space. This is not the VICKY's local address for this bitmap. This address MUST be within the VRAM, however, it cannot be in non-VRAM memory space.
	bool			in_vram_;	//!< a way to know if this bitmap is pointing to VRAM or standard RAM space.
};


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor

//! Create a new bitmap object by allocating space for the bitmap struct in regular memory, and for the graphics, in VRAM
//! NOTE: when creating a bitmap to represent something actually in VRAM, pass true to in_vram, and manually assign a known VRAM location afterwards.
//! @param	width: width, in pixels, of the bitmap to be created
//! @param	height: height, in pixels, of the bitmap to be created
//! @param	the_font: optional font object to associate with the Bitmap. 
//! @param	in_vram: if true, no space will be allocated for the bitmap graphics. If false, width * height area of memory will be allocated in standard memory.
Bitmap* Bitmap_New(int16_t width, int16_t height, Font* the_font, bool in_vram);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Bitmap_Destroy(Bitmap** the_bitmap);



// **** Block copy functions ****

//! Blit a rect from source bitmap to distination bitmap
//! This is a wrapper around Bitmap_Blit()
//! The source and destination bitmaps can be the same: you can use this to copy a chunk of pixels from one part of a screen to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param dst_bm: the destination bitmap. It must have a valid address within the VRAM memory space. It can be the same bitmap as the source.
//! @param src_rect: the rectangle from the source bitmap to be blitted to the target bitmap
//! @param dst_x, dst_y: the location within the destination bitmap to copy pixels to. May be negative.
bool Bitmap_BlitRect(Bitmap* src_bm, Rectangle src_rect, Bitmap* dst_bm, int16_t dst_x, int16_t dst_y);

//! Blit from source bitmap to distination bitmap. 
//! The source and destination bitmaps can be the same: you can use this to copy a chunk of pixels from one part of a screen to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param dst_bm: the destination bitmap. It must have a valid address within the VRAM memory space. It can be the same bitmap as the source.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the rectangle you want to copy. May be negative.
//! @param dst_x, dst_y: the location within the destination bitmap to copy pixels to. May be negative.
//! @param width, height: the scope of the copy, in pixels.
bool Bitmap_Blit(Bitmap* src_bm, int16_t src_x, int16_t src_y, Bitmap* dst_bm, int16_t dst_x, int16_t dst_y, int16_t width, int16_t height);

//! Tile the source bitmap into the destination bitmap, filling it
//! The source and destination bitmaps can be the same: you can use this to copy a chunk of pixels from one part of a screen to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param dst_bm: the destination bitmap. It must have a valid address within the VRAM memory space. It can be the same bitmap as the source.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the tile you want to copy. Must be non-negative.
//! @param width, height: the size of the tile to be derived from the source bitmap, in pixels.
bool Bitmap_Tile(Bitmap* src_bm, int16_t src_x, int16_t src_y, Bitmap* dst_bm, int16_t width, int16_t height);

//! Tile the source bitmap into the destination bitmap, filling it
//! The source and destination bitmaps can be the same: you can use this to copy a chunk of pixels from one part of a screen to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param dst_bm: the destination bitmap. It must have a valid address within the VRAM memory space. It can be the same bitmap as the source.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the tile you want to copy. Must be non-negative.
//! @param width, height: the size of the tile to be derived from the source bitmap, in pixels.
bool Bitmap_TileV1(Bitmap* src_bm, int16_t src_x, int16_t src_y, Bitmap* dst_bm, int16_t width, int16_t height);

//! Tile the source bitmap into the destination bitmap, filling it
//! The source and destination bitmaps can be the same: you can use this to copy a chunk of pixels from one part of a screen to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param dst_bm: the destination bitmap. It must have a valid address within the VRAM memory space. It can be the same bitmap as the source.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the tile you want to copy. Must be non-negative.
//! @param width, height: the size of the tile to be derived from the source bitmap, in pixels.
bool Bitmap_TileV2(Bitmap* src_bm, int16_t src_x, int16_t src_y, Bitmap* dst_bm, int16_t width, int16_t height);



// **** Block fill functions ****

// Fill graphics memory with specified value
// calling function must validate the screen ID before passing!
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_FillMemory(Bitmap* the_bitmap, uint8_t the_color);

//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_FillBoxRect(Bitmap* the_bitmap, Rectangle* the_coords, uint8_t the_color);

// Fill pixel values for a specific box area, using the specified LUT value
// calling function must validate screen id, coords!
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_FillBox(Bitmap* the_bitmap, int16_t x, int16_t y, int16_t width, int16_t height, uint8_t the_color);




// **** Bitmap functions *****

//! Set the font
//! This is the font that will be used for all font drawing in this bitmap
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	the_font: reference to a complete, loaded Font object.
//! @return Returns false on any error condition
bool Bitmap_SetFont(Bitmap* the_bitmap, Font* the_font);

//! Set the "pen" color
//! This is the color that the next pen-based graphics function will use
//! This only affects functions that use the pen: any graphics function that specifies a color will use that instead
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return Returns false on any error condition
bool Bitmap_SetColor(Bitmap* the_bitmap, uint8_t the_color);

//! Set the "pen" position
//! This is the location that the next pen-based graphics function will use for a starting location
//! NOTE: you are allowed to set negative values, but not values greater than the height/width of the screen. This is to allow for functions that may have portions visible on the screen, such as a row of text that starts 2 pixels to the left of the bitmap's left edge. 
//! This only affects functions that use the pen: any graphics function that specifies an X, Y coordinate will use that instead
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	x: the horizontal position, between 0 and bitmap width - 1
//! @param	y: the vertical position, between 0 and bitmap height - 1
//! @return Returns false on any error condition
bool Bitmap_SetXY(Bitmap* the_bitmap, int16_t x, int16_t y);

//! Get the current color of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns a 1-byte index to the current LUT, or 0 on any error
uint8_t Bitmap_GetColor(Bitmap* the_bitmap);

//! Get the current X position of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns -1 on any error
int16_t Bitmap_GetX(Bitmap* the_bitmap);

//! Get the current Y position of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns -1 on any error
int16_t Bitmap_GetY(Bitmap* the_bitmap);

//! Calculate the VRAM location of the current coordinate within the bitmap
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns a pointer to the VRAM location that corresponds to the current "pen" X, Y, or NULL on any error condition
unsigned char* Bitmap_GetMemLoc(Bitmap* the_bitmap);

//! Get the current font of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns NULL on any error
Font* Bitmap_GetFont(Bitmap* the_bitmap);



// **** Set pixel functions *****


//! Set a char at a specified x, y coord
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_SetPixelAtXY(Bitmap* the_bitmap, int16_t x, int16_t y, uint8_t the_color);



// **** Get pixel functions *****


//! Get the pixel CLUT index at a specified x, y coord
//! @return	returns an 8-bit CLUT index
uint8_t Bitmap_GetPixelAtXY(Bitmap* the_bitmap, int16_t x, int16_t y);



// **** Drawing functions *****


//! Draws a line between 2 passed coordinates.
//! Use for any line that is not perfectly vertical or perfectly horizontal
//! Based on http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C. Used in C128 Lich King. 
bool Bitmap_DrawLine(Bitmap* the_bitmap, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color);

//! Draws a horizontal line from specified coords, for n pixels, using the specified pixel value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_DrawHLine(Bitmap* the_bitmap, int16_t x, int16_t y, int16_t the_line_len, uint8_t the_color);

//! Draws a vertical line from specified coords, for n pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_DrawVLine(Bitmap* the_bitmap, int16_t x, int16_t y, int16_t the_line_len, uint8_t the_color);

//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_DrawBoxRect(Bitmap* the_bitmap, Rectangle* the_coords, uint8_t the_color);

//! Draws a rectangle based on 2 sets of coords, using the specified LUT value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Bitmap_DrawBoxCoords(Bitmap* the_bitmap, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color);

//! Draws a rectangle based on start coords and width/height, and optionally fills the rectangle.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Bitmap_DrawBox(Bitmap* the_bitmap, int16_t x, int16_t y, int16_t width, int16_t height, uint8_t the_color, bool do_fill);

//! Draws a rounded rectangle with the specified size and radius, and optionally fills the rectangle.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Bitmap_DrawRoundBox(Bitmap* the_bitmap, int16_t x, int16_t y, int16_t width, int16_t height, int16_t radius, uint8_t the_color, bool do_fill);

//! Draw a circle
//! Based on http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
bool Bitmap_DrawCircle(Bitmap* the_bitmap, int16_t x1, int16_t y1, int16_t radius, uint8_t the_color);





#endif /* LIB_BITMAP_H_ */


