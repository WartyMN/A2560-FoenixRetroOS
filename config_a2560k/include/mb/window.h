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
#include "a2560_platform.h"
#include "general.h"
#include "text.h"
#include "bitmap.h"
#include "mouse.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define WIN_DEFAULT_WIDTH			512
#define WIN_DEFAULT_HEIGHT			342
#define WIN_DEFAULT_MIN_WIDTH		90
#define WIN_DEFAULT_MIN_HEIGHT		60
#define WIN_DEFAULT_MAX_WIDTH		800
#define WIN_DEFAULT_MAX_HEIGHT		600

#define WIN_DEFAULT_DRAG_ZONE_SIZE		3		//! The width of left/right drag zones, starting from edge of window, or the height of top/bottom drag zones

#define WIN_DEFAULT_X	50
#define WIN_DEFAULT_Y	25
#define WIN_STAGGER_X	14	// when multiple windows are opened, how far apart are they positioned?
#define WIN_STAGGER_Y	14

#define WINDOW_MAX_WINTITLE_SIZE		128

#define WIN_MAX_CLIP_RECTS				10	//! if a window accumulates more clip rects than this, it will refresh the entire window in one go
#define WIN_MENU_MAX_GROUPS				4	//! Maximum number of menus levels that can be defined per window

#define WIN_PARAM_OPEN_AS_BACKDROP				true	// Window_New() parameter
#define WIN_PARAM_DO_NOT_OPEN_AS_BACKDROP		false	// Window_New() parameter

#define WIN_PARAM_FORCE_CONTROL_REDRAW			true	// Window_DrawControls() parameter
#define WIN_PARAM_ONLY_REDRAW_INVAL_CONTROLS	false	// Window_DrawControls() parameter

#define WIN_PARAM_UPDATE_NORM_SIZE_TO_MATCH		true	// Window_ChangeWindow() parameter
#define WIN_PARAM_DO_NOT_UPDATE_NORM_SIZE		false	// Window_ChangeWindow() parameter


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
	CLOSE_WIDGET_ID			= 0,
	MINIMIZE_WIDGET_ID 		= 1,
	NORM_SIZE_WIDGET_ID		= 2,
	MAXIMIZE_WIDGET_ID		= 3,
	MAX_BUILT_IN_WIDGET		= 4,
} window_base_control_id;




/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


