//! @file window.h

/*
 * window.h
 *
*  Created on: Mar 19, 2022
 *      Author: micahbly
 */

#ifndef LIB_WINDOW_H_
#define LIB_WINDOW_H_


/* about this library: Window
 *
 * This provides functionality for working with windows
 * 
 * See also: control.h
 *
 *** things this library needs to be able to do
 * Create a new window instance based on a Theme object and other parameters
 * Render a window on the screen
 * Receive and manage events
 * Identify which control is affected by a mouse click or mouse move event
 * Manage state of all controls on the window
 * Handle resizing/minimizing/maximizing of window
 * Reposition controls on window in response to resize events
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
#include "control.h"

// C includes
#include <stdbool.h>

// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
#include <mb/bitmap.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define WIN_DEFAULT_WIDTH			512
#define WIN_DEFAULT_HEIGHT			342
#define WIN_DEFAULT_MIN_WIDTH		90
#define WIN_DEFAULT_MIN_HEIGHT		60
#define WIN_DEFAULT_MAX_WIDTH		800
#define WIN_DEFAULT_MAX_HEIGHT		600

#define WIN_DEFAULT_DRAG_ZONE_SIZE		8		//! The width of left/right drag zones, starting from edge of window, or the height of top/bottom drag zones

#define WIN_DEFAULT_X	50
#define WIN_DEFAULT_Y	25
#define WIN_STAGGER_X	14	// when multiple windows are opened, how far apart are they positioned?
#define WIN_STAGGER_Y	14

#define WINDOW_MAX_WINTITLE_SIZE		128

#define WIN_OPEN_AS_BACKDROP			true	// Window_New() parameter
#define WIN_DO_NOT_OPEN_AS_BACKDROP		false	// Window_New() parameter


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum window_type
{
	WIN_STANDARD	= 0,
	WIN_BACKDROP 	= 1,
	WIN_DIALOG		= 2,
	WIN_UNKNOWN_TYPE,
} window_type;

typedef enum window_state
{
	WIN_HIDDEN			= 0,
	WIN_MINIMIZED 		= 1,
	WIN_NORMAL			= 2,
	WIN_MAXIMIZED		= 3,
	WIN_UNKNOWN_STATE,
} window_state;

typedef enum window_base_control_id
{
	CLOSE_WIDGET_ID		= 0,
	MINIMIZE_WIDGET_ID 	= 1,
	NORM_SIZE_WIDGET_ID	= 2,
	MAXIMIZE_WIDGET_ID	= 3,
} window_base_control_id;

/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

struct Window
{
	uint8_t					id_;							// reserved. Not currently used.
	int8_t					display_order_;					// 0 = active window, ascending order from there. maintained by system. 
	uint32_t				user_data_;						// 32 bits for use of programs. The system will not process this field. 
	char*					title_;
	window_type				type_;
	window_state			state_;							// read-only: whether the window is currently hidden, minimized, normal/window sized, or maximized
	Bitmap* 				bitmap_;						// on-screen bitmap covering the visible portion of window
	Bitmap* 				buffer_bitmap_;					// off-screen bitmap covering the visible portion of window
// 	struct Region*			global_region_;
// 	struct Region*			content_region_;				// portion of window that will contain content. excludes the borders and title bar
// 	struct Region*			iconbar_region_;				// region for the iconbar, if any
// 	struct Region*			titlebar_region_;
	Rectangle				overall_rect_;					// the rect describing the total area of the window
	Rectangle				content_rect_;					// the rect describing the content area of the window
	signed int				pen_x_;							// H position relative to the content_rect_, of the "pen", for drawing functions
	signed int				pen_y_;							// V position relative to the content_rect_, of the "pen", for drawing functions
	uint8_t					pen_color_;						// Color index of the "pen", for drawing functions
	Font*					pen_font_;						// Font to be used by the "pen", for drawing functions
	Rectangle				titlebar_rect_;					// the rect describing the titlebar area
	Rectangle				grow_left_rect_;				// the rect defining the area in which a click/drag will resize window
	Rectangle				grow_right_rect_;				// the rect defining the area in which a click/drag will resize window
	Rectangle				grow_top_rect_;					// the rect defining the area in which a click/drag will resize window
	Rectangle				grow_bottom_rect_;				// the rect defining the area in which a click/drag will resize window
	bool					is_backdrop_;					// true if this is the backdrop (desktop) window
	bool					visible_;						// is the window visible?
	bool					active_;						// keep 1 window as the active one. only active windows get regular updates
	bool					changes_to_save_;				// starts at false; if window is resized or repositioned, is set to true. used to know if we have to save out to icon file when window is closed.
	bool					can_resize_;					// if true, window can be stretched or shrunk. If false, the width_ and height_ will be locked.
	bool					invalidated_;					// if true, the window needs to be completed re-rendered on the next render pass
	signed int				x_;								// horizontal coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				y_;								// vertical coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				width_;							// width of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				height_;						// height of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				min_width_;						// minimum width of window when in window-sized (normal) mode.
	signed int				min_height_;					// minimum height of window when in window-sized (normal) mode. 
	signed int				max_width_;						// maximum width of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0.
	signed int				max_height_;					// maximum height of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0. 
	signed int				inner_width_;					// space available inside the content area, accounting for border thicknesses
	signed int				inner_height_;					// space available inside the content area, accounting for border thicknesses and title bar
	signed int				content_left_;					// of the raw x pos of the window (non-gzz), the x pos where window should start rendering content. =gzz_left_ until window is scrolled leftwards
	signed int				content_top_;					// of the raw y pos of the window (non-gzz), the y pos where window should start rendering content. =gzz_top_ until window is scrolled down
	signed int				required_inner_width_;			// greater of current inner_width or H space required inside the window to display all content. If greater than H space, a scroller is needed.
	signed int				required_inner_height_;			// greater of current inner_height or V space required inside the window to display all content. If greater than V space, a scroller is needed.
	bool					h_scroller_visible_;
	bool					v_scroller_visible_;
	Bitmap*					pattern_;						// optional pattern used for filling the window content rect background on refresh. 
	Window*					parent_window_;					// can be NULL. used for requesters that are spawned from a specific window.
	Window*					child_window_;					// can be NULL. used when a window spawns a requester. (This is the requester). NULLs out again when requester is closed. 
	Control*				root_control_;					// first control in the window

// 	MouseTracker*			mouse_tracker_;					// tracks mouse up/down points within this window
// 	Window*					zoom_to_window_;				// the window that contains the zoom_to_file, so we can get offset to global screen coords
// 	unsigned short			zoom_x_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	unsigned short			zoom_y_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	unsigned short			zoom_w_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	unsigned short			zoom_h_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	struct Menu*			menu_;
// 	unsigned char*			keyboard_buffer_;		// used by key detection
// 	unsigned short			keyboard_buf_pos_;		// used by key detection
};


struct NewWinTemplate
{
	unsigned long			user_data_;						// 32 bits for use of programs. The system will not process this field. 
	char*					title_;
	window_type				type_;
	Bitmap* 				bitmap_;						// on-screen bitmap covering the visible portion of window
	Bitmap* 				buffer_bitmap_;					// off-screen bitmap covering the visible portion of window
	bool					is_backdrop_;					// true if this is the backdrop (desktop) window
	bool					can_resize_;					// if true, window can be stretched or shrunk. If false, the width_ and height_ will be locked.
	signed int				x_;								// horizontal coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				y_;								// vertical coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				width_;							// width of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				height_;						// height of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				min_width_;						// minimum width of window when in window-sized (normal) mode.
	signed int				min_height_;					// minimum height of window when in window-sized (normal) mode. 
	signed int				max_width_;						// maximum width of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0.
	signed int				max_height_;					// maximum height of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0. 
};



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate a Window object
Window* Window_New(NewWinTemplate* the_win_template);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Window_Destroy(Window** the_window);

//! Allocate and populate a new window template object
//! Ensures that all fields have appropriate default values
//! Calling method must free this after creating a window with it. 
NewWinTemplate* Window_GetNewWinTemplate(char* the_win_title);






// **** Set functions *****


bool Window_SetControlState(Window* the_window, uint16_t the_control_id);

// replace the current window title with the passed string
// Note: the passed string will be copied into storage by the window. The passing function can dispose of the passed string when done.
void Window_SetTitle(Window* the_window, char* the_title);

//! Set the passed window's visibility flag.
//! This does not immediately cause the window to render. The window will be rendered on the next system rendering pass.
void Window_SetVisible(Window* the_window, bool is_visible);


// **** Get functions *****

Control* Window_GetRootControl(Window* the_window);
Control* Window_GetControl(Window* the_window, uint16_t the_control_id);
uint16_t Window_GetControlID(Window* the_window, Control* the_control);

// return the current window title
// Note: the window title is maintained by the window. Do not free the string pointer returned by this function!
char* Window_GetTitle(Window* the_window);

uint32_t Window_GetUserData(Window* the_window);

window_type Window_GetType(Window* the_window);
window_state Window_GetState(Window* the_window);
Bitmap* Window_GetBitmap(Window* the_window);

signed int Window_GetX(Window* the_window);
signed int Window_GetY(Window* the_window);
signed int Window_GetWidth(Window* the_window);
signed int Window_GetHeight(Window* the_window);

// Get backdrop yes/no flag. returns true if backdrop, false if not
bool Window_IsBackdrop(Window* the_window);

// Get visible yes/no flag. returns true if window should be rendered, false if not
bool Window_IsVisible(Window* the_window);




// **** RENDER functions *****

void Window_Render(Window* the_window);

// clears the content area rect, setting it to the theme's backcolor
void Window_ClearContent(Window* the_window);




// **** DRAW functions *****

//! Set the font
//! This is the font that will be used for any subsequent font drawing in this Window
//! This also sets the font of the window's bitmap
//! This only affects programmer-controlled drawing actions; it will not change the title bar font, the icon font, etc. Those are controlled by the theme.
//! @param	the_window: reference to a valid Window object.
//! @param	the_font: reference to a complete, loaded Font object.
//! @return Returns false on any error condition
bool Window_SetFont(Window* the_window, Font* the_font);

//! Set the "pen" color
//! This is the color that the next pen-based graphics function will use
//! This also sets the pen color of the window's bitmap
//! This only affects functions that use the pen: any graphics function that specifies a color will use that instead
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return Returns false on any error condition
bool Window_SetColor(Window* the_window, uint8_t the_color);

//! Set the "pen" position within the content area
//! This also sets the pen position of the window's bitmap
//! This is the location that the next pen-based graphics function will use for a starting location
//! @param	the_window: reference to a valid Window object.
//! @param	x: the horizontal position within the content area of the window. Will be clipped to the edges.
//! @param	y: the vertical position within the content area of the window. Will be clipped to the edges.
//! @return Returns false on any error condition
bool Window_SetPenXY(Window* the_window, signed int x, signed int y);

//! Fill a rectangle drawn from the current pen location, for the passed width/height
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_FillBox(Window* the_window, signed int width, signed int height, unsigned char the_color);

//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_FillBoxRect(Window* the_window, Rectangle* the_coords, unsigned char the_color);

//! Set the color of the pixel at the current pen location
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_SetPixel(Window* the_window, unsigned char the_color);

//! Draws a line between 2 passed coordinates.
//! Use for any line that is not perfectly vertical or perfectly horizontal
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! Based on http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C. Used in C128 Lich King. 
bool Window_DrawLine(Window* the_window, signed int x1, signed int y1, signed int x2, signed int y2, unsigned char the_color);

//! Draws a horizontal line from the current pen location, for n pixels, using the specified pixel value
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawHLine(Window* the_window, signed int the_line_len, unsigned char the_color);

//! Draws a vertical line from specified coords, for n pixels
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawVLine(Window* the_window, signed int the_line_len, unsigned char the_color);

//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawBoxRect(Window* the_window, Rectangle* the_coords, unsigned char the_color);

//! Draws a rectangle based on 2 sets of coords, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawBoxCoords(Window* the_window, signed int x1, signed int y1, signed int x2, signed int y2, unsigned char the_color);

//! Draws a rectangle based on start coords and width/height, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Window_DrawBox(Window* the_window, signed int width, signed int height, unsigned char the_color, bool do_fill);

//! Draws a rounded rectangle from the current pen location, with the specified size and radius, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Window_DrawRoundBox(Window* the_window, signed int width, signed int height, signed int radius, unsigned char the_color, bool do_fill);

//! Draw a circle centered on the current pen location
//! Based on http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
//! @param	the_window: reference to a valid Window object.
bool Window_DrawCircle(Window* the_window, signed int radius, unsigned char the_color);

// Draw a string at the current "pen" location, using the current pen color of the Window
// Truncate, but still draw the string if it is too long to display on the line it started.
// No word wrap is performed. 
// If max_chars is less than the string length, only that many characters will be drawn (as space allows)
// If max_chars is -1, then the full string length will be drawn, as space allows.
bool Window_DrawString(Window* the_window, char* the_string, signed int max_chars);

//! Draw a string in a rectangular block on the window, with wrap.
//! The current font, pen location, and pen color of the window will be used
//! If a word can't be wrapped, it will break the word and move on to the next line. So if you pass a rect with 1 char of width, it will draw a vertical line of chars down the screen.
//! @param	the_bitmap: a valid Bitmap object, with a valid font_ property
//! @param	width: the horizontal size of the text wrap box, in pixels. The total of 'width' and the current X coord of the bitmap must not be greater than width of the window's content area.
//! @param	height: the vertical size of the text wrap box, in pixels. The total of 'height' and the current Y coord of the bitmap must not be greater than height of the window's content area.
//! @param	the_string: the null-terminated string to be displayed.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed.
//! @param	wrap_buffer: pointer to a pointer to a temporary text buffer that can be used to hold the wrapped ('formatted') characters. The buffer must be large enough to hold num_chars of incoming text, plus additional line break characters where necessary. 
//! @param	continue_function: optional hook to a function that will be called if the provided text cannot fit into the specified box. If provided, the function will be called each time text exceeds available space. If the function returns true, another chunk of text will be displayed, replacing the first. If the function returns false, processing will stop. If no function is provided, processing will stop at the point text exceeds the available space.
//! @return	returns a pointer to the first character in the string after which it stopped processing (if string is too long to be displayed in its entirety). Returns the original string if the entire string was processed successfully. Returns NULL in the event of any error.
char* Window_DrawStringInBox(Window* the_window, signed int width, signed int height, char* the_string, signed int num_chars, char** wrap_buffer, bool (* continue_function)(void));



// **** Debug functions *****

void Window_Print(Window* the_window);



#endif /* LIB_WINDOW_H_ */


