/*
 * mouse.h
 *
 *  Created on: May 1, 2022
 *      Author: micahbly
 */

#ifndef MOUSE_H_
#define MOUSE_H_



/* about this class: MouseTracker
 *
 * This tracks mouse positions and button actions to make it easier to see if double-click has happened, and to easily see if a given icon is under the mouse pointer
 *
 *** things this class needs to be able to do
 * Track X, Y at last mouse click
 * Track time of last mouse click, and determine if a double-click happened
 * Track a mouse mode, which includes icon selected, drag mode, lasso mode, etc.
 * Determine if a given coordinate pair is within the defined zone around the mouse pointer
 *
 *** things objects of this class have
 *
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/


// project includes
//#include "control.h"

// C includes
#include <stdbool.h>

// A2560 includes
// #include <mcp/syscalls.h>
#include "a2560_platform.h"
#include "general.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define MOUSE_POINTER_RADIUS		2	// number of pixels up/down/left/right from mouse pointer that will be included in selection. might need to be 0
#define MOUSE_MOVEMENT_THRESHOLD	4	// number of pixels away from the mouse-down point that mouse must before before lasso starts drawing or drag mode begins
#define MOUSE_DOUBLE_CLICK_TICKS	30	// maximum number of ticks between first and second click for a double-click event to be registered


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

// covers what action mode the mouse is in after a mouse down event: either dragging one or more icons, or selecting via lasso, or neither
typedef enum MouseMode
{
	mouseFree				= 0,
	mouseSelect				= 1,	// user has clicked on icon(s), but not moved mouse enough to start drag
	mouseDoubleclick		= 2,	
	mouseDrag				= 3,	// user clicked on icon(s) and moved mouse enough to start drag mode
	mouseLasso				= 4,	// nothing under cursor, button down, ready to start drawing a lasso
	mouseLassoInProgress	= 5,	// user has moved mouse from origin point with mouse down, lasso is actively being drawn
	mouseDownOnControl		= 6,	// mouse was clicked within bounds of a control. Mouse button is not released.
	mouseDragTitle			= 7,
	mouseResizeUp			= 8,
	mouseResizeRight		= 9,
	mouseResizeDown			= 10,
	mouseResizeLeft			= 11,
	mouseResizeDownRight	= 12,
} MouseMode;


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

struct MouseTracker
{
	int16_t			clicked_x_;
	int16_t			clicked_y_;
	int16_t			x_;
	int16_t			y_;
	MouseMode		mode_;
	uint32_t		clicked_ticks;
	Rectangle		selection_area_;	// a box around the pointer (if not lasso), or the lasso box, used to detect icon selection and drag-mode start
	Rectangle		movement_area_;		// a box between the last clicked and current location
};


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****


// constructor
// allocates space for the object
MouseTracker* Mouse_New(void);

// destructor
void Mouse_Destroy(MouseTracker** the_mouse);


// **** SETTERS *****

                    
// sets the mouse mode (select, doubleclick, drag, lasso, etc.)
void Mouse_SetMode(MouseTracker* the_mouse, MouseMode the_mode);

// sets the current x, y coord
void Mouse_SetXY(MouseTracker* the_mouse, int16_t x, int16_t y);

// sets the current x, y coord. If button_down is true, it also sets button down coord to passed coord.
void Mouse_AcceptUpdate(MouseTracker* the_mouse, int16_t x, int16_t y, bool button_down);

// updates the selection rectangle
void Mouse_UpdateSelectionRectangle(MouseTracker* the_mouse, int16_t x_scrolled, int16_t y_scrolled);

// clears the target window, panel, and folder
// void Mouse_ClearTarget(MouseTracker* the_mouse);

// sets the source window, panel, and folder
// void Mouse_SetSource(MouseTracker* the_mouse, WB2KWindow* the_surface, WB2KViewPanel* the_panel, WB2KFolderObject* the_folder);

// resets the mode, coordinates, time
void Mouse_Clear(MouseTracker* the_mouse);


// **** GETTERS *****

// Get the mouse mode
MouseMode Mouse_GetMode(MouseTracker* the_mouse);

// Get the x coord
int16_t Mouse_GetX(MouseTracker* the_mouse);

// Get the y coord
int16_t Mouse_GetY(MouseTracker* the_mouse);

// Get the last clicked x coord
int16_t Mouse_GetClickedX(MouseTracker* the_mouse);

// Get the last clicked y coord
int16_t Mouse_GetClickedY(MouseTracker* the_mouse);

// get the horizontal delta between current and last clicked position
int16_t Mouse_GetXDelta(MouseTracker* the_mouse);

// get the vertical delta between current and last clicked position
int16_t Mouse_GetYDelta(MouseTracker* the_mouse);



// **** OTHER FUNCTIONS *****

// check for a double click, assuming it just happened. Returns true if timer says it was double click. If not double click, resets the button down time to now.
bool Mouse_WasDoubleClick(MouseTracker* the_mouse);

// detect an overlap (selection) between the current selection area of the mouse, and the passed rectangle
bool Mouse_DetectOverlap(MouseTracker* the_mouse, Rectangle the_other_object);

// detect whether mouse pointer is far enough away from last click spot to engage lasso mode
bool Mouse_MovedEnoughForLassoStart(MouseTracker* the_mouse);
	
// detect whether mouse pointer is far enough away from last click spot to engage drag mode
bool Mouse_MovedEnoughForDragStart(MouseTracker* the_mouse);

// draw a rectangle in the rastport passed, using the mouse coordinates. If doUnDraw is TRUE, try to undraw it (unimplemented TODO)
void Mouse_DrawSelectionBox(MouseTracker* the_mouse);



// **** Debug functions *****

void Mouse_Print(MouseTracker* the_mouse);




#endif /* MOUSE_H_ */