struct ClipRect
{
	int16_t					x_;								// horizontal coordinate, relative to the parent window
	int16_t					y_;								// vertical coordinate, relative to the parent window
	int16_t					width_;
	int16_t					height_;	
};

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
	int16_t					pen_x_;							// H position relative to the content_rect_, of the "pen", for drawing functions
	int16_t					pen_y_;							// V position relative to the content_rect_, of the "pen", for drawing functions
	uint8_t					pen_color_;						// Color index of the "pen", for drawing functions
	Font*					pen_font_;						// Font to be used by the "pen", for drawing functions
	Rectangle				overall_rect_;					// the local rect describing the total area of the window
	Rectangle				titlebar_rect_;					// the local rect describing the titlebar area
	Rectangle				title_drag_rect_;				// the local rect describing the drag zone for the titlebar (excludes the close/etc controls)
	Rectangle				iconbar_rect_;					// the local rect describing the optional iconbar area
	Rectangle				content_rect_;					// the local rect describing the content area of the window
	Rectangle				grow_left_rect_;				// the local rect defining the area in which a click/drag will resize window
	Rectangle				grow_right_rect_;				// the local rect defining the area in which a click/drag will resize window
	Rectangle				grow_top_rect_;					// the local rect defining the area in which a click/drag will resize window
	Rectangle				grow_bottom_rect_;				// the local rect defining the area in which a click/drag will resize window
	Rectangle				grow_bottom_right_rect_;		// the local rect defining the area in which a click/drag will resize window
	bool					show_iconbar_;					// true if the iconbar area should be rendered
	bool					is_backdrop_;					// true if this is the backdrop (desktop) window
	bool					visible_;						// is the window visible?
	bool					active_;						// keep 1 window as the active one. only active windows get regular updates
	bool					changes_to_save_;				// starts at false; if window is resized or repositioned, is set to true. used to know if we have to save out to icon file when window is closed.
	bool					can_resize_;					// if true, window can be stretched or shrunk. If false, the width_ and height_ will be locked.
	bool					invalidated_;					// if true, the window needs to be completely re-rendered on the next render pass
	bool					titlebar_invalidated_;			// if true, the titlebar needs to be redrawn and reblitted on the next render pass
	int16_t					x_;								// current global horizontal coordinate
	int16_t					y_;								// current global vertical coordinate
	int16_t					proposed_x_;					// during a window drag or resize event, the potential future global horizontal coordinate
	int16_t					proposed_y_;					// during a window drag or resize event, the potential future global vertical coordinate
	Rectangle				global_rect_;					// the global rect describing the total area of the window
	int16_t					width_;							// current width of window
	int16_t					height_;						// current height of window
	int16_t					norm_x_;						// global horizontal coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					norm_y_;						// global vertical coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					norm_width_;					// width of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					norm_height_;					// height of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					min_width_;						// minimum width of window when in window-sized (normal) mode.
	int16_t					min_height_;					// minimum height of window when in window-sized (normal) mode. 
	int16_t					max_width_;						// maximum width of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0.
	int16_t					max_height_;					// maximum height of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0. 
	int16_t					inner_width_;					// space available inside the content area, accounting for border thicknesses
	int16_t					inner_height_;					// space available inside the content area, accounting for border thicknesses and title bar
	int16_t					avail_title_width_;				// available pixel width for the title to be rendered, based on delta between title x offset and left-most titlebar control
	int16_t					content_left_;					// of the raw x pos of the window (non-gzz), the x pos where window should start rendering content. =gzz_left_ until window is scrolled leftwards
	int16_t					content_top_;					// of the raw y pos of the window (non-gzz), the y pos where window should start rendering content. =gzz_top_ until window is scrolled down
	int16_t					required_inner_width_;			// greater of current inner_width or H space required inside the window to display all content. If greater than H space, a scroller is needed.
	int16_t					required_inner_height_;			// greater of current inner_height or V space required inside the window to display all content. If greater than V space, a scroller is needed.
	bool					h_scroller_visible_;
	bool					v_scroller_visible_;
	Bitmap*					pattern_;						// optional pattern used for filling the window content rect background on refresh. 
	Window*					parent_window_;					// can be NULL. used for requesters that are spawned from a specific window.
	Window*					child_window_;					// can be NULL. used when a window spawns a requester. (This is the requester). NULLs out again when requester is closed. 
	Control*				root_control_;					// first control in the window
	Control*				selected_control_;				// the currently selected control for the window. Only 1 can be selected per window. No guarantee that any are selected.
	Rectangle				clip_rect_[WIN_MAX_CLIP_RECTS];		// one or more clipping rects; determines which parts of window need to be blitted to the main screen
	int16_t					clip_count_;					// number of clip rects the window is currently tracking
	Rectangle				damage_rect_[4];				// 0 to 4 rects that describe to other windows under this one, which parts of the screen were previously covered by this window (prior to a move or resize)
	int16_t					damage_count_;					// number of damage rects the window is currently tracking
	void					(*event_handler_)(EventRecord*);	// function that will be called by the system when an event related to the window is encountered.
	Menu*					menu_[WIN_MENU_MAX_GROUPS];				// non-permanent containers for menu structures; will be used for first, 2nd, 3rd, and 4th level menus as used in the window.
	int16_t					current_menu_level_;			// index to menu_[]; starts out at menu_no_menu; when a menu is opened, it goes to menu_level_0; increases with each submenu. Resets to menu_no_men uon close of menu.
// 	Window*					zoom_to_window_;				// the window that contains the zoom_to_file, so we can get offset to global screen coords
// 	int16_t					zoom_x_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	int16_t					zoom_y_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	int16_t					zoom_w_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	int16_t					zoom_h_[WIN_ZOOM_RECT_COUNT];	// Used to plot the coords for zoom rects when opening/closing window
// 	unsigned char*			keyboard_buffer_;		// used by key detection
//	uint16_t				keyboard_buf_pos_;		// used by key detection
};


