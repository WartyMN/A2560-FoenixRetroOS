//! @file lib_graphics.h

/*
 * lib_graphics.h
 *
*  Created on: Mar 10, 2022
 *      Author: micahbly
 */

#ifndef LIB_GRAPHICS_H_
#define LIB_GRAPHICS_H_


/* about this library: Graphics
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

// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/lib_text.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define PARAM_DRAW_NE		true	//!< for Graphics_DrawCircleQuadrants, do draw NE quadrant
#define PARAM_DRAW_SE		true	//!< for Graphics_DrawCircleQuadrants, do draw SE quadrant
#define PARAM_DRAW_SW		true	//!< for Graphics_DrawCircleQuadrants, do draw SW quadrant
#define PARAM_DRAW_NW		true	//!< for Graphics_DrawCircleQuadrants, do draw NW quadrant
#define PARAM_SKIP_NE		false	//!< for Graphics_DrawCircleQuadrants, do NOT draw NE quadrant
#define PARAM_SKIP_SE		false	//!< for Graphics_DrawCircleQuadrants, do NOT draw SE quadrant
#define PARAM_SKIP_SW		false	//!< for Graphics_DrawCircleQuadrants, do NOT draw SW quadrant
#define PARAM_SKIP_NW		false	//!< for Graphics_DrawCircleQuadrants, do NOT draw NW quadrant

#define PARAM_DO_FILL		true	//!< for various graphic routines
#define PARAM_DO_NOT_FILL	false	//!< for various graphic routines

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

struct Bitmap
{
	signed int		width_;		//!< width of the bitmap in pixels
	signed int		height_;	//!< height of the bitmap in pixels
	signed int		x_;			//!< H position within this bitmap, of the "pen", for functions that draw from that point
	signed int		y_;			//!< V position within this bitmap, of the "pen", for functions that draw from that point
	uint8_t			color_;		//!< color value to use for next "pen" based operation in this bitmap
	uint8_t			reserved_;	//!< future use
	Font*			font_;		//!< the currently selected font. All text drawing activities will use this font face.
	unsigned char*	addr_;		//!< address of the start of the bitmap, within the machine's global address space. This is not the VICKY's local address for this bitmap. This address MUST be within the VRAM, however, it cannot be in non-VRAM memory space.
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
//! @param	Font: optional font object to associate with the Bitmap. 
Bitmap* Bitmap_New(signed int width, signed int height, Font* the_font);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
boolean Bitmap_Destroy(Bitmap** the_bitmap);


// **** Block copy functions ****

//! Blit from source bitmap to distination bitmap. 
//! The source and destination bitmaps can be the same: you can use this to copy a chunk of pixels from one part of a screen to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param dst_bm: the destination bitmap. It must have a valid address within the VRAM memory space. It can be the same bitmap as the source.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the rectangle you want to copy. May be negative.
//! @param dst_x, dst_y: the location within the destination bitmap to copy pixels to. May be negative.
//! @param width, height: the scope of the copy, in pixels.
boolean Graphics_BlitBitMap(Bitmap* src_bm, int src_x, int src_y, Bitmap* dst_bm, int dst_x, int dst_y, int width, int height);


// **** Block fill functions ****

// Fill graphics memory with specified value
// calling function must validate the screen ID before passing!
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_FillMemory(Bitmap* the_bitmap, unsigned char the_color);

//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_FillBoxRect(Bitmap* the_bitmap, Rectangle* the_coords, unsigned char the_color);

// Fill pixel values for a specific box area, using the specified LUT value
// calling function must validate screen id, coords!
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_FillBox(Bitmap* the_bitmap, signed int x, signed int y, signed int width, signed int height, unsigned char the_color);




// **** Bitmap functions *****

//! Set the font
//! This is the font that will be used for all font drawing in this bitmap
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	the_font: reference to a complete, loaded Font object.
//! @return Returns false on any error condition
boolean Bitmap_SetCurrentFont(Bitmap* the_bitmap, Font* the_font);

//! Set the "pen" color
//! This is the color that the next pen-based graphics function will use
//! This only affects functions that use the pen: any graphics function that specifies a color will use that instead
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return Returns false on any error condition
boolean Bitmap_SetCurrentColor(Bitmap* the_bitmap, uint8_t the_color);

//! Set the "pen" position
//! This is the location that the next pen-based graphics function will use for a starting location
//! NOTE: you are allowed to set negative values, but not values greater than the height/width of the screen. This is to allow for functions that may have portions visible on the screen, such as a row of text that starts 2 pixels to the left of the bitmap's left edge. 
//! This only affects functions that use the pen: any graphics function that specifies an X, Y coordinate will use that instead
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	x: the horizontal position, between 0 and bitmap width - 1
//! @param	y: the vertical position, between 0 and bitmap height - 1
//! @return Returns false on any error condition
boolean Bitmap_SetCurrentXY(Bitmap* the_bitmap, signed int x, signed int y);

//! Get the current color of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns a 1-byte index to the current LUT, or 0 on any error
uint8_t Bitmap_GetCurrentColor(Bitmap* the_bitmap);

//! Get the current X position of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns -1 on any error
signed int Bitmap_GetCurrentX(Bitmap* the_bitmap);

//! Get the current Y position of the pen
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns -1 on any error
signed int Bitmap_GetCurrentY(Bitmap* the_bitmap);

//! Calculate the VRAM location of the specified coordinate within the bitmap
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @param	x: the horizontal position, between 0 and bitmap width - 1
//! @param	y: the vertical position, between 0 and bitmap height - 1
//! @return Returns a pointer to the VRAM location that corresponds to the passed X, Y, or NULL on any error condition
unsigned char* Bitmap_GetMemLocForXY(Bitmap* the_bitmap, signed int x, signed int y);

//! Calculate the VRAM location of the current coordinate within the bitmap
//! @param	the_bitmap: reference to a valid Bitmap object.
//! @return Returns a pointer to the VRAM location that corresponds to the current "pen" X, Y, or NULL on any error condition
unsigned char* Bitmap_GetCurrentMemLoc(Bitmap* the_bitmap);




// **** Set pixel functions *****


//! Set a char at a specified x, y coord
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_SetPixelAtXY(Bitmap* the_bitmap, signed int x, signed int y, unsigned char the_color);



// **** Get pixel functions *****


//! Get the char at a specified x, y coord
//! @return	returns a character code
unsigned char Graphics_GetPixelAtXY(Bitmap* the_bitmap, signed int x, signed int y);



// **** Drawing functions *****


//! Draws a line between 2 passed coordinates.
//! Use for any line that is not perfectly vertical or perfectly horizontal
//! Based on http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C. Used in C128 Lich King. 
boolean Graphics_DrawLine(Bitmap* the_bitmap, signed int x1, signed int y1, signed int x2, signed int y2, unsigned char the_color);

//! Draws a horizontal line from specified coords, for n pixels, using the specified pixel value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_DrawHLine(Bitmap* the_bitmap, signed int x, signed int y, signed int the_line_len, unsigned char the_color);

//! Draws a vertical line from specified coords, for n pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_DrawVLine(Bitmap* the_bitmap, signed int x, signed int y, signed int the_line_len, unsigned char the_color);

//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_DrawBoxRect(Bitmap* the_bitmap, Rectangle* the_coords, unsigned char the_color);

//! Draws a rectangle based on 2 sets of coords, using the specified LUT value
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
boolean Graphics_DrawBoxCoords(Bitmap* the_bitmap, signed int x1, signed int y1, signed int x2, signed int y2, unsigned char the_color);

//! Draws a rectangle based on start coords and width/height, and optionally fills the rectangle.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
boolean Graphics_DrawBox(Bitmap* the_bitmap, signed int x, signed int y, signed int width, signed int height, unsigned char the_color, boolean do_fill);

//! Draws a rounded rectangle with the specified size and radius, and optionally fills the rectangle.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
boolean Graphics_DrawRoundBox(Bitmap* the_bitmap, signed int x, signed int y, signed int width, signed int height, signed int radius, unsigned char the_color, boolean do_fill);

//! Draw a circle
//! Based on http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
boolean Graphics_DrawCircle(Bitmap* the_bitmap, signed int x1, signed int y1, signed int radius, unsigned char the_color);





#endif /* LIB_GRAPHICS_H_ */


