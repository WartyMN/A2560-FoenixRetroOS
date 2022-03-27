//! @file lib_window.h

/*
 * lib_window.h
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


// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/lib_general.h>
#include <mb/lib_text.h>
#include <mb/lib_graphics.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define WIN_DEFAULT_MIN_WIDTH		90
#define WIN_DEFAULT_MIN_HEIGHT		60
#define WIN_DEFAULT_MAX_WIDTH		1024
#define WIN_DEFAULT_MAX_HEIGHT		768

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
} window_type;

typedef enum window_state
{
	HIDDEN			= 0,
	MINIMIZED 		= 1,
	NORMAL			= 2,
	MAXIMIZED		= 3,
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
	unsigned long			user_data_;						// 32 bits for use of programs. The system will not process this field. 
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
	Rectangle				titlebar_rect_;					// the rect describing the titlebar area
	Rectangle				grow_left_rect_;				// the rect defining the area in which a click/drag will resize window
	Rectangle				grow_right_rect_;				// the rect defining the area in which a click/drag will resize window
	Rectangle				grow_top_rect_;					// the rect defining the area in which a click/drag will resize window
	Rectangle				grow_bottom_rect_;				// the rect defining the area in which a click/drag will resize window
	boolean					is_backdrop_;					// true if this is the backdrop (desktop) window
	boolean					visible_;						// is the window visible?
	boolean					active_;						// keep 1 window as the active one. only active windows get regular updates
	boolean					changes_to_save_;				// starts at false; if window is resized or repositioned, is set to true. used to know if we have to save out to icon file when window is closed.
	boolean					can_resize_;					// if true, window can be stretched or shrunk. If false, the width_ and height_ will be locked.
	signed int				x_;								// horizontal coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				y_;								// vertical coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				width_;							// width of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				height_;						// height of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	signed int				min_width_;						// minimum width of window when in window-sized (normal) mode.
	signed int				min_height_;					// minimum height of window when in window-sized (normal) mode. 
	signed int				max_width_;						// maximum width of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0.
	signed int				max_height_;					// maximum height of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0. 
	signed int				inner_width_;					// space available inside the window, accounting for border thicknesses
	signed int				inner_height_;					// space available inside the window, accounting for border thicknesses and title bar
	signed int				content_left_;					// of the raw x pos of the window (non-gzz), the x pos where window should start rendering content. =gzz_left_ until window is scrolled leftwards
	signed int				content_top_;					// of the raw y pos of the window (non-gzz), the y pos where window should start rendering content. =gzz_top_ until window is scrolled down
	signed int				required_inner_width_;			// greater of current inner_width or H space required inside the window to display all content. If greater than H space, a scroller is needed.
	signed int				required_inner_height_;			// greater of current inner_height or V space required inside the window to display all content. If greater than V space, a scroller is needed.
	boolean					h_scroller_visible_;
	boolean					v_scroller_visible_;
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


struct NewWindowData
{
	unsigned long			user_data_;						// 32 bits for use of programs. The system will not process this field. 
	char*					title_;
	window_type				type_;
	Bitmap* 				bitmap_;						// on-screen bitmap covering the visible portion of window
	Bitmap* 				buffer_bitmap_;					// off-screen bitmap covering the visible portion of window
	boolean					is_backdrop_;					// true if this is the backdrop (desktop) window
	boolean					can_resize_;					// if true, window can be stretched or shrunk. If false, the width_ and height_ will be locked.
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
Window* Window_New(NewWindowData* the_win_setup);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
boolean Window_Destroy(Window** the_window);






// **** xxx functions *****




// **** Set xxx functions *****





// **** Get xxx functions *****

Control* Window_GetRootControl(Window* the_window);
Control* Window_GetControl(Window* the_window, uint16_t the_control_id);
uint16_t Window_GetControlID(Window* the_window, Control* the_control);

boolean Window_SetControlState(Window* the_window, uint16_t the_control_id);



// **** xxx functions *****

void Window_Render(Window* the_window);



// **** xxx functions *****





// **** Debug functions *****

void Window_Print(Window* the_window);



#endif /* LIB_WINDOW_H_ */