struct NewWinTemplate
{
	uint32_t				user_data_;						// 32 bits for use of programs. The system will not process this field. 
	char*					title_;
	window_type				type_;
	Bitmap* 				bitmap_;						// on-screen bitmap covering the visible portion of window
	Bitmap* 				buffer_bitmap_;					// off-screen bitmap covering the visible portion of window
	bool					show_iconbar_;					// true if the iconbar area should be rendered
	bool					is_backdrop_;					// true if this is the backdrop (desktop) window
	bool					can_resize_;					// if true, window can be stretched or shrunk. If false, the width_ and height_ will be locked.
	int16_t					x_;								// horizontal coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					y_;								// vertical coordinate when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					width_;							// width of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					height_;						// height of window when in window-sized (normal) mode. Not adjusted when window is minimized or maximized.
	int16_t					min_width_;						// minimum width of window when in window-sized (normal) mode.
	int16_t					min_height_;					// minimum height of window when in window-sized (normal) mode. 
	int16_t					max_width_;						// maximum width of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0.
	int16_t					max_height_;					// maximum height of window when in window-sized (normal) mode. If > 0, the window will not maximize. Default 0. 
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
//! the_win_template->width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH, cannot be greater than WIN_DEFAULT_MAX_WIDTH
//! the_win_template->height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT, cannot be greater than WIN_DEFAULT_MAX_HEIGHT
//! the_win_template->min_width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH
//! the_win_template->min_height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT
//! the_win_template->max_width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH
//! the_win_template->max_height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT
//! @param	the_win_template: a populated new window template whose data will be used to create the new window object
//! @param	event_handler: pointer to the function that will handle all events for the window. This function will be called when the system detects an event associated with the window.
Window* Window_New(NewWinTemplate* the_win_template, void (* event_handler)(EventRecord*));

// destructor
// frees all allocated memory associated with the passed object, and the object itself
//! @param	the_window: reference to a valid Window object.
bool Window_Destroy(Window** the_window);

//! Allocate and populate a new window template object
//! Assigns (but does not copy) the passed title string; leaves bitmaps NULL; assigns the pre-defined default value to all other fields
//! Calling method must free the returned NewWinTemplate pointer after creating a window with it.
//! @param	the_win_title: pointer to the string that will be assigned to the title_ property. No copy or allocation will take place.
//! @return:	A NewWinTemplate with all values set to default, or NULL on any error condition
NewWinTemplate* Window_GetNewWinTemplate(char* the_win_title);



// **** CLIP RECT MANAGEMENT functions *****

//! Copy the passed rectangle to the window's clip rect collection
//! If the window already has the maximum allowed number of clip rects, the rectangle will not be added.
//! NOTE: the incoming rect must be using window-local coordinates, not global. No translation will be performed.
//! @param	the_window: reference to a valid Window object.
//! @param	new_rect: reference to the rectangle describing the coordinates to be added to the window as a clipping rect. Coordinates of this rect must be window-local! Coordinates in rect are copied to window storage, so it is safe to free the rect after calling this function.
//! @return:	Returns true if rect is copied successfully. Returns false on any error, or if the window already had the maximum number of allowed clip rects.
bool Window_AddClipRect(Window* the_window, Rectangle* new_rect);

//! Merge and de-duplicate clip rects
//! Consolidating the clip rects will happen reduce unnecessary reblitting
//! @param	the_window: reference to a valid Window object.
bool Window_MergeClipRects(Window* the_window);

//! Blit each clip rect to the screen, and clear all clip rects when done
//! This is the actual mechanics of rendering the window to the screen
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if there are either no clips to blit, or if there are clips and they are blitted successfully. Returns false on any error.
bool Window_BlitClipRects(Window* the_window);

//! Calculate damage rects, if any, caused by window moving or being resized
//! NOTE: it is not necessarily an error condition if a given window doesn't end up with damage rects as a result of this operation: if the window rect doesn't intersect the incoming rect, no damage is relevant.
//! @param	the_window: reference to a valid Window object.
//! @param	the_old_rect: reference to the rectangle to be checked for overlap with the specified window. Coordinates of this rect must be global!
//! @return:	Returns true if 1 or more damage rects were created. Returns false on any error condition, or if no damage rects needed to be created.
bool Window_GenerateDamageRects(Window* the_window, Rectangle* the_old_rect);

//! Copy the passed rectangle to the window's clip rect collection, translating to local coordinates as it does so
//! NOTE: the incoming rect is assumed to be using global, not window-local coordinates. Coordinates will be translated to window-local. 
//! Note: it is safe to pass non-intersecting rects to this function: it will check for non-intersection; will trim copy of clip to just the intersection
//! @param	the_window: reference to a valid Window object.
//! @param	damage_rect: reference to the rectangle describing the coordinates to be added to the window as a clipping rect.. Coordinates of this rect must be global!
//! @return:	Returns true if the passed rect has any intersection with the window. Returns false if not intersection, or on any error condition.
bool Window_AcceptDamageRect(Window* the_window, Rectangle* damage_rect);



// **** BUILT-IN PSEUDO CONTROL MANAGEMENT functions *****

//! Checks if the passed coordinate is within one of the draggable event zones
//! Draggable event zones include the title bar, 4 single-direction resize zones on the window edges, and the lower-right traditional resize zone
//! @param	the_window: reference to a valid Window object.
//! @param	x: window-local horizontal coordinate
//! @param	y: window-local vertical coordinate
//@ return	Returns mouseFree if the coordinates are in anything but a draggable region. Otherwise returns mouseResizeUp, etc., as appropriate.
MouseMode Window_CheckForDragZone(Window* the_window, int16_t x, int16_t y);



// **** CONTROL MANAGEMENT functions *****

//! Sets the passed control as the currently selected control and unselects any previously selected control
//! @param	the_window: reference to a valid Window object.
//! @param	the_control: reference to a valid Control object.
//! @return:	Returns false on any error
bool Window_SetSelectedControl(Window* the_window, Control* the_control);

//! Adds the passed control to the window's list of controls
//! @param	the_window: reference to a valid Window object.
//! @param	the_control: reference to a valid Control object.
//! @return:	Returns false in any error condition
bool Window_AddControl(Window* the_window, Control* the_control);

//! Instantiate a new control from the passed template, and add it to the window's list of controls
//! @param	the_window: reference to a valid Window object.
//! @param	the_template: reference to a valid, populated ControlTemplate object. The created control will take most of its properties from this template.
//! @param	the_id: the unique ID (within the specified window) to be assigned to the control. WARNING: assigning multiple controls the same ID will result in undefined behavior.
//! @param	group_id: 1 byte group ID value to be assigned to the control. Pass CONTROL_NO_GROUP if the control is not to be part of a group.
//! @return:	Returns a pointer to the new control, or NULL in any error condition
Control* Window_AddNewControlFromTemplate(Window* the_window, ControlTemplate* the_template, uint16_t the_id, uint16_t group_id);

//! Instantiate a new control of the type specified, and add it to the window's list of controls
//! @param	the_window: reference to a valid Window object.
//! @param	the_type: the type of control to be created. See the control_type enum definition.
//! @param	width: width, in pixels, of the control to be created
//! @param	height: height, in pixels, of the control to be created
//! @param	x_offset: horizontal offset, in pixels, from the left or right edge of the control, to the left or right edge of the parent rect, depending on the alignment choice
//! @param	y_offset: vertical offset, in pixels, from the top or bottom edge of the control, to the top or bottom edge of the parent rect, depending on the alignment choice
//! @param	the_h_align: horizontal alignment choice; determines if the control is located relative to the right or left edge of the parent rect, or is centered
//! @param	the_v_align: vertical alignment choice; determines if the control is located relative to the top or bottom edge of the parent rect, or is centered
//! @param	the_caption: optional string to be used as the caption for the control. Not all controls support captions. The string will be copied to the control's storage, so it is safe to free the string after calling this function.
//! @param	the_id: the unique ID (within the specified window) to be assigned to the control. WARNING: assigning multiple controls the same ID will result in undefined behavior.
//! @param	group_id: 1 byte group ID value to be assigned to the control. Pass CONTROL_NO_GROUP if the control is not to be part of a group.
//! @return:	Returns a pointer to the new control, or NULL in any error condition
Control* Window_AddNewControl(Window* the_window, control_type the_type, int16_t width, int16_t height, int16_t x_offset, int16_t y_offset, h_align_type the_h_align, v_align_type the_v_align, char* the_caption, uint16_t the_id, uint16_t group_id);

//! Invalidate the title bar and the controls in the title bar
//! Call when switching from inactive to active window, and vice versa, to force controls and title bar to redraw appropriately
//! @param	the_window: reference to a valid Window object.
void Window_InvalidateTitlebar(Window* the_window);

//! Invalidate the control matching the ID passed
//! @param	the_window: reference to a valid Window object.
//! @param	the_control_id: ID of the control that you want to invalidate
void Window_InvalidateControlByID(Window* the_window, uint16_t the_control_id);



//! Get the control listed as the currently selected control.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a control pointer, or NULL on any error, or if there is no selected control currently
Control* Window_GetSelectedControl(Window* the_window);

//! Find and return the last control in the window's chain of controls
//! This corresponds to the first control with a NULL value for next_
//! @param	the_window: reference to a valid Window object.
Control* Window_GetLastControl(Window* the_window);

//! Return a pointer to the control owned by the window that matches the specified ID
//! NOTE: Control IDs 0-3 are reserved by the system for standard controls (close, minimize, etc.). Other control IDs are specified by the programmer for each window. Control IDs are not global, they are window-specific.
//! @param	the_window: reference to a valid Window object.
//! @param	the_control_id: ID of the control that you want to find
//! @return:	Returns a pointer to the control with the ID passed, or NULL if no match found, or on any error condition
Control* Window_GetControl(Window* the_window, uint16_t the_control_id);

//! Return the ID of the control passed, if the window actually owns that control
//! NOTE: Control IDs 0-3 are reserved by the system for standard controls (close, minimize, etc.). Other control IDs are specified by the programmer for each window. Control IDs are not global, they are window-specific.
//! @param	the_window: reference to a valid Window object.
//! @param	the_control: Pointer to the control whose ID you want to find
//! @return:	Returns the ID of the control, or CONTROL_ID_NOT_FOUND if no match found, or CONTROL_ID_ERROR on any error condition
uint16_t Window_GetControlID(Window* the_window, Control* the_control);

//! Find the control, if any, located at the specified local coordinates
//! Transparency is not taken into account: if the passed coordinate intersects the control's rectangle at any point, it is considered a match
//! @param	the_window: reference to a valid Window object.
//! @param	x: window-local horizontal coordinate
//! @param	y: window-local vertical coordinate
//! @return:	Returns a pointer to the control at the passed coordinates, or NULL if no match found, or on any error condition
Control* Window_GetControlAtXY(Window* the_window, int16_t x, int16_t y);



// **** Set functions *****


//! Replace the current window title with the passed string
//! Note: the passed string will be copied into storage by the window. The passing function is free to dispose of the passed string after calling this function.
//! @param	the_window: reference to a valid Window object.
//! @param	the_title: string containing the new title for the window.
void Window_SetTitle(Window* the_window, char* the_title);

//! Set the window's visibility flag.
//! NOTE: This does not immediately cause the window to render. The window will be rendered on the next system rendering pass.
//! @param	the_window: reference to a valid Window object.
//! @param	is_visible: set to true if window should be rendered in the next pass, false if not
void Window_SetVisible(Window* the_window, bool is_visible);

//! Set the display order of the window
//! NOTE: This does not immediately re-render or change the display order visibly.
//! WARNING: This function is designed to be called by the system only: do not use this
//! @param	the_window: reference to a valid Window object
//! @param	the_display_order: the new display order value for the window
void Window_SetDisplayOrder(Window* the_window, int8_t the_display_order);

//! Set the passed window's active flag.
//! NOTE: This does not immediately cause the window to render as active or inactive, but it does invalidate the title bar so that it re-renders in the next render pass.
//! @param	the_window: reference to a valid Window object.
//! @param	is_active: set to true if window is now considered the active window, false if not
void Window_SetActive(Window* the_window, bool is_active);

//! Set the window's state (maximized, minimized, etc.)
//! NOTE: This does not immediately cause the window to render in the passed state.
//! @param	the_window: reference to a valid Window object.
//! @param	the_state: the new state
void Window_SetState(Window* the_window, window_state the_state);

//! Evaluate potential change to window position or size, and correct if out of allowed limits
//! Negative value positions will be corrected to 0.
//! @param	the_window: reference to a valid Window object.
//! @param	x: Pointer to the proposed new horizontal position. If less than 0, it will be set to 0.
//! @param	y: Pointer to the proposed new vertical position. If less than 0, it will be set to 0.
//! @param	width: Pointer to the proposed new width. Will be set to window's minimum or maximum if necessary.
//! @param	height: Pointer to the proposed new height. Will be set to window's minimum or maximum if necessary.
void Window_EvaluateWindowChange(Window* the_window, int16_t* x, int16_t* y, int16_t* width, int16_t* height);

//! Change position and/or size of window
//! NOTE: passed x, y will be checked against the window's min/max values
//! Will also adjust the position of the built-in maximize/minimize/normsize controls
//! @param	the_window: reference to a valid Window object.
//! @param	x: The new global horizontal position
//! @param	y: The new global vertical position
//! @param	width: The new width
//! @param	height: The new height
//! @param	update_norm: if true, the window's normal x/y/width/height properties will be updated to match the passed values. Pass false if setting maximize size, etc.
void Window_ChangeWindow(Window* the_window, int16_t x, int16_t y, int16_t width, int16_t height, bool update_norm);

//! Set the window to full-screen size (maximize mode)
//! Sets window's x, y, width, height parameters to match those of the screen
//! @param	the_window: reference to a valid Window object.
void Window_Maximize(Window* the_window);

//! Set the window to normal size (window-size mode)
//! Sets window's x, y, width, height parameters to those stored in norm_x, etc.
//! @param	the_window: reference to a valid Window object.
void Window_NormSize(Window* the_window);

//! Hides the window (minimize mode)
//! Does not change the window's x, y, width, height parameters, it just makes it invisible
//! @param	the_window: reference to a valid Window object.
void Window_Minimize(Window* the_window);


// **** Get functions *****


//! Get a pointer to the current window title
//! Note: It is not guaranteed that every window will have a title. Backdrop windows, for example, do not have a title.
//! Note: the window title is maintained by the window. Do not free the string pointer returned by this function!
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a pointer to the title string. Returns NULL in any error condition.
char* Window_GetTitle(Window* the_window);

//! Get the value stored in the user data field of the window.
//! NOTE: this field is for the exclusive use of application programs. The system will not act on this data in any way: you are free to store whatever 4-byte value you want here.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns an unsigned 32 bit value. Returns 0 in any error condition.
uint32_t Window_GetUserData(Window* the_window);

//! Get the window's type (normal, backdrop, dialog, etc.)
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a window_type enum. Returns WIN_UNKNOWN_TYPE in any error condition.
window_type Window_GetType(Window* the_window);

//! Get the window's state (maximized, minimized, etc.)
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a window_state enum. Returns WIN_UNKNOWN_STATE in any error condition.
window_state Window_GetState(Window* the_window);

//! Get the bitmap object used as the offscreen buffer for the window
//! NOTE: this is not a pointer into VRAM, or directly to the screen.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a pointer to the bitmap used by the window. Returns NULL in any error condition.
Bitmap* Window_GetBitmap(Window* the_window);

//! Get the global horizontal coordinate of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns the horizontal portion of the upper-left coordinate of the window. Returns -1 in any error condition.
int16_t Window_GetX(Window* the_window);

//! Get the global vertical coordinate of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns the vertical portion of the upper-left coordinate of the window. Returns -1 in any error condition.
int16_t Window_GetY(Window* the_window);

//! Get the width of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns -1 in any error condition.
int16_t Window_GetWidth(Window* the_window);

//! Get the height of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns -1 in any error condition.
int16_t Window_GetHeight(Window* the_window);

//! Check if a window is a backdrop window or a regular window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if backdrop, false if not
bool Window_IsBackdrop(Window* the_window);

//! Check if a window should be visible or not
//! NOTE: this does not necessarily mean the window isn't currently rendered to the screen. This indicates if it will or won't be after the next render pass.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if window should be rendered, false if not
bool Window_IsVisible(Window* the_window);

//! Get the active/inactive condition of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if window is active, false if not
bool Window_IsActive(Window* the_window);



// **** RENDER functions *****

//! Draw/re-draw any necessary components, and blit the window (or parts of it, via cliprects) to the screen
//! @param	the_window: reference to a valid Window object.
void Window_Render(Window* the_window);

//! Clears the content area rect by filling it with the theme's backcolor
//! @param	the_window: reference to a valid Window object.
void Window_ClearContent(Window* the_window);

//! Updates the current window controls, etc., to match the current system theme 
//! @param	the_window: reference to a valid Window object.
void Window_UpdateTheme(Window* the_window);

//! Mark entire window as invalidated
//! This will cause it to be redrawn and fully reblitted in the next render cycle
//! @param	the_window: reference to a valid Window object.
void Window_Invalidate(Window* the_window);



// **** DRAW functions *****

//! Convert the passed x, y global coordinates to local (to window) coordinates
//! @param	the_window: reference to a valid Window object.
//! @param	x: the global horizontal position to be converted to window-local.
//! @param	y: the global vertical position to be converted to window-local.
void Window_GlobalToLocal(Window* the_window, int16_t* x, int16_t* y);

//! Convert the passed x, y local coordinates to global coordinates
//! @param	x: the window-local horizontal position to be converted to global.
//! @param	y: the window-local vertical position to be converted to global.
//! @param	the_window: reference to a valid Window object.
void Window_LocalToGlobal(Window* the_window, int16_t* x, int16_t* y);

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

//! Set the local "pen" position within the window, based on global coordinates
//! This also sets the pen position of the window's bitmap
//! This is the location that the next pen-based graphics function will use for a starting location
//! @param	the_window: reference to a valid Window object.
//! @param	x: the global horizontal position to be converted to window-local. Will be clipped to the edges.
//! @param	y: the global vertical position to be converted to window-local. Will be clipped to the edges.
//! @return Returns false on any error condition
bool Window_SetPenXYFromGlobal(Window* the_window, int16_t x, int16_t y);

//! Set the "pen" position within the content area
//! This also sets the pen position of the window's bitmap
//! This is the location that the next pen-based graphics function will use for a starting location
//! @param	the_window: reference to a valid Window object.
//! @param	x: the horizontal position within the content area of the window. Will be clipped to the edges.
//! @param	y: the vertical position within the content area of the window. Will be clipped to the edges.
//! @return Returns false on any error condition
bool Window_SetPenXY(Window* the_window, int16_t x, int16_t y);

//! Blit from source bitmap to the window's content area, at the window's current pen coordinate
//! The source bitmap can be the window's bitmap: you can use this to copy a chunk of pixels from one part of a window to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param	the_window: reference to a valid Window object.
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the rectangle you want to copy. May be negative.
//! @param width, height: the scope of the copy, in pixels.
bool Window_Blit(Window* the_window, Bitmap* src_bm, int16_t src_x, int16_t src_y, int16_t width, int16_t height);

//! Fill a rectangle drawn from the current pen location, for the passed width/height
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_FillBox(Window* the_window, int16_t width, int16_t height, uint8_t the_color);

//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_FillBoxRect(Window* the_window, Rectangle* the_coords, uint8_t the_color);

//! Set the color of the pixel at the current pen location
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_SetPixel(Window* the_window, uint8_t the_color);

//! Draws a line between 2 passed coordinates.
//! Use for any line that is not perfectly vertical or perfectly horizontal
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! @param	the_color: a 1-byte index to the current color LUT
//! Based on http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C. Used in C128 Lich King. 
bool Window_DrawLine(Window* the_window, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color);

//! Draws a horizontal line from the current pen location, for n pixels, using the specified pixel value
//! @param	the_window: reference to a valid Window object.
//! @param	the_line_len: The total length of the line, in pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawHLine(Window* the_window, int16_t the_line_len, uint8_t the_color);

//! Draws a vertical line from specified coords, for n pixels
//! @param	the_window: reference to a valid Window object.
//! @param	the_line_len: The total length of the line, in pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawVLine(Window* the_window, int16_t the_line_len, uint8_t the_color);

//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawBoxRect(Window* the_window, Rectangle* the_coords, uint8_t the_color);

//! Draws a rectangle based on 2 sets of coords, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawBoxCoords(Window* the_window, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color);

//! Draws a rectangle based on start coords and width/height, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return:	returns false on any error/invalid input.
bool Window_DrawBox(Window* the_window, int16_t width, int16_t height, uint8_t the_color, bool do_fill);

//! Draws a rounded rectangle from the current pen location, with the specified size and radius, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return:	returns false on any error/invalid input.
bool Window_DrawRoundBox(Window* the_window, int16_t width, int16_t height, int16_t radius, uint8_t the_color, bool do_fill);

//! Draw a circle centered on the current pen location
//! Based on http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
//! @param	the_window: reference to a valid Window object.
//! @param	radius: radius, in pixels, measured from the window's current pen location
//! @param	the_color: a 1-byte index to the current color LUT
bool Window_DrawCircle(Window* the_window, int16_t radius, uint8_t the_color);

// Draw a string at the current "pen" location, using the current pen color of the Window
// Truncate, but still draw the string if it is too long to display on the line it started.
// No word wrap is performed. 
// If max_chars is less than the string length, only that many characters will be drawn (as space allows)
// If max_chars is -1, then the full string length will be drawn, as space allows.
bool Window_DrawString(Window* the_window, char* the_string, int16_t max_chars);

//! Draw a string in a rectangular block on the window, with wrap.
//! The current font, pen location, and pen color of the window will be used
//! If a word can't be wrapped, it will break the word and move on to the next line. So if you pass a rect with 1 char of width, it will draw a vertical line of chars down the screen.
//! @param	the_window: reference to a valid Window object.
//! @param	width: the horizontal size of the text wrap box, in pixels. The total of 'width' and the current X coord of the bitmap must not be greater than width of the window's content area.
//! @param	height: the vertical size of the text wrap box, in pixels. The total of 'height' and the current Y coord of the bitmap must not be greater than height of the window's content area.
//! @param	the_string: the null-terminated string to be displayed.
//! @param	num_chars: either the length of the passed string, or as much of the string as should be displayed.
//! @param	wrap_buffer: pointer to a pointer to a temporary text buffer that can be used to hold the wrapped ('formatted') characters. The buffer must be large enough to hold num_chars of incoming text, plus additional line break characters where necessary. 
//! @param	continue_function: optional hook to a function that will be called if the provided text cannot fit into the specified box. If provided, the function will be called each time text exceeds available space. If the function returns true, another chunk of text will be displayed, replacing the first. If the function returns false, processing will stop. If no function is provided, processing will stop at the point text exceeds the available space.
//! @return:	returns a pointer to the first character in the string after which it stopped processing (if string is too long to be displayed in its entirety). Returns the original string if the entire string was processed successfully. Returns NULL in the event of any error.
char* Window_DrawStringInBox(Window* the_window, int16_t width, int16_t height, char* the_string, int16_t num_chars, char** wrap_buffer, bool (* continue_function)(void));



// **** Debug functions *****

//! @param	the_window: reference to a valid Window object.
void Window_Print(Window* the_window);

// helper function called by List class's print function: prints one window entry
void Window_PrintBrief(void* the_payload);


#endif /* LIB_WINDOW_H_ */


