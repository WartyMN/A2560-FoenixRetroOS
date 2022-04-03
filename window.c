/*
 * window.c
 *
 *  Created on: Mar 19, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "window.h"
#include "theme.h"
#include "control.h"

// C includes
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/bitmap.h>
#include <mb/text.h>
#include <mb/font.h>
#include <mb/lib_sys.h>


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

//! set or update the drag rects on the four edges of the screen
//! Call this after every window resize event
void Window_ConfigureDragRects(Window* the_window);

//! set up the rects for titlebar, content, etc. 
void Window_ConfigureStructureRects(Window* the_window);

//! verify and adjust window width, height, etc, 
void Window_CheckDimensions(Window* the_window, NewWindowData* the_win_setup);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


//! set or update the drag rects on the four edges of the screen
//! Call this after every window resize event
void Window_ConfigureDragRects(Window* the_window)
{
	the_window->grow_left_rect_.MinX = 0;
	the_window->grow_left_rect_.MinY = 0;
	the_window->grow_left_rect_.MaxX = WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_left_rect_.MaxY = the_window->height_ - 1;

	the_window->grow_right_rect_.MinX = the_window->width_ - WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_right_rect_.MinY = 0;
	the_window->grow_right_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_right_rect_.MaxY = the_window->height_ - 1;

	the_window->grow_top_rect_.MinX = 0;
	the_window->grow_top_rect_.MinY = 0;
	the_window->grow_top_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_top_rect_.MaxY = WIN_DEFAULT_DRAG_ZONE_SIZE - 1;

	the_window->grow_bottom_rect_.MinX = 0;
	the_window->grow_bottom_rect_.MinY = the_window->height_ - WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_bottom_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_bottom_rect_.MaxY = the_window->height_ - 1;
}



//! set up the rects for titlebar, content, etc. 
void Window_ConfigureStructureRects(Window* the_window)
{
	// LOGIC: 
	//   In final model, the location and height of the content and titlebar rects will be configurable
	//   In this early prototype, they are going to be hard coded here
	
	// inset the titlebar and content rects by 1 pixel, compared to overall rect.
	the_window->titlebar_rect_.MinX = 1;
	the_window->titlebar_rect_.MinY = 1;
	the_window->titlebar_rect_.MaxX = the_window->width_ - 2;
	the_window->titlebar_rect_.MaxY = 19;

	the_window->content_rect_.MinX = 1;
	the_window->content_rect_.MinY = the_window->titlebar_rect_.MaxY + 1;
	the_window->content_rect_.MaxX = the_window->width_ - 2;
	the_window->content_rect_.MaxY = the_window->height_ - 2;

	the_window->overall_rect_.MinX = 0;
	the_window->overall_rect_.MinY = 0;
	the_window->overall_rect_.MaxX = the_window->width_ - 1;
	the_window->overall_rect_.MaxY = the_window->height_ - 1;
}


//! verify and adjust window width, height, etc, 
void Window_CheckDimensions(Window* the_window, NewWindowData* the_win_setup)
{
	
	DEBUG_OUT(("%s %d: checking window dimensions...", __func__, __LINE__));

	if (the_win_setup->min_width_ < WIN_DEFAULT_MIN_WIDTH)
	{
		the_win_setup->min_width_ = WIN_DEFAULT_MIN_WIDTH;
	}
	
	if (the_win_setup->min_height_ < WIN_DEFAULT_MIN_HEIGHT)
	{
		the_win_setup->min_height_ = WIN_DEFAULT_MIN_HEIGHT;
	}
	
	if (the_win_setup->max_width_ < WIN_DEFAULT_MIN_WIDTH)
	{
		the_win_setup->max_width_ = WIN_DEFAULT_MIN_WIDTH;
	}
	
	if (the_win_setup->max_height_ < WIN_DEFAULT_MIN_HEIGHT)
	{
		the_win_setup->max_height_ = WIN_DEFAULT_MIN_HEIGHT;
	}
	
	if (the_win_setup->width_ < WIN_DEFAULT_MIN_WIDTH)
	{
		the_win_setup->width_ = WIN_DEFAULT_MIN_WIDTH;
	}
	
	if (the_win_setup->height_ < WIN_DEFAULT_MIN_HEIGHT)
	{
		the_win_setup->height_ = WIN_DEFAULT_MIN_HEIGHT;
	}
	
	if (the_win_setup->width_ > WIN_DEFAULT_MAX_WIDTH)
	{
		the_win_setup->width_ = WIN_DEFAULT_MAX_WIDTH;
	}
	
	if (the_win_setup->height_ > WIN_DEFAULT_MAX_HEIGHT)
	{
		the_win_setup->height_ = WIN_DEFAULT_MAX_HEIGHT;
	}

	if (!the_win_setup->can_resize_)
	{
		the_win_setup->min_width_ = the_win_setup->width_;
		the_win_setup->min_height_ = the_win_setup->height_;
		the_win_setup->max_width_ = the_win_setup->width_;
		the_win_setup->max_height_ = the_win_setup->height_;
	}
}






// **** Debug functions *****

void Window_Print(Window* the_window)
{
	DEBUG_OUT(("Window print out:"));
	DEBUG_OUT(("  address: %p", 		the_window));
	DEBUG_OUT(("  id_: %i", 			the_window->id_));
	DEBUG_OUT(("  display_order_: %i", 	the_window->display_order_));
	DEBUG_OUT(("  user_data_: %i",		the_window->user_data_));
	DEBUG_OUT(("  title_: '%s'",		the_window->title_));
	DEBUG_OUT(("  state_: %i",			the_window->state_));
	DEBUG_OUT(("  bitmap_: %p",			the_window->bitmap_));
	DEBUG_OUT(("  buffer_bitmap_: %p",	the_window->buffer_bitmap_));
	DEBUG_OUT(("  overall_rect_: %i, %i, %i, %i", the_window->overall_rect_.MinX, the_window->overall_rect_.MinY, the_window->overall_rect_.MaxX, the_window->overall_rect_.MaxY));
	DEBUG_OUT(("  content_rect_: %i, %i, %i, %i", the_window->content_rect_.MinX, the_window->content_rect_.MinY, the_window->content_rect_.MaxX, the_window->content_rect_.MaxY));
	DEBUG_OUT(("  titlebar_rect_: %i, %i, %i, %i", the_window->titlebar_rect_.MinX, the_window->titlebar_rect_.MinY, the_window->titlebar_rect_.MaxX, the_window->titlebar_rect_.MaxY));
	DEBUG_OUT(("  grow_left_rect_: %i, %i, %i, %i", the_window->grow_left_rect_.MinX, the_window->grow_left_rect_.MinY, the_window->grow_left_rect_.MaxX, the_window->grow_left_rect_.MaxY));
	DEBUG_OUT(("  grow_right_rect_: %i, %i, %i, %i", the_window->grow_right_rect_.MinX, the_window->grow_right_rect_.MinY, the_window->grow_right_rect_.MaxX, the_window->grow_right_rect_.MaxY));
	DEBUG_OUT(("  grow_top_rect_: %i, %i, %i, %i", the_window->grow_top_rect_.MinX, the_window->grow_top_rect_.MinY, the_window->grow_top_rect_.MaxX, the_window->grow_top_rect_.MaxY));
	DEBUG_OUT(("  grow_bottom_rect_: %i, %i, %i, %i", the_window->grow_bottom_rect_.MinX, the_window->grow_bottom_rect_.MinY, the_window->grow_bottom_rect_.MaxX, the_window->grow_bottom_rect_.MaxY));
	DEBUG_OUT(("  is_backdrop_: %i",	the_window->is_backdrop_));
	DEBUG_OUT(("  visible_: %i",		the_window->visible_));
	DEBUG_OUT(("  active_: %i",			the_window->active_));
	DEBUG_OUT(("  changes_to_save_: %i",	the_window->changes_to_save_));
	DEBUG_OUT(("  can_resize_: %i",		the_window->can_resize_));	
	DEBUG_OUT(("  x_: %i",				the_window->x_));	
	DEBUG_OUT(("  y_: %i",				the_window->y_));	
	DEBUG_OUT(("  width_: %i",			the_window->width_));	
	DEBUG_OUT(("  height_: %i",			the_window->height_));	
	DEBUG_OUT(("  min_width_: %i",		the_window->min_width_));	
	DEBUG_OUT(("  min_height_: %i",		the_window->min_height_));	
	DEBUG_OUT(("  max_width_: %i",		the_window->max_width_));	
	DEBUG_OUT(("  max_height_: %i",		the_window->max_height_));	
	DEBUG_OUT(("  inner_width_: %i",	the_window->inner_width_));	
	DEBUG_OUT(("  inner_height_: %i",	the_window->inner_height_));	
	DEBUG_OUT(("  content_left_: %i",	the_window->content_left_));	
	DEBUG_OUT(("  content_top_: %i",	the_window->content_top_));	
	DEBUG_OUT(("  required_inner_width_: %i",		the_window->required_inner_width_));	
	DEBUG_OUT(("  required_inner_height_: %i",		the_window->required_inner_height_));	
	DEBUG_OUT(("  h_scroller_visible_: %i",			the_window->h_scroller_visible_));	
	DEBUG_OUT(("  v_scroller_visible_: %i",			the_window->v_scroller_visible_));	
	DEBUG_OUT(("  pattern_: %p",		the_window->pattern_));	
	DEBUG_OUT(("  parent_window_: %p",	the_window->parent_window_));	
	DEBUG_OUT(("  child_window_: %p",	the_window->child_window_));	
	DEBUG_OUT(("  root_control_: %p",	the_window->root_control_));	
}



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate a Window object
//! @param	the_win_setup->width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH, cannot be greater than WIN_DEFAULT_MAX_WIDTH
//! @param	the_win_setup->height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT, cannot be greater than WIN_DEFAULT_MAX_HEIGHT
//! @param	the_win_setup->min_width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH
//! @param	the_win_setup->min_height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT
//! @param	the_win_setup->max_width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH
//! @param	the_win_setup->max_height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT
Window* Window_New(NewWindowData* the_win_setup)
{
	Window*		the_window;
	Theme*		the_theme;
	Control*	close_control;
	Control*	minimize_control;
	Control*	normsize_control;
	Control*	maximize_control;
		
	// LOGIC: 
	//   Only a few parameter values can be so bad that the window creation process must terminate
	//   The offscreen bitmap can be NULL and be ok, but the main Bitmap MUST be valid. 
	//   The size parameters can be adjusted to match system mins and maxs.
	
	if ( the_win_setup == NULL)
	{
		LOG_ERR(("%s %d: passed NewWindowData was NULL", __func__ , __LINE__));
		goto error;
	}
	
	if ( the_win_setup->bitmap_ == NULL)
	{
		LOG_ERR(("%s %d: passed bitmap was NULL", __func__ , __LINE__));
		goto error;
	}
	
	DEBUG_OUT(("%s %d: x=%i, y=%i, width=%i", __func__, __LINE__, the_win_setup->x_, the_win_setup->y_, the_win_setup->width_));
	
	if ( (the_window = (Window*)f_calloc(1, sizeof(Window), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new Window", __func__ , __LINE__));
		goto error;
	}
	DEBUG_OUT(("%s %d:	__ALLOC__	the_window	%p	size	%i", __func__ , __LINE__, the_window, sizeof(Window)));

	if ( (the_window->title_ = General_StrlcpyWithAlloc(the_win_setup->title_, WINDOW_MAX_WINTITLE_SIZE)) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory for the window name string", __func__ , __LINE__));
		goto error;
	}
	DEBUG_OUT(("%s %d:	__ALLOC__	the_window->title_	%p	size	%i		'%s'", __func__ , __LINE__, the_window->title_, General_Strnlen(the_window->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, the_window->title_));
	
	// do check on the height, max height, min height, etc. 
	Window_CheckDimensions(the_window, the_win_setup);
	
	the_window->x_ = the_win_setup->x_;
	the_window->y_ = the_win_setup->y_;
	the_window->width_ = the_win_setup->width_;
	the_window->height_ = the_win_setup->height_;
	the_window->min_width_ = the_win_setup->min_width_;
	the_window->min_height_ = the_win_setup->min_height_;
	the_window->max_width_ = the_win_setup->max_width_;
	the_window->max_height_ = the_win_setup->max_height_;

	the_window->user_data_ = the_win_setup->user_data_;
	the_window->bitmap_ = the_win_setup->bitmap_;
	the_window->buffer_bitmap_ = the_win_setup->buffer_bitmap_;
	the_window->is_backdrop_ = the_win_setup->is_backdrop_;
	the_window->can_resize_ = the_win_setup->can_resize_;

	if (the_window->can_resize_)
	{
		// window can be resized, so needs drag zones to be established
		Window_ConfigureDragRects(the_window);
	}
	
	// set up the rects for titlebar, content, etc. 
	Window_ConfigureStructureRects(the_window);
	
	// all windows start off non-visible
	the_window->visible_ = false;
	
	// add the base controls
	
	DEBUG_OUT(("%s %d: getting theme and theme controls...", __func__, __LINE__));
		
	the_theme = Sys_GetCurrentTheme(global_system);

	close_control = Control_New(Theme_GetCloseControlTemplate(the_theme), the_window, CLOSE_WIDGET_ID, 0);
	minimize_control = Control_New(Theme_GetMinimizeControlTemplate(the_theme), the_window, MINIMIZE_WIDGET_ID, 0);
	normsize_control = Control_New(Theme_GetNormSizeControlTemplate(the_theme), the_window, NORM_SIZE_WIDGET_ID, 0);
	maximize_control = Control_New(Theme_GetMaximizeControlTemplate(the_theme), the_window, MAXIMIZE_WIDGET_ID, 0);
	
	DEBUG_OUT(("%s %d: close_control=%p", __func__, __LINE__, close_control));
	DEBUG_OUT(("%s %d: minimize_control=%p", __func__, __LINE__, minimize_control));
	DEBUG_OUT(("%s %d: normsize_control=%p", __func__, __LINE__, normsize_control));
	DEBUG_OUT(("%s %d: maximize_control=%p", __func__, __LINE__, maximize_control));
	
	the_window->root_control_ = close_control;

	Control_SetNextControl(close_control, minimize_control);
	Control_SetNextControl(minimize_control, normsize_control);
	Control_SetNextControl(normsize_control, maximize_control);
	Control_SetNextControl(maximize_control, NULL);
	
	return the_window;
	
error:
	if (the_window)					Window_Destroy(&the_window);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Window_Destroy(Window** the_window)
{
	if (*the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	if ((*the_window)->title_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	(*the_window)->title_	%p	size	%i		'%s'", __func__ , __LINE__, (*the_window)->title_, General_Strnlen((*the_window)->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, (*the_window)->title_));
		free((*the_window)->title_);
		(*the_window)->title_ = NULL;
	}
	
	LOG_ALLOC(("%s %d:	__FREE__	*the_window	%p	size	%i", __func__ , __LINE__, *the_window, sizeof(Window)));
	f_free(*the_window, MEM_STANDARD);
	*the_window = NULL;
	
	return true;
}





// **** Render functions *****

void Window_Render(Window* the_window)
{
	Theme*	the_theme;
	
	the_theme = Sys_GetCurrentTheme(global_system);

	// put up some drawings in the window to fake represent real window graphics 

	// draw basic boxes to represent window shape
	//Bitmap_FillBoxRect(the_window->bitmap_, &the_window->overall_rect_, SYS_COLOR_WHITE);
	Bitmap_DrawBoxRect(the_window->bitmap_, &the_window->overall_rect_, Theme_GetOutlineColor(the_theme));
	
	Bitmap_FillBoxRect(the_window->bitmap_, &the_window->titlebar_rect_, Theme_GetTitlebarColor(the_theme));
	//Bitmap_DrawBoxRect(the_window->bitmap_, &the_window->titlebar_rect_, 0xff);

 	Bitmap_FillBoxRect(the_window->bitmap_, &the_window->content_rect_, Theme_GetContentAreaColor(the_theme));
	//Bitmap_DrawBoxRect(the_window->bitmap_, &the_window->content_rect_, SYS_COLOR_GREEN1);

	// Draw window title
	Bitmap_SetCurrentColor(the_window->bitmap_, SYS_COLOR_WHITE);
	Bitmap_SetCurrentXY(the_window->bitmap_, the_window->titlebar_rect_.MinX + 25, the_window->titlebar_rect_.MinY + 4);

	if (Font_DrawString(the_window->bitmap_, the_window->title_, FONT_NO_STRLEN_CAP) == false)
	{
	}

	// draw some color blocks
	int i;
	int x = 1;
	int y = the_window->content_rect_.MinY;
	int height = 30;
	
	for (i=1; i<256; i++)
	{
		Bitmap_FillBox(the_window->bitmap_, x, y, 2, height, i);
		x += 2;
		
		if (i == 125)
		{
			x = 1;
			y += height;
		}
	}
	
	// ask controls to render themselves
	Control*	this_control;
	
	this_control = Window_GetRootControl(the_window);
	
	while (this_control)
	{
		this_control->enabled_ = true;
		this_control->visible_ = true;
		
		Control_Render(this_control);
		
		this_control = this_control->next_;
	}
	
}





// **** Set functions *****


bool Window_SetControlState(Window* the_window, uint16_t the_control_id);

// replace the current window title with the passed string
// Note: the passed string will be copied into storage by the window. The passing function can dispose of the passed string when done.
void Window_SetTitle(Window* the_window, char* the_title);






// **** Get functions *****

Control* Window_GetRootControl(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	return the_window->root_control_;
}


Control* Window_GetControl(Window* the_window, uint16_t the_control_id)
{
	Control*	the_control = NULL;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	the_control = Window_GetRootControl(the_window);
	
	while (the_control)
	{
		if (Control_GetID(the_control) == the_control_id)
		{
			return the_control;
		}
		
		the_control = the_control->next_;
	}
	
	return the_control;
}


uint16_t Window_GetControlID(Window* the_window, Control* the_control)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return Control_GetID(the_control);
}


// return the current window title
// Note: the window title is maintained by the window. Do not free the string pointer returned by this function!
char* Window_GetTitle(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	return the_window->title_;
}


uint32_t Window_GetUserData(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return 0;
	}
	
	return the_window->user_data_;
}


window_type Window_GetType(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return WIN_UNKNOWN_TYPE;
	}
	
	return the_window->type_;
}


window_state Window_GetState(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return WIN_UNKNOWN_STATE;
	}
	
	return the_window->state_;
}


Bitmap* Window_GetBitmap(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	return the_window->bitmap_;
}


signed int Window_GetX(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->x_;
}


signed int Window_GetY(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->y_;
}


signed int Window_GetWidth(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->width_;
}


signed int Window_GetHeight(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->height_;
}





// **** Set functions *****





// **** Get functions *****






// **** xxx functions *****




