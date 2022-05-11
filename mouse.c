/*
 * mouse.c
 *
 *  Created on: May 1, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "mouse.h"

// A2560 includes
#include <mcp/syscalls.h>
#include "lib_sys.h"
#include "general.h"
#include "window.h"

// C includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>



/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern System*			global_system;



/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/



/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/





/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/



// **** CONSTRUCTOR AND DESTRUCTOR *****


// constructor
// allocates space for the object, copies in the passed key string and sets a dummy string value
MouseTracker* Mouse_New(void)
{
	MouseTracker*	the_mouse;

	if ( (the_mouse = (MouseTracker*)calloc(1, sizeof(MouseTracker)) ) == NULL)
	{
		LOG_ERR(("Mouse_New: could not allocate memory to create new MouseTracker object."));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_mouse	%p	size	%i", __func__ , __LINE__, the_mouse, sizeof(MouseTracker)));

	// zero out, just in case
	Mouse_Clear(the_mouse);
	
	return the_mouse;
	
error:
	if (the_mouse) Mouse_Destroy(&the_mouse);
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// destructor
void Mouse_Destroy(MouseTracker** the_mouse)
{
	if (*the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	LOG_ALLOC(("%s %d:	__FREE__	*the_mouse	%p	size	%i", __func__ , __LINE__, *the_mouse, sizeof(MouseTracker)));
	free(*the_mouse);
	*the_mouse = NULL;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}



// **** SETTERS *****

// sets the current x, y coord. If button_down is true, it also sets button down coord to passed coord.
// Note: regardless of the value of the_window, clicked_window_ will only be updated if button_down is true. 
//   Window should only be set when calling AcceptUpdate on a mouse down (click)
void Mouse_AcceptUpdate(MouseTracker* the_mouse, Window* the_window, int16_t x, int16_t y, bool button_down)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_mouse->x_ = x;
	the_mouse->y_ = y;
	
	if (button_down)
	{
		the_mouse->clicked_window_ = the_window;
		the_mouse->clicked_x_ = x;
		the_mouse->clicked_y_ = y;
		
		the_mouse->movement_area_.MinX = the_mouse->clicked_x_ - MOUSE_MOVEMENT_THRESHOLD;
		the_mouse->movement_area_.MaxX = the_mouse->clicked_x_ + MOUSE_MOVEMENT_THRESHOLD;
		the_mouse->movement_area_.MinY = the_mouse->clicked_y_ - MOUSE_MOVEMENT_THRESHOLD;
		the_mouse->movement_area_.MaxY = the_mouse->clicked_y_ + MOUSE_MOVEMENT_THRESHOLD;
	}
	
	//Mouse_Print(the_mouse);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// sets the current x, y coord
void Mouse_SetXY(MouseTracker* the_mouse, int16_t x, int16_t y)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_mouse->x_ = x;
	the_mouse->y_ = y;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// resets the mode, coordinates, time
void Mouse_Clear(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_mouse->clicked_window_ = NULL;
	the_mouse->clicked_x_ = -1;
	the_mouse->clicked_y_ = -1;
	the_mouse->x_ = -1;
	the_mouse->y_ = -1;
	the_mouse->mode_ = mouseFree;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// sets the mouse mode (select, doubleclick, drag, lasso, etc.)
void Mouse_SetMode(MouseTracker* the_mouse, MouseMode the_mode)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_mouse->mode_ = the_mode;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}





// **** GETTERS *****

// Get the mouse mode
MouseMode Mouse_GetMode(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->mode_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


// Get the x coord
int16_t Mouse_GetX(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->x_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


// Get the y coord
int16_t Mouse_GetY(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->y_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


// Get the window the mouse was clicked on
Window* Mouse_GetClickedWindow(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->clicked_window_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// Get the last clicked x coord
int16_t Mouse_GetClickedX(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->clicked_x_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


// Get the last clicked y coord
int16_t Mouse_GetClickedY(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->clicked_y_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


// get the horizontal delta between current and last clicked position
int16_t Mouse_GetXDelta(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->x_ - the_mouse->clicked_x_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


// get the vertical delta between current and last clicked position
int16_t Mouse_GetYDelta(MouseTracker* the_mouse)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_mouse->y_ - the_mouse->clicked_y_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}




// **** OTHER FUNCTIONS *****


// check for a double click, assuming it just happened. Returns true if timer says it was double click. If not double click, resets the button down time to now.
bool Mouse_WasDoubleClick(MouseTracker* the_mouse)
{
	uint32_t	now_ticks;
	
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	now_ticks = sys_time_jiffies();
	
	if ( now_ticks - the_mouse->clicked_ticks <= MOUSE_DOUBLE_CLICK_TICKS )
	{
		the_mouse->mode_ = mouseDoubleclick;
		the_mouse->clicked_ticks = 0;
		//DEBUG_OUT(("%s %d: Double click event!", __func__ , __LINE__));

		return true;
	}
	else
	{
		the_mouse->clicked_ticks = now_ticks;
		return false;
	}
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// updates the selection rectangle
void Mouse_UpdateSelectionRectangle(MouseTracker* the_mouse, int16_t x_scrolled, int16_t y_scrolled)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// set up mouse rect differently depending on whether user dragged up->left, up->right, down->right, down->left
	// if lasso mode, we set up rects based on original mouse down point, to current point
	// if not lasso mode, we just make a small buffer around the current mouse point
	if (Mouse_GetMode(the_mouse) == mouseLassoInProgress)
	{
		if (the_mouse->clicked_x_ > the_mouse->x_)
		{
			the_mouse->selection_area_.MinX = the_mouse->x_;
			the_mouse->selection_area_.MaxX = the_mouse->clicked_x_;
		}
		else
		{
			the_mouse->selection_area_.MinX = the_mouse->clicked_x_;
			the_mouse->selection_area_.MaxX = the_mouse->x_;
		}

		if (the_mouse->clicked_y_ > the_mouse->y_)
		{
			the_mouse->selection_area_.MinY = the_mouse->y_;
			the_mouse->selection_area_.MaxY = the_mouse->clicked_y_;
		}
		else
		{
			the_mouse->selection_area_.MinY = the_mouse->clicked_y_;
			the_mouse->selection_area_.MaxY = the_mouse->y_;
		}
	}
	else
	{
		the_mouse->selection_area_.MinX = the_mouse->x_ - MOUSE_POINTER_RADIUS;
		the_mouse->selection_area_.MaxX = the_mouse->x_ + MOUSE_POINTER_RADIUS;
		the_mouse->selection_area_.MinY = the_mouse->y_ - MOUSE_POINTER_RADIUS;
		the_mouse->selection_area_.MaxY = the_mouse->y_ + MOUSE_POINTER_RADIUS;
	}

	// account for scrolling
	the_mouse->selection_area_.MinX += x_scrolled;
	the_mouse->selection_area_.MaxX += x_scrolled;
	the_mouse->selection_area_.MinY += y_scrolled;
	the_mouse->selection_area_.MaxY += y_scrolled;

	//DEBUG_OUT(("%s %d: minx %i, maxX %i, content left %i", __func__ , __LINE__, the_mouse->selection_area_.MinX, the_mouse->selection_area_.MaxX, x_scrolled));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// detect an overlap (selection) between the current selection area of the mouse, and the passed rectangle
bool Mouse_DetectOverlap(MouseTracker* the_mouse, Rectangle the_other_object)
{
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return General_RectIntersect(the_mouse->selection_area_, the_other_object);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// detect whether mouse pointer is far enough away from last click spot to engage lasso mode
bool Mouse_MovedEnoughForLassoStart(MouseTracker* the_mouse)
{	
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return !(General_PointInRect(the_mouse->x_, the_mouse->y_, the_mouse->movement_area_));
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}
	

// detect whether mouse pointer is far enough away from last click spot to engage drag mode
bool Mouse_MovedEnoughForDragStart(MouseTracker* the_mouse)
{	
	if (the_mouse == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return !(General_PointInRect(the_mouse->x_, the_mouse->y_, the_mouse->movement_area_));
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}



// draw a rectangle in the rastport passed, using the mouse coordinates. If doUnDraw is TRUE, try to undraw it (unimplemented TODO)
void Mouse_DrawSelectionBox(MouseTracker* the_mouse)
{
// 	int16_t		coordinates[10];
// 	int16_t		numCoords = ( sizeof(coordinates) / sizeof(coordinates[0]) ) / 2;
// 	int16_t		x1 = the_mouse->clicked_x_ + WINDOW_EDGE_BUFFER; // account for buffer region around buffer bitmap
// 	int16_t		y1 = the_mouse->clicked_y_ + WINDOW_EDGE_BUFFER;
// 	int16_t		x2 = the_mouse->x_ + WINDOW_EDGE_BUFFER;
// 	int16_t		y2 = the_mouse->y_ + WINDOW_EDGE_BUFFER;
// 
// 	if (the_mouse == NULL)
// 	{
// 		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
// 		Sys_Destroy(&global_system); // crash early, crash often
// 	}
// 	
// 	// COMPLEMENT mode will simply inverse pixels. This makes it possible to draw once, then draw again, to restore what was there
// 	SetDrMd(the_rastport, COMPLEMENT);
// 
// 	// need 10 coords to use Intuitions PolyDraw for a box
// 	coordinates[0] = x1;
// 	coordinates[1] = y1;
// 
// 	coordinates[2] = x2;
// 	coordinates[3] = y1;
// 
// 	coordinates[4] = x2;
// 	coordinates[5] = y2;
// 
// 	coordinates[6] = x1;
// 	coordinates[7] = y2;
// 
// 	coordinates[8] = x1;
// 	coordinates[9] = y1;
// 
// 	// set a dotted line pattern
// 	SetDrPt(the_rastport, 0xAAAA);
// 
// 	// draw the surrounding box
// 	General_DrawPoly(the_rastport, numCoords, &coordinates[0], colorPenPrimary);
// 
// 	// turn off pattern
// 	SetDrPt(the_rastport, 0xFFFF);
// 
// 	/* reset draw mode in case we need it for plotting/replotting/etc. */
// 	SetDrMd(the_rastport, JAM1);
}


// **** Debug functions *****

void Mouse_Print(MouseTracker* the_mouse)
{
	DEBUG_OUT(("Mouse print out:"));
	DEBUG_OUT(("  x_: %i",				the_mouse->x_));	
	DEBUG_OUT(("  y_: %i",				the_mouse->y_));	
	DEBUG_OUT(("  clicked_window_: %p",	the_mouse->clicked_window_));	
	DEBUG_OUT(("  clicked_x_: %i",		the_mouse->clicked_x_));	
	DEBUG_OUT(("  clicked_y_: %i",		the_mouse->clicked_y_));	
	DEBUG_OUT(("  mode_: %i", 			the_mouse->mode_));
	DEBUG_OUT(("  clicked_ticks: %lu", 	the_mouse->clicked_ticks));
	DEBUG_OUT(("  selection_area_: %i, %i, %i, %i", the_mouse->selection_area_.MinX, the_mouse->selection_area_.MinY, the_mouse->selection_area_.MaxX, the_mouse->selection_area_.MaxY));
	DEBUG_OUT(("  movement_area_: %i, %i, %i, %i", the_mouse->movement_area_.MinX, the_mouse->movement_area_.MinY, the_mouse->movement_area_.MaxX, the_mouse->movement_area_.MaxY));
}

