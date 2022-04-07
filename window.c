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
void Window_CheckDimensions(Window* the_window, NewWinTemplate* the_win_template);

// draws or redraws the window controls
void Window_DrawControls(Window* the_window);
// draws or redraws the structure area of the windows (excluding the content area and controls)
void Window_DrawStructure(Window* the_window);
// draws or redraws the entire window, including clearing the content area
void Window_DrawAll(Window* the_window);


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
	the_window->titlebar_rect_.MaxX = the_window->width_ - 1;
	the_window->titlebar_rect_.MaxY = 19;

	the_window->content_rect_.MinX = 1;
	the_window->content_rect_.MinY = the_window->titlebar_rect_.MaxY + 1;
	the_window->content_rect_.MaxX = the_window->width_ - 1;
	the_window->content_rect_.MaxY = the_window->height_ - 1;
	the_window->inner_width_ = the_window->content_rect_.MaxX - the_window->content_rect_.MinX;
	the_window->inner_height_ = the_window->content_rect_.MaxY - the_window->content_rect_.MinY;

	the_window->overall_rect_.MinX = 0;
	the_window->overall_rect_.MinY = 0;
	the_window->overall_rect_.MaxX = the_window->width_ - 1;
	the_window->overall_rect_.MaxY = the_window->height_ - 1;
}


//! verify and adjust window width, height, etc, 
void Window_CheckDimensions(Window* the_window, NewWinTemplate* the_win_template)
{
	
	DEBUG_OUT(("%s %d: checking window dimensions...", __func__, __LINE__));

	if (the_win_template->min_width_ < WIN_DEFAULT_MIN_WIDTH)
	{
		the_win_template->min_width_ = WIN_DEFAULT_MIN_WIDTH;
	}
	
	if (the_win_template->min_height_ < WIN_DEFAULT_MIN_HEIGHT)
	{
		the_win_template->min_height_ = WIN_DEFAULT_MIN_HEIGHT;
	}
	
	if (the_win_template->max_width_ < WIN_DEFAULT_MIN_WIDTH)
	{
		the_win_template->max_width_ = WIN_DEFAULT_MIN_WIDTH;
	}
	
	if (the_win_template->max_height_ < WIN_DEFAULT_MIN_HEIGHT)
	{
		the_win_template->max_height_ = WIN_DEFAULT_MIN_HEIGHT;
	}
	
	if (the_win_template->width_ < the_win_template->min_width_)
	{
		the_win_template->width_ = the_win_template->min_width_;
	}
	
	if (the_win_template->height_ < the_win_template->min_height_)
	{
		the_win_template->height_ = the_win_template->min_height_;
	}
	
	if (the_win_template->width_ > the_win_template->max_width_)
	{
		the_win_template->width_ = the_win_template->max_width_;
	}
	
	if (the_win_template->height_ > the_win_template->max_height_)
	{
		the_win_template->height_ = the_win_template->max_height_;
	}

	if (!the_win_template->can_resize_)
	{
		the_win_template->min_width_ = the_win_template->width_;
		the_win_template->min_height_ = the_win_template->height_;
		the_win_template->max_width_ = the_win_template->width_;
		the_win_template->max_height_ = the_win_template->height_;
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
//! @param	the_win_template->width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH, cannot be greater than WIN_DEFAULT_MAX_WIDTH
//! @param	the_win_template->height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT, cannot be greater than WIN_DEFAULT_MAX_HEIGHT
//! @param	the_win_template->min_width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH
//! @param	the_win_template->min_height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT
//! @param	the_win_template->max_width_: cannot be smaller than WIN_DEFAULT_MIN_WIDTH
//! @param	the_win_template->max_height_: cannot be smaller than WIN_DEFAULT_MIN_HEIGHT
Window* Window_New(NewWinTemplate* the_win_template)
{
	Window*		the_window;
	Theme*		the_theme;
	Control*	close_control;
	Control*	minimize_control;
	Control*	normsize_control;
	Control*	maximize_control;
		
	// LOGIC: 
	//   Only a few parameter values can be so bad that the window creation process must terminate
	//   The offscreen bitmap can be NULL and be ok
	//   The main bitmap can also be NULL, and if so, a new one will be allocated
	//     If the new window is going to be a backdrop window, however, it will be assigned the system screen's bitmap
	//   The size parameters can be adjusted to match system mins and maxs.
	
	if ( the_win_template == NULL)
	{
		LOG_ERR(("%s %d: passed NewWinTemplate was NULL", __func__ , __LINE__));
		goto error;
	}
	
	DEBUG_OUT(("%s %d: x=%i, y=%i, width=%i", __func__, __LINE__, the_win_template->x_, the_win_template->y_, the_win_template->width_));
	
	if ( (the_window = (Window*)f_calloc(1, sizeof(Window), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new Window", __func__ , __LINE__));
		goto error;
	}
	DEBUG_OUT(("%s %d:	__ALLOC__	the_window	%p	size	%i", __func__ , __LINE__, the_window, sizeof(Window)));

	if ( (the_window->title_ = General_StrlcpyWithAlloc(the_win_template->title_, WINDOW_MAX_WINTITLE_SIZE)) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory for the window name string", __func__ , __LINE__));
		goto error;
	}
	DEBUG_OUT(("%s %d:	__ALLOC__	the_window->title_	%p	size	%i		'%s'", __func__ , __LINE__, the_window->title_, General_Strnlen(the_window->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, the_window->title_));

	// do check on the height, max height, min height, etc. 
	Window_CheckDimensions(the_window, the_win_template);

	// assign the bitmap passed by win_setup, or allocate a new one
	if ( the_win_template->bitmap_ == NULL)
	{
		if ( the_win_template->is_backdrop_)
		{
			if ( (the_window->bitmap_ = Sys_GetScreenBitmap(global_system, ID_CHANNEL_B)) == NULL)
			{
				LOG_ERR(("%s %d: Failed to acquire global screen bitmap for use with backdrop", __func__, __LINE__));
				goto error;
			}
		}
		else
		{
			if ( (the_window->bitmap_ = Bitmap_New(the_win_template->width_, the_win_template->height_, Sys_GetAppFont(global_system))) == NULL)
			{
				LOG_ERR(("%s %d: Failed to create bitmap", __func__, __LINE__));
				goto error;
			}
		}
	}
	else
	{
		the_window->bitmap_ = the_win_template->bitmap_;
	}
	
	the_window->x_ = the_win_template->x_;
	the_window->y_ = the_win_template->y_;
	the_window->width_ = the_win_template->width_;
	the_window->height_ = the_win_template->height_;
	the_window->min_width_ = the_win_template->min_width_;
	the_window->min_height_ = the_win_template->min_height_;
	the_window->max_width_ = the_win_template->max_width_;
	the_window->max_height_ = the_win_template->max_height_;

	the_window->user_data_ = the_win_template->user_data_;
	the_window->buffer_bitmap_ = the_win_template->buffer_bitmap_;
	the_window->is_backdrop_ = the_win_template->is_backdrop_;
	the_window->can_resize_ = the_win_template->can_resize_;

	if (the_window->can_resize_)
	{
		// window can be resized, so needs drag zones to be established
		Window_ConfigureDragRects(the_window);
	}
	
	// set up the rects for titlebar, content, etc. 
	Window_ConfigureStructureRects(the_window);
	
	// all windows start off non-visible
	the_window->visible_ = false;
	
	// all windows start out as invalidated, so that they are rendered fully next render pass
	the_window->invalidated_ = true;
	
	// pen location starts at 0,0 (relative to content area, not to window rect)
	the_window->pen_x_ = 0;
	the_window->pen_y_ = 0;
	
	// add the base controls (not needed for backdrops -- we don't want user to be able to close a backdrop)
	
	if ( the_win_template->is_backdrop_ == false)
	{
		DEBUG_OUT(("%s %d: getting theme and theme controls...", __func__, __LINE__));

		if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
		{
			LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
			goto error;
		}
		
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

		// set controls to active, except for the normal size one, because windows open at normal size
		Control_SetActive(close_control, CONTROL_ACTIVE);
		Control_SetActive(minimize_control, CONTROL_ACTIVE);
		Control_SetActive(normsize_control, CONTROL_INACTIVE);
		Control_SetActive(maximize_control, CONTROL_ACTIVE);
		
		// do first pass clear of the content area
		Window_ClearContent(the_window);
	}
	
	// Add this window to the list of windows
	Sys_AddToWindowList(global_system, the_window);
	
	//Window_Print(the_window);
		
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
		f_free((*the_window)->title_, MEM_STANDARD);
		(*the_window)->title_ = NULL;
	}
	
	LOG_ALLOC(("%s %d:	__FREE__	*the_window	%p	size	%i", __func__ , __LINE__, *the_window, sizeof(Window)));
	f_free(*the_window, MEM_STANDARD);
	*the_window = NULL;
	
	return true;
}


//! Allocate and populate a new window template object
//! Ensures that all fields have appropriate default values
//! Calling method must free this after creating a window with it. 
NewWinTemplate* Window_GetNewWinTemplate(char* the_win_title)
{
	NewWinTemplate*		the_win_template;
	
	if ( (the_win_template = (NewWinTemplate*)f_calloc(1, sizeof(NewWinTemplate), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new window template", __func__ , __LINE__));
		return NULL;
	}
	DEBUG_OUT(("%s %d:	__ALLOC__	the_win_template	%p	size	%i", __func__ , __LINE__, the_win_template, sizeof(NewWinTemplate)));

	the_win_template->user_data_ = 0L;
	the_win_template->title_ = the_win_title;
	the_win_template->type_ = WIN_STANDARD;
	the_win_template->bitmap_ = NULL;
	the_win_template->buffer_bitmap_ = NULL;
	the_win_template->is_backdrop_ = false;
	the_win_template->can_resize_ = true;
	the_win_template->x_ = WIN_DEFAULT_X;
	the_win_template->y_ = WIN_DEFAULT_Y;
	the_win_template->width_ = WIN_DEFAULT_WIDTH;
	the_win_template->height_ = WIN_DEFAULT_HEIGHT;
	the_win_template->min_width_ = WIN_DEFAULT_MIN_WIDTH;
	the_win_template->min_height_ = WIN_DEFAULT_MIN_HEIGHT;
	the_win_template->max_width_ = WIN_DEFAULT_MAX_WIDTH;
	the_win_template->max_height_ = WIN_DEFAULT_MAX_HEIGHT;

	return the_win_template;
}






// **** Render functions *****

void Window_Render(Window* the_window)
{
	Theme*	the_theme;
	Bitmap*	the_pattern;
	
	// LOGIC:
	//   From a rendering point of view, windows are split into "backdrop" and "not-backdrop" windows
	//   Backdrop windows always fill the screen and always are filled with their backdrop pattern and never have borders, controls, etc. 
	//   Non-backdrop windows are built up from overall struct, content area, and controls. 
	//     Except for the first render, the overall struct and content area are generally not cleared/re-rendered. 
	//     Controls are currently re-rendered each cycle
	
	the_theme = Sys_GetTheme(global_system);
	the_pattern = Theme_GetDesktopPattern(the_theme);

	if (the_window->is_backdrop_)
	{
		// backdrop window: fill it with its pattern. no controls, borders, etc. 
		// tile the default theme's background pattern
		Bitmap_Tile(the_pattern, 0, 0, the_window->bitmap_, 16, 16);
	}
	else
	{
		// non-backdrop window: render borders, titlebars, controls, etc, as appropriate

		if (the_window->invalidated_ == true)
		{
			Window_DrawStructure(the_window);
		}

		// either way, we are currently re-rendering all controls until damage regions/etc are available.
		Window_DrawControls(the_window);
	}
}


// draws or redraws the entire window, including clearing the content area
void Window_DrawAll(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	Window_DrawStructure(the_window);
	Window_ClearContent(the_window);
	Window_DrawControls(the_window);
}


// draws or redraws the structure area of the windows (excluding the content area and controls)
void Window_DrawStructure(Window* the_window)
{
	Theme*	the_theme;
	Font*	old_font;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	the_theme = Sys_GetTheme(global_system);

	Bitmap_DrawBoxRect(the_window->bitmap_, &the_window->overall_rect_, Theme_GetOutlineColor(the_theme));

	Bitmap_FillBoxRect(the_window->bitmap_, &the_window->titlebar_rect_, Theme_GetTitlebarColor(the_theme));

	// TODO: refactor this into it's own function, and have real way of calculating the font baseline position
	// Draw window title with system (app) font, being careful to reset to whatever font was before.
	old_font = Bitmap_GetFont(the_window->bitmap_);

	if (Bitmap_SetFont(the_window->bitmap_, Sys_GetSystemFont(global_system)) == false)
	{
		DEBUG_OUT(("%s %d: Couldn't get the system font and assign it to bitmap", __func__, __LINE__));
		Sys_Destroy(&global_system);	// crash early, crash often
	}
	
	Bitmap_SetColor(the_window->bitmap_, SYS_COLOR_WHITE);
	Bitmap_SetXY(the_window->bitmap_, the_window->titlebar_rect_.MinX + 25, the_window->titlebar_rect_.MinY + 2);

	if (Font_DrawString(the_window->bitmap_, the_window->title_, FONT_NO_STRLEN_CAP) == false)
	{
	}

	if (Bitmap_SetFont(the_window->bitmap_, old_font) == false)
	{
		DEBUG_OUT(("%s %d: Couldn't set the bitmap's font back to what it had been", __func__, __LINE__));
		Sys_Destroy(&global_system);	// crash early, crash often
	}
}


// draws or redraws the window controls
void Window_DrawControls(Window* the_window)
{
	Control*	this_control;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	this_control = Window_GetRootControl(the_window);

	while (this_control)
	{
		this_control->enabled_ = true;
		this_control->visible_ = true;
	
		Control_Render(this_control);
	
		this_control = this_control->next_;
	}
}


// clears the content area rect, setting it to the theme's backcolor
void Window_ClearContent(Window* the_window)
{
	Theme*	the_theme;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	the_theme = Sys_GetTheme(global_system);

	Bitmap_FillBoxRect(the_window->bitmap_, &the_window->content_rect_, Theme_GetContentAreaColor(the_theme));
}




// **** Set functions *****


bool Window_SetControlState(Window* the_window, uint16_t the_control_id);

// replace the current window title with the passed string
// Note: the passed string will be copied into storage by the window. The passing function can dispose of the passed string when done.
void Window_SetTitle(Window* the_window, char* the_title);



//! Set the passed window's visibility flag.
//! This does not immediately cause the window to render. The window will be rendered on the next system rendering pass.
void Window_SetVisible(Window* the_window, bool is_visible)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	the_window->visible_ = is_visible;
}




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


// Get backdrop yes/no flag. returns true if backdrop, false if not
bool Window_IsBackdrop(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	return the_window->is_backdrop_;
}


// Get visible yes/no flag. returns true if window should be rendered, false if not
bool Window_IsVisible(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	return the_window->visible_;
}





// **** DRAWING functions *****

//! Set the font
//! This is the font that will be used for any subsequent font drawing in this Window
//! This also sets the font of the window's bitmap
//! This only affects programmer-controlled drawing actions; it will not change the title bar font, the icon font, etc. Those are controlled by the theme.
//! @param	the_window: reference to a valid Window object.
//! @param	the_font: reference to a complete, loaded Font object.
//! @return Returns false on any error condition
bool Window_SetFont(Window* the_window, Font* the_font)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	if (the_font == NULL)
	{
		LOG_WARN(("%s %d: passed font was null", __func__ , __LINE__));
		return false;
	}
	
	the_window->pen_font_ = the_font;
	the_window->bitmap_->font_ = the_font;
}


//! Set the "pen" color
//! This is the color that the next pen-based graphics function will use
//! This also sets the pen color of the window's bitmap
//! This only affects functions that use the pen: any graphics function that specifies a color will use that instead
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return Returns false on any error condition
bool Window_SetColor(Window* the_window, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	the_window->pen_color_ = the_color;
	the_window->bitmap_->color_ = the_color;
	
	return true;
}


//! Set the "pen" position within the content area
//! This also sets the pen position of the window's bitmap
//! This is the location that the next pen-based graphics function will use for a starting location
//! @param	the_window: reference to a valid Window object.
//! @param	x: the horizontal position within the content area of the window. Will be clipped to the edges.
//! @param	y: the vertical position within the content area of the window. Will be clipped to the edges.
//! @return Returns false on any error condition
bool Window_SetPenXY(Window* the_window, signed int x, signed int y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	if (x < 0)
	{
		x = 0;
	}
	
	if (y < 0)
	{
		y = 0;
	}
	
	if (x > the_window->content_rect_.MaxX)
	{
		x = the_window->content_rect_.MaxX;
	}
	
	if (y > the_window->content_rect_.MaxY)
	{
		y = the_window->content_rect_.MaxY;
	}
	
	the_window->pen_x_ = x;
	the_window->pen_y_ = y;
	the_window->bitmap_->x_ = x + the_window->content_rect_.MinX;
	the_window->bitmap_->y_ = y + the_window->content_rect_.MinY;;
	
	return true;
}


//! Fill a rectangle drawn from the current pen location, for the passed width/height
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_FillBox(Window* the_window, signed int width, signed int height, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_FillBox(the_window->bitmap_, the_window->x_, the_window->y_, width, height, the_color);
}


//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_FillBoxRect(Window* the_window, Rectangle* the_coords, unsigned char the_color)
{
	signed int	x1;
	signed int	y1;
	signed int	x2;
	signed int	y2;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	// localize to content area
	x1 = the_coords->MinX + the_window->content_rect_.MinX;
	y1 = the_coords->MinY + the_window->content_rect_.MinY;
	x2 = the_coords->MaxX + the_window->content_rect_.MinX;
	y2 = the_coords->MaxY + the_window->content_rect_.MinY;
	
	return Bitmap_FillBox(the_window->bitmap_, x1, y1, x2 - x1, y2 - y1, the_color);
}


//! Set the color of the pixel at the current pen location
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_SetPixel(Window* the_window, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_SetPixelAtXY(the_window->bitmap_, the_window->x_, the_window->y_, the_color);
}


//! Draws a line between 2 passed coordinates.
//! Use for any line that is not perfectly vertical or perfectly horizontal
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! Based on http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C. Used in C128 Lich King. 
bool Window_DrawLine(Window* the_window, signed int x1, signed int y1, signed int x2, signed int y2, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	// localize to content area
	x1 += the_window->content_rect_.MinX;
	y1 += the_window->content_rect_.MinY;
	x2 += the_window->content_rect_.MinX;
	y2 += the_window->content_rect_.MinY;
	
	return Bitmap_DrawLine(the_window->bitmap_, x1, y1, x2, y2, the_color);
}


//! Draws a horizontal line from the current pen location, for n pixels, using the specified pixel value
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawHLine(Window* the_window, signed int the_line_len, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawHLine(the_window->bitmap_, the_window->x_, the_window->y_, the_line_len, the_color);
}


//! Draws a vertical line from specified coords, for n pixels
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawVLine(Window* the_window, signed int the_line_len, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawVLine(the_window->bitmap_, the_window->x_, the_window->y_, the_line_len, the_color);
}


//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawBoxRect(Window* the_window, Rectangle* the_coords, unsigned char the_color)
{
	signed int	x1;
	signed int	y1;
	signed int	x2;
	signed int	y2;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	// localize to content area
	x1 = the_coords->MinX + the_window->content_rect_.MinX;
	y1 = the_coords->MinY + the_window->content_rect_.MinY;
	x2 = the_coords->MaxX + the_window->content_rect_.MinX;
	y2 = the_coords->MaxY + the_window->content_rect_.MinY;
	
	return Bitmap_DrawBoxCoords(the_window->bitmap_, x1, y1, x2, y2, the_color);
}


//! Draws a rectangle based on 2 sets of coords, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawBoxCoords(Window* the_window, signed int x1, signed int y1, signed int x2, signed int y2, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	// localize to content area
	x1 += the_window->content_rect_.MinX;
	y1 += the_window->content_rect_.MinY;
	x2 += the_window->content_rect_.MinX;
	y2 += the_window->content_rect_.MinY;
	
	return Bitmap_DrawBoxCoords(the_window->bitmap_, x1, y1, x2, y2, the_color);
}


//! Draws a rectangle based on start coords and width/height, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Window_DrawBox(Window* the_window, signed int width, signed int height, unsigned char the_color, bool do_fill)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawBox(the_window->bitmap_, the_window->x_, the_window->y_, width, height, the_color, do_fill);
}


//! Draws a rounded rectangle from the current pen location, with the specified size and radius, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Window_DrawRoundBox(Window* the_window, signed int width, signed int height, signed int radius, unsigned char the_color, bool do_fill)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawRoundBox(the_window->bitmap_, the_window->x_, the_window->y_, width, height, radius, the_color, do_fill);
}


//! Draw a circle centered on the current pen location
//! Based on http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
//! @param	the_window: reference to a valid Window object.
bool Window_DrawCircle(Window* the_window, signed int radius, unsigned char the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawCircle(the_window->bitmap_, the_window->x_, the_window->y_, radius, the_color);
}


// Draw a string at the current "pen" location, using the current pen color of the Window
// Truncate, but still draw the string if it is too long to display on the line it started.
// No word wrap is performed. 
// If max_chars is less than the string length, only that many characters will be drawn (as space allows)
// If max_chars is -1, then the full string length will be drawn, as space allows.
bool Window_DrawString(Window* the_window, char* the_string, signed int max_chars)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Font_DrawString(the_window->bitmap_, the_string, max_chars);
}


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
char* Window_DrawStringInBox(Window* the_window, signed int width, signed int height, char* the_string, signed int num_chars, char** wrap_buffer, bool (* continue_function)(void))
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	// the next routine will check if it fits in the bitmap, but won't check if it fits within the window's content area
	if (the_window->pen_x_ + width > the_window->inner_width_)
	{
		width -= the_window->inner_width_ - the_window->pen_x_;
	}
	
	if (the_window->pen_y_ + height > the_window->inner_height_)
	{
		height -= the_window->inner_height_ - the_window->pen_y_;
	}
	
	return Font_DrawStringInBox(the_window->bitmap_, width, height, the_string, num_chars, wrap_buffer, continue_function);
}




// **** Get functions *****






// **** xxx functions *****




