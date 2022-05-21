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
#include "a2560_platform.h"
#include "general.h"
#include "bitmap.h"
#include "text.h"
#include "font.h"
#include "lib_sys.h"


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

// **** Private CONFIGURATION functions *****

//! set or update the drag rects on the four edges of the screen
//! Call this after every window resize event
void Window_ConfigureDragRects(Window* the_window);

//! set up the rects for titlebar, content, etc. 
void Window_ConfigureStructureRects(Window* the_window);

//! verify and adjust window width, height, etc, 
void Window_CheckDimensions(Window* the_window, NewWinTemplate* the_win_template);

//! Updates the current window controls with control template info from the current system theme 
void Window_UpdateControlTheme(Window* the_window);

//! Determine available pixel width for the title, based on theme's title x offset and left-most control
//! Run after every re-size, or after a theme change
//! Note: this returns the result space and sets the window's avail_title_width_ property. It does not force any re-rendering.
//! @return:	Returns -1 in event of error, or the calculated width
static int16_t Window_CalculateTitleSpace(Window* the_window);



// **** Private CONTROL management functions *****

//! Get the first, or root, control object for the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a control pointer, or NULL on any error, or if there is no root control
Control* Window_GetRootControl(Window* the_window);



	
// **** Private RENDER functions *****

// draws or redraws the structure area of the windows (excluding the content area and controls)
static void Window_DrawStructure(Window* the_window);

// draws or redraws the entire window, including clearing the content area
static void Window_DrawAll(Window* the_window);

// draws or redraws the window controls
// if force_redraw is false, only controls that have been invalidated will re-draw.
static void Window_DrawControls(Window* the_window, bool force_redraw);

// draws or redraws the titlebar area
static void Window_DrawTitlebar(Window* the_window);

//! Draws the title text into the titlebar using the active theme's system font
//! @param	the_window: a valid pointer to a Window
static void Window_DrawTitle(Window* the_window);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


// **** Private CONFIGURATION functions *****

//! set or update the drag rects on the four edges of the screen
//! Call this after every window resize event
void Window_ConfigureDragRects(Window* the_window)
{
	int16_t		upper_start;
	int16_t		lower_end;
	
	// LOGIC:
	//   windows have 6 possible drag zones: titlebar, left side, right side, top, bottom, lower-right corner.
	//   the title_drag_rect_ is for dragging the window, the others are for resizing the window
	//     the position of the title drag rect is not set here: it is set in Window_CalculateTitleSpace()
	//     as the position needs to change not only on window resize, but on theme change (controls may move)
	//   in order to not have mouse clicks aimed at closing/minimizing/etc the window, we have to 
	//     take the position of the title bar into account, and limit left/right drag zones to avoid it
	//     comparing the top of the title rect to top of window overall rect is informative about top vs bottom positioning
	
	if (the_window->titlebar_rect_.MinY == the_window->overall_rect_.MinY)
	{
		upper_start = the_window->titlebar_rect_.MaxY;
		lower_end = the_window->overall_rect_.MaxY - 2; // a pixel or two above bottom of window is good
	}
	else
	{
		upper_start = the_window->overall_rect_.MinY + 2; // a pixel or two below top of window is good
		lower_end = the_window->titlebar_rect_.MinY;
	}

	DEBUG_OUT(("%s %d: upper_start=%i, lower_end=%i", __func__ , __LINE__, upper_start, lower_end));

	the_window->grow_top_rect_.MinX = 0;
	the_window->grow_top_rect_.MinY = 0;
	the_window->grow_top_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_top_rect_.MaxY = WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	
	the_window->grow_left_rect_.MinX = 0;
	the_window->grow_left_rect_.MinY = upper_start;
	the_window->grow_left_rect_.MaxX = WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_left_rect_.MaxY = lower_end;

	the_window->grow_right_rect_.MinX = the_window->width_ - WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_right_rect_.MinY = upper_start;
	the_window->grow_right_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_right_rect_.MaxY = lower_end - 15;	// cutout at bottom for the traditional lower/right corner drag zone

	the_window->grow_bottom_rect_.MinX = 0;
	the_window->grow_bottom_rect_.MinY = the_window->height_ - WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_bottom_rect_.MaxX = the_window->width_ - 15;	// cutout at right for the traditional lower/right corner drag zone
	the_window->grow_bottom_rect_.MaxY = the_window->height_ - 1;
	
	the_window->grow_bottom_right_rect_.MinX = the_window->grow_bottom_rect_.MaxX + 1;
	the_window->grow_bottom_right_rect_.MinY = the_window->grow_right_rect_.MaxY + 1;
	the_window->grow_bottom_right_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_bottom_right_rect_.MaxY = the_window->height_ - 1;	
}



//! set up the rects for titlebar, content, etc. 
void Window_ConfigureStructureRects(Window* the_window)
{
	Theme*		the_theme;
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		goto error;
	}

	the_window->overall_rect_.MinX = 0;
	the_window->overall_rect_.MinY = 0;
	the_window->overall_rect_.MaxX = the_window->width_ - 1;
	the_window->overall_rect_.MaxY = the_window->height_ - 1;

	// LOGIC for BACKDROP windows:
	//   Backdrop windows do not have titlebars, iconbars: they are just one big content area. 
	//   So they override rect guidance from theme on that point. 
	
	if (the_window->is_backdrop_)
	{
		the_window->titlebar_rect_.MinX = 0;
		the_window->titlebar_rect_.MaxX = 0;
		the_window->titlebar_rect_.MinY = 0;
		the_window->titlebar_rect_.MaxY = 0;
		the_window->iconbar_rect_.MinX = 1;
		the_window->iconbar_rect_.MaxX = 1;
		the_window->iconbar_rect_.MinY = 0;
		the_window->iconbar_rect_.MaxY = 0;
		the_window->content_rect_.MinY = the_window->overall_rect_.MinY;
		the_window->content_rect_.MaxY = the_window->overall_rect_.MaxY;
		the_window->content_rect_.MinX = the_window->overall_rect_.MinX;
		the_window->content_rect_.MaxX = the_window->overall_rect_.MaxX;
	}
	else
	{
		// LOGIC for VERTICAL placement of title bar, icon bar, and content area:
		//   if theme's flow_from_bottom_ property is true, the vertical flow of elements from top will be content area->iconbar->titlebar
		//     if false, the vertical flow of elements from top will be titlebar->iconbar->content area
	
		// LOGIC for titlebar placement:
		//   do NOT inset the titlebar vertically or horizontally by one. Titlebar must be allowed to overwrite the overall window border
	
		if (the_theme->flow_from_bottom_ == false)
		{
			the_window->titlebar_rect_.MinY = the_window->overall_rect_.MinY;
		}
		else
		{
			the_window->titlebar_rect_.MinY = the_window->overall_rect_.MaxY - the_theme->titlebar_height_;
		}
	
		the_window->titlebar_rect_.MaxY = the_window->titlebar_rect_.MinY + the_theme->titlebar_height_;
		the_window->titlebar_rect_.MinX = 0;
		the_window->titlebar_rect_.MaxX = the_window->width_ - 1;
	

		// LOGIC for iconbar placement:
		//   if iconbar is set to not show, then do not allocate any vertical space for it.
		//     Let it overlap the titlebar area exactly so that content area can position itself always relative to iconbar.
		//   DO inset the iconbar vertically by one from the title. Iconbar must not be allowed to overwrite the titlebar border
		//   Do NOT inset the iconbar horizontally by one from the window edge. Iconbar must be allowed to overwrite the overall window border
		if (the_window->show_iconbar_)
		{
			if (the_theme->flow_from_bottom_ == false)
			{
				the_window->iconbar_rect_.MinY = the_window->titlebar_rect_.MaxY + 1;
			}
			else
			{
				the_window->iconbar_rect_.MinY = the_window->titlebar_rect_.MinY - 1 - the_theme->iconbar_height_;
			}

			the_window->iconbar_rect_.MaxY = the_window->iconbar_rect_.MinY + the_theme->iconbar_height_;
			the_window->iconbar_rect_.MinX = 1;							// inset iconbar by 1 > it does not need to overwrite outer border
			the_window->iconbar_rect_.MaxX = the_window->width_ - 1;	// inset iconbar by 1 > it does not need to overwrite outer border
		}
		else
		{
			the_window->iconbar_rect_.MinX = the_window->titlebar_rect_.MaxX;
			the_window->iconbar_rect_.MaxX = the_window->titlebar_rect_.MaxX;
			the_window->iconbar_rect_.MinY = the_window->titlebar_rect_.MinY;
			the_window->iconbar_rect_.MaxY = the_window->titlebar_rect_.MaxY;
		}
	

		// LOGIC for content area placement:
		//   DO inset the iconbar vertically and horizontally by one from the window edge and titlebar/iconbar: content area must not be allowed to overwrite any borders
		if (the_theme->flow_from_bottom_ == false)
		{
			the_window->content_rect_.MinY = the_window->iconbar_rect_.MaxY + 1;
			the_window->content_rect_.MaxY = the_window->overall_rect_.MaxY - 1;
		}
		else
		{
			the_window->content_rect_.MinY = the_window->overall_rect_.MinY + 1;
			the_window->content_rect_.MaxY = the_window->iconbar_rect_.MinY - 1;
		}

		the_window->content_rect_.MinX = 1;
		the_window->content_rect_.MaxX = the_window->width_ - 1;
	}
	

	the_window->inner_width_ = the_window->content_rect_.MaxX - the_window->content_rect_.MinX;
	the_window->inner_height_ = the_window->content_rect_.MaxY - the_window->content_rect_.MinY;

	//DEBUG_OUT(("%s %d: Window after configure struct rects...", __func__, __LINE__));
	//Window_Print(the_window);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! verify and adjust window width, height, etc, 
void Window_CheckDimensions(Window* the_window, NewWinTemplate* the_win_template)
{
	
	//DEBUG_OUT(("%s %d: checking window dimensions...", __func__, __LINE__));

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


//! Updates the current window controls with control template info from the current system theme 
void Window_UpdateControlTheme(Window* the_window)
{
	Theme*				the_theme;
	Control*			the_control = NULL;
	ControlTemplate*	the_template;
	
	// LOGIC:
	//   old controls will not be destroyed and recreated
	//   the parts of the control that could have been affected by the theme are simply adjusted
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		goto error;
	}

	DEBUG_OUT(("%s %d: updating controls based on current system theme...", __func__, __LINE__));

	the_control = Window_GetRootControl(the_window);
	
	while (the_control)
	{
		if (the_control->id_ == CLOSE_WIDGET_ID)
		{
			the_template = Theme_GetCloseControlTemplate(the_theme);
		}
		else if (the_control->id_ == MINIMIZE_WIDGET_ID)
		{
			the_template = Theme_GetMinimizeControlTemplate(the_theme);
		}
		else if (the_control->id_ == NORM_SIZE_WIDGET_ID)
		{
			the_template = Theme_GetNormSizeControlTemplate(the_theme);
		}
		else if (the_control->id_ == MAXIMIZE_WIDGET_ID)
		{
			the_template = Theme_GetMaximizeControlTemplate(the_theme);
		}
		else if (the_control->type_ == TEXT_BUTTON)
		{
			int16_t		new_height = the_theme->flex_width_backdrops_[TEXT_BUTTON].height_;
			
			the_control->height_ = new_height; // each theme controls the height of expandable controls, and it may differ from what control had previously.
			
			if ( (the_template = Theme_CreateControlTemplateFlexWidth(the_theme, TEXT_BUTTON, the_control->width_, the_control->height_, the_control->x_offset_, the_control->y_offset_, the_control->h_align_, the_control->v_align_, the_control->caption_)) == NULL)
			{
				LOG_ERR(("%s %d: Failed to create the control template", __func__, __LINE__));
				return;
			}
		}
		
		Control_UpdateFromTemplate(the_control, the_template);
	
		the_control = the_control->next_;
	}
	
	// TODO: maybe add a Theme_GetXXControl() function that takes one of the widget IDs. 
	// that could let above just be iteration. 

	// TODO: once other control types are defined (textfield, textbutton, etc,) add calls to update from template here....
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Determine available pixel width for the title, based on theme's title x offset and left-most control
//! Run after every re-size, or after a theme change
//! Also sets the size of the title bar drag rect
//! Note: this returns the result space and sets the window's avail_title_width_ property. It does not force any re-rendering.
//! @return:	Returns -1 in event of error, or the calculated width
static int16_t Window_CalculateTitleSpace(Window* the_window)
{
	Theme*		the_theme;
	Control*	the_control = NULL;
	int16_t		lowest_left = -1;
	int16_t		lowest_right = -1;
	int16_t		this_left;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   Controls could be arranged in all kinds of ways
	//   The theme though is required to set the left-most drawing point for the title text
	//   Whichever control is the closest to the RIGHT of that position will represent the end of usable text
	
	lowest_left = the_window->titlebar_rect_.MaxX;
	
	the_control = Window_GetRootControl(the_window);
	
	while (the_control)
	{
		switch (Control_GetType(the_control))
		{
			case CLOSE_WIDGET:
			case SIZE_MINIMIZE:
			case SIZE_NORMAL:
			case SIZE_MAXIMIZE:
				//DEBUG_OUT(("before: lowest_left=%i, control MinX=%i; lowest_right=%i, control MaxX=%i", lowest_left, the_control->rect_.MinX, lowest_right, the_control->rect_.MaxX));
				this_left = lowest_left;
				
				if (Control_IsLefter(the_control, &lowest_left))
				{
					// left got adjusted. but was that control always left of the starting point?
					if (lowest_left < the_theme->title_x_offset_)
					{
						lowest_left = this_left;
					}
				}
				//DEBUG_OUT(("after: lowest_left=%i, control MinX=%i; lowest_right=%i, control MaxX=%i", lowest_left, the_control->rect_.MinX, lowest_right, the_control->rect_.MaxX));
			
				break;

			default:
				break;
		}
		
		the_control = the_control->next_;
	}
	
	// were all controls to the one side or the other? 
	the_window->avail_title_width_ = lowest_left - the_theme->title_x_offset_;
//	DEBUG_OUT(("after: the_window->avail_title_width_=%i", the_window->avail_title_width_));
		
	// set the title_drag_rect_ to match the available space
	the_window->title_drag_rect_.MinX = the_theme->title_x_offset_ - 5;
	the_window->title_drag_rect_.MinY = the_window->titlebar_rect_.MinY;
	the_window->title_drag_rect_.MaxX = lowest_left - 5;
	the_window->title_drag_rect_.MaxY = the_window->titlebar_rect_.MaxY;

	return the_window->avail_title_width_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}





// **** Private CONTROL management functions *****



//! Get the first, or root, control object for the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a control pointer, or NULL on any error, or if there is no root control
Control* Window_GetRootControl(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->root_control_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}





// **** Private RENDER functions *****


// draws or redraws the structure area of the windows (excluding the content area and controls)
static void Window_DrawStructure(Window* the_window)
{
	Theme*	the_theme;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	// LOGIC:
	//   order of rect rendering must be Fill titlebar > Draw overall border rect > draw titlebar rect
	//   this allows titlebar to overwrite the overall border when it has one, but not have its fill overwrite the overall border if no titlebar border
	
	the_theme = Sys_GetTheme(global_system);

	if (the_window->titlebar_invalidated_ == true)
	{
		Window_DrawTitlebar(the_window);
	}

	Bitmap_DrawBoxRect(the_window->bitmap_, &the_window->overall_rect_, Theme_GetOutlineColor(the_theme));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// draws or redraws the entire window, including clearing the content area
// forces redraw of all controls, by marking them invalidated
static void Window_DrawAll(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	Window_DrawStructure(the_window);
	Window_ClearContent(the_window);
	Window_DrawControls(the_window, WIN_PARAM_FORCE_CONTROL_REDRAW);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// draws or redraws the window controls
// if force_redraw is false, only controls that have been invalidated will re-draw.
static void Window_DrawControls(Window* the_window, bool force_redraw)
{
	Control*	this_control;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	this_control = Window_GetRootControl(the_window);

	while (this_control)
	{
		this_control->enabled_ = true;
		this_control->visible_ = true;
	
		if (force_redraw)
		{
			this_control->invalidated_ = true;
		}
		
		if (this_control->invalidated_)
		{
			Control_Render(this_control);
		}
		
		this_control = this_control->next_;
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// draws or redraws the titlebar area, including close/min/max/normal size controls
static void Window_DrawTitlebar(Window* the_window)
{
	Theme*	the_theme;
	int16_t	i;

	// LOGIC:
	//   not checking for valid window, because this is only called by Window_DrawStructure, and it checks validity

	// LOGIC:
	//   order of rect rendering must be Fill titlebar > Draw overall border rect > draw titlebar rect
	//   this allows titlebar to overwrite the overall border when it has one, but not have its fill overwrite the overall border if no titlebar border
	
	the_theme = Sys_GetTheme(global_system);

	if (the_window->active_)
	{
		Bitmap_FillBoxRect(the_window->bitmap_, &the_window->titlebar_rect_, Theme_GetTitlebarColor(the_theme));
	}
	else
	{
		Bitmap_FillBoxRect(the_window->bitmap_, &the_window->titlebar_rect_, Theme_GetInactiveBackColor(the_theme));
	}

	if (the_theme->titlebar_outline_)
	{
		Bitmap_DrawBoxRect(the_window->bitmap_, &the_window->titlebar_rect_, Theme_GetOutlineColor(the_theme));
	}

	Window_DrawTitle(the_window);
	
	// no longer invalidated
	the_window->titlebar_invalidated_ = false;

	// add titlebar rect to the list of clip rects that need to be blitted from window to screen
	Window_AddClipRect(the_window, &the_window->titlebar_rect_);
	
	// mark the controls positioned in the title bar as invalid so they get redrawn as well
	for (i=CLOSE_WIDGET_ID; i < MAX_BUILT_IN_WIDGET; i++)
	{
		Control*	the_control;
		
		the_control = Window_GetControl(the_window, i);
		
		if (the_control != NULL)
		{
			Control_MarkInvalidated(the_control, true);
			Control_SetActive(the_control, the_window->active_);
		}
	}
}


//! Draws the title text into the titlebar using the active theme's system font
//! @param	the_window: a valid pointer to a Window
static void Window_DrawTitle(Window* the_window)
{
	Theme*		the_theme;
	Font*		new_font;
	Font*		old_font;
	int16_t		available_width;
	int16_t		chars_that_fit;
	int16_t		pixels_used;

	// LOGIC:
	//   not checking for valid window, because this is only called by Window_DrawStructure->Window_DrawTitlebar, and it checks validity

	// Draw window title with system (app) font, being careful to reset to whatever font was before.
	// title is to be drawn centered vertically within the titlebar Rect
	// title is to be clipped if too long to fit horizontally

	the_theme = Sys_GetTheme(global_system);
	new_font = Sys_GetSystemFont(global_system);
	old_font = Bitmap_GetFont(the_window->bitmap_);

	if (Bitmap_SetFont(the_window->bitmap_, new_font) == false)
	{
		DEBUG_OUT(("%s %d: Couldn't get the system font and assign it to bitmap", __func__, __LINE__));
		goto error;
	}
	
	available_width = the_window->avail_title_width_;
	chars_that_fit = Font_MeasureStringWidth(new_font, the_window->title_, GEN_NO_STRLEN_CAP, available_width, 0, &pixels_used);
	//DEBUG_OUT(("%s %d: available_width=%i, chars_that_fit=%i, title='%s', pixels_used=%i", __func__, __LINE__, available_width, chars_that_fit, the_window->title_, pixels_used));
	
	if (the_window->active_)
	{
		Bitmap_SetColor(the_window->bitmap_, Theme_GetTitleColor(the_theme));
	}
	else
	{
		Bitmap_SetColor(the_window->bitmap_, Theme_GetInactiveForeColor(the_theme));
	}
	
	Bitmap_SetXY(the_window->bitmap_, the_window->titlebar_rect_.MinX + the_theme->title_x_offset_, the_window->titlebar_rect_.MinY + (new_font->fRectHeight - new_font->ascent) );

	if (Font_DrawString(the_window->bitmap_, the_window->title_, chars_that_fit) == false)
	{
	}

	if (Bitmap_SetFont(the_window->bitmap_, old_font) == false)
	{
		DEBUG_OUT(("%s %d: Couldn't set the bitmap's font back to what it had been", __func__, __LINE__));
		goto error;
	}

	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
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
	DEBUG_OUT(("  titlebar_rect_: %i, %i, %i, %i", the_window->titlebar_rect_.MinX, the_window->titlebar_rect_.MinY, the_window->titlebar_rect_.MaxX, the_window->titlebar_rect_.MaxY));
	DEBUG_OUT(("  iconbar_rect_: %i, %i, %i, %i", the_window->iconbar_rect_.MinX, the_window->iconbar_rect_.MinY, the_window->iconbar_rect_.MaxX, the_window->iconbar_rect_.MaxY));
	DEBUG_OUT(("  content_rect_: %i, %i, %i, %i", the_window->content_rect_.MinX, the_window->content_rect_.MinY, the_window->content_rect_.MaxX, the_window->content_rect_.MaxY));
	DEBUG_OUT(("  grow_left_rect_: %i, %i, %i, %i", the_window->grow_left_rect_.MinX, the_window->grow_left_rect_.MinY, the_window->grow_left_rect_.MaxX, the_window->grow_left_rect_.MaxY));
	DEBUG_OUT(("  grow_right_rect_: %i, %i, %i, %i", the_window->grow_right_rect_.MinX, the_window->grow_right_rect_.MinY, the_window->grow_right_rect_.MaxX, the_window->grow_right_rect_.MaxY));
	DEBUG_OUT(("  grow_top_rect_: %i, %i, %i, %i", the_window->grow_top_rect_.MinX, the_window->grow_top_rect_.MinY, the_window->grow_top_rect_.MaxX, the_window->grow_top_rect_.MaxY));
	DEBUG_OUT(("  grow_bottom_rect_: %i, %i, %i, %i", the_window->grow_bottom_rect_.MinX, the_window->grow_bottom_rect_.MinY, the_window->grow_bottom_rect_.MaxX, the_window->grow_bottom_rect_.MaxY));
	DEBUG_OUT(("  show_iconbar_: %i",	the_window->show_iconbar_));
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
	DEBUG_OUT(("  avail_title_width_: %i",	the_window->avail_title_width_));	
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


// helper function called by List class's print function: prints one window entry
void Window_PrintBrief(void* the_payload)
{
	Window*		this_window = (Window*)(the_payload);

	DEBUG_OUT(("%s %d: %s (%p, %i)", __func__ , __LINE__, (this_window->is_backdrop_ == false ? this_window->title_ : (char*)"Backdrop"), this_window, this_window->display_order_ ));

}


/*****************************************************************************/
/*                        Public Function Definitions                        */
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
Window* Window_New(NewWinTemplate* the_win_template, void (* event_handler)(EventRecord*))
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
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		goto error;
	}
	
	DEBUG_OUT(("%s %d: x=%i, y=%i, width=%i", __func__, __LINE__, the_win_template->x_, the_win_template->y_, the_win_template->width_));
	
	if ( (the_window = (Window*)calloc(1, sizeof(Window)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new Window", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_window	%p	size	%i", __func__ , __LINE__, the_window, sizeof(Window)));

	if ( (the_window->title_ = General_StrlcpyWithAlloc(the_win_template->title_, WINDOW_MAX_WINTITLE_SIZE)) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory for the window name string", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_window->title_	%p	size	%i		'%s'", __func__ , __LINE__, the_window->title_, General_Strnlen(the_window->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, the_window->title_));

	// do check on the height, max height, min height, etc. 
	Window_CheckDimensions(the_window, the_win_template);

	// assign the bitmap passed by win_setup, or allocate a new one
	if ( the_win_template->bitmap_ == NULL)
	{
		if ( (the_window->bitmap_ = Bitmap_New(the_win_template->width_, the_win_template->height_, Sys_GetAppFont(global_system), PARAM_NOT_IN_VRAM)) == NULL)
		{
			LOG_ERR(("%s %d: Failed to create bitmap", __func__, __LINE__));
			goto error;
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
	the_window->norm_x_ = the_win_template->x_;
	the_window->norm_y_ = the_win_template->y_;
	the_window->norm_width_ = the_win_template->width_;
	the_window->norm_height_ = the_win_template->height_;
	the_window->global_rect_.MinX = the_window->x_;
	the_window->global_rect_.MaxX = (the_window->x_ + the_window->width_) - 1;
	the_window->global_rect_.MinY = the_window->y_;
	the_window->global_rect_.MaxY = (the_window->y_ + the_window->height_) - 1;
	the_window->min_width_ = the_win_template->min_width_;
	the_window->min_height_ = the_win_template->min_height_;
	the_window->max_width_ = the_win_template->max_width_;
	the_window->max_height_ = the_win_template->max_height_;

	the_window->user_data_ = the_win_template->user_data_;
	the_window->buffer_bitmap_ = the_win_template->buffer_bitmap_;
	the_window->show_iconbar_ = the_win_template->show_iconbar_;
	the_window->is_backdrop_ = the_win_template->is_backdrop_;
	the_window->can_resize_ = the_win_template->can_resize_;
	the_window->clip_count_ = 0;
	the_window->event_handler_ = event_handler;
	the_window->selected_control_ = NULL;
	
	// set up the rects for titlebar, content, etc. 
	Window_ConfigureStructureRects(the_window);

	if (the_window->can_resize_)
	{
		// window can be resized, so needs drag zones to be established
		Window_ConfigureDragRects(the_window);
	}
	
	// all windows start off non-visible
	the_window->visible_ = false;
	
	// all windows start out as invalidated, so that they are rendered fully next render pass
	the_window->invalidated_ = true;
	the_window->titlebar_invalidated_ = true;
	
	// pen location starts at 0,0 (relative to content area, not to window rect)
	the_window->pen_x_ = 0;
	the_window->pen_y_ = 0;
	
	// add the base controls (not needed for backdrops -- we don't want user to be able to close a backdrop)
	
	if ( the_win_template->is_backdrop_ == false)
	{
		DEBUG_OUT(("%s %d: getting theme and theme controls...", __func__, __LINE__));

		close_control = Control_New(Theme_GetCloseControlTemplate(the_theme), the_window, &the_window->titlebar_rect_, CLOSE_WIDGET_ID, 0);
		minimize_control = Control_New(Theme_GetMinimizeControlTemplate(the_theme), the_window, &the_window->titlebar_rect_, MINIMIZE_WIDGET_ID, 0);
		normsize_control = Control_New(Theme_GetNormSizeControlTemplate(the_theme), the_window, &the_window->titlebar_rect_, NORM_SIZE_WIDGET_ID, 0);
		maximize_control = Control_New(Theme_GetMaximizeControlTemplate(the_theme), the_window, &the_window->titlebar_rect_, MAXIMIZE_WIDGET_ID, 0);
	
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
	
		// calculate available title width
		Window_CalculateTitleSpace(the_window);
		
		// do first pass clear of the content area
		Window_ClearContent(the_window);
	}
	
	// Add this window to the list of windows
	if (Sys_AddToWindowList(global_system, the_window) == false)
	{
		LOG_ERR(("%s %d: this window cannot be opened: '%s'", __func__, __LINE__, the_window->title_));
		goto error;
	}
	
	// display order is initially set by the system to a number equivalent to the count of windows
	// but backdrop window needs to always be at a specially low (negative number)
	if ( the_win_template->is_backdrop_ == true)
	{
		the_window->display_order_ = SYS_WIN_Z_ORDER_BACKDROP;
	}

	//Window_Print(the_window);
		
	return the_window;
	
error:
	if (the_window)					Window_Destroy(&the_window);
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Window_Destroy(Window** the_window)
{
	if (*the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	if ((*the_window)->title_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	(*the_window)->title_	%p	size	%i		'%s'", __func__ , __LINE__, (*the_window)->title_, General_Strnlen((*the_window)->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, (*the_window)->title_));
		free((*the_window)->title_);
		(*the_window)->title_ = NULL;
	}

	if ((*the_window)->bitmap_)
	{
		Bitmap_Destroy(&(*the_window)->bitmap_);
	}
	
	LOG_ALLOC(("%s %d:	__FREE__	*the_window	%p	size	%i", __func__ , __LINE__, *the_window, sizeof(Window)));
	free(*the_window);
	*the_window = NULL;
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Allocate and populate a new window template object
//! Assigns (but does not copy) the passed title string; leaves bitmaps NULL; assigns the pre-defined default value to all other fields
//! Calling method must free the returned NewWinTemplate pointer after creating a window with it.
//! @param	the_win_title: pointer to the string that will be assigned to the title_ property. No copy or allocation will take place.
//! @return:	A NewWinTemplate with all values set to default, or NULL on any error condition
NewWinTemplate* Window_GetNewWinTemplate(char* the_win_title)
{
	NewWinTemplate*		the_win_template;
	
	if ( (the_win_template = (NewWinTemplate*)calloc(1, sizeof(NewWinTemplate)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new window template", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_win_template	%p	size	%i", __func__ , __LINE__, the_win_template, sizeof(NewWinTemplate)));

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
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}







// **** CLIP RECT MANAGEMENT functions *****


//! Copy the passed rectangle to the window's clip rect collection
//! If the window already has the maximum allowed number of clip rects, the rectangle will not be added.
//! NOTE: the incoming rect must be using window-local coordinates, not global. No translation will be performed.
//! @param	the_window: reference to a valid Window object.
//! @param	new_rect: reference to the rectangle describing the coordinates to be added to the window as a clipping rect. Coordinates of this rect must be window-local! Coordinates in rect are copied to window storage, so it is safe to free the rect after calling this function.
//! @return:	Returns true if rect is copied successfully. Returns false on any error, or if the window already had the maximum number of allowed clip rects.
bool Window_AddClipRect(Window* the_window, Rectangle* new_rect)
{
	Rectangle*	the_clip;
	
	// LOGIC:
	//   A window has a maximum number of clip rects it will track
	//   after that number has been reached, the whole window will need to be reblitted
	//   therefore, once that number is reached, stop accepting new ones
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( new_rect == NULL)
	{
		LOG_ERR(("%s %d: passed rect was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( the_window->clip_count_ >= WIN_MAX_CLIP_RECTS)
	{
		return false;
	}
	
	// LOGIC:
	//   controls / etc pass on their window-local coords
	//   but to blit to screen, we need to correct to global coords

	the_clip = &the_window->clip_rect_[the_window->clip_count_];

	General_CopyRect(the_clip, new_rect);	
	
	the_window->clip_count_++;

	//DEBUG_OUT(("%s %d: window '%s' picked up a clip rect, now has %i cliprects; new clip rect is %i, %i : %i, %i", __func__, __LINE__, the_window->title_, the_window->clip_count_, the_clip->MinX, the_clip->MinY, the_clip->MaxX, the_clip->MaxY));
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Merge and de-duplicate clip rects
//! Consolidating the clip rects will happen reduce unnecessary reblitting
// need to study more before trying to implement this; looks hairy. 
bool Window_MergeClipRects(Window* the_window)
{
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return false;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Blit each clip rect to the screen, and clear all clip rects when done
//! This is the actual mechanics of rendering the window to the screen
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if there are either no clips to blit, or if there are clips and they are blitted successfully. Returns false on any error.
bool Window_BlitClipRects(Window* the_window)
{
	Rectangle*	the_clip;
	Bitmap*		the_screen_bitmap;
	int16_t		i;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_window->clip_count_ == 0)
	{
		return true; // not an error condition
	}
	
	the_screen_bitmap = Sys_GetScreenBitmap(global_system, back_layer);
	
	for (i = 0; i < the_window->clip_count_; i++)
	{
		the_clip = &the_window->clip_rect_[i];

		DEBUG_OUT(("%s %d: win '%s' blitting cliprect %p (%i, %i -- %i, %i)", __func__, __LINE__, the_window->title_, the_clip, the_clip->MinX, the_clip->MinY, the_clip->MaxX, the_clip->MaxY));
	
		Bitmap_Blit(the_window->bitmap_, 
					the_clip->MinX, 
					the_clip->MinY, 
					the_screen_bitmap, 
					the_clip->MinX + the_window->x_, 
					the_clip->MinY + the_window->y_, 
					the_clip->MaxX - the_clip->MinX + 1, 
					the_clip->MaxY - the_clip->MinY + 1
					);
	}
	
	// LOGIC: 
	//   clip rects are one-time usage: once we have blitted them, we never want to blit them again
	//   we want to clear the decks for the next set of updates
	
	the_window->clip_count_ = 0;
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Calculate damage rects, if any, caused by window moving or being resized
//! NOTE: it is not necessarily an error condition if a given window doesn't end up with damage rects as a result of this operation: if the window rect doesn't intersect the incoming rect, no damage is relevant.
//! @param	the_window: reference to a valid Window object.
//! @param	the_old_rect: reference to the rectangle to be checked for overlap with the specified window. Coordinates of this rect must be global!
//! @return:	Returns true if 1 or more damage rects were created. Returns false on any error condition, or if no damage rects needed to be created.
bool Window_GenerateDamageRects(Window* the_window, Rectangle* the_old_rect)
{
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
		
	the_window->damage_count_ = General_CalculateRectDifference(&the_window->global_rect_, the_old_rect, &the_window->damage_rect_[0], &the_window->damage_rect_[1], &the_window->damage_rect_[2], &the_window->damage_rect_[3]);

	//DEBUG_OUT(("%s %d: window '%s' has damage count of %i", __func__, __LINE__, the_window->title_, the_window->damage_count_));

	return (the_window->damage_count_ != -1);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Copy the passed rectangle to the window's clip rect collection, translating to local coordinates as it does so
//! NOTE: the incoming rect is assumed to be using global, not window-local coordinates. Coordinates will be translated to window-local. 
//! Note: it is safe to pass non-intersecting rects to this function: it will check for non-intersection; will trim copy of clip to just the intersection
//! @param	the_window: reference to a valid Window object.
//! @param	damage_rect: reference to the rectangle describing the coordinates to be added to the window as a clipping rect.. Coordinates of this rect must be global!
//! @return:	Returns true if the passed rect has any intersection with the window. Returns false if not intersection, or on any error condition.
bool Window_AcceptDamageRect(Window* the_window, Rectangle* damage_rect)
{
	Rectangle*	the_clip;
	
	// LOGIC:
	//   A window has a maximum number of clip rects it will track
	//   after that number has been reached, the whole window will need to be reblitted
	//   therefore, once that number is reached, stop accepting new ones
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( damage_rect == NULL)
	{
		LOG_ERR(("%s %d: passed rect was null", __func__ , __LINE__));
		goto error;
	}
	
	//DEBUG_OUT(("%s %d: window '%s' has %i cliprects; incoming dmg rect is %i, %i : %i, %i", __func__, __LINE__, the_window->title_, the_window->clip_count_, damage_rect->MinX, damage_rect->MinY, damage_rect->MaxX, damage_rect->MaxY));
	
	if ( the_window->clip_count_ >= WIN_MAX_CLIP_RECTS)
	{
		return false;
	}
	
	the_clip = &the_window->clip_rect_[the_window->clip_count_];
	
	if (General_CalculateRectIntersection(&the_window->global_rect_, damage_rect, the_clip) == true)
	{
		Window_GlobalToLocal(the_window, &the_clip->MinX, &the_clip->MinY);
		Window_GlobalToLocal(the_window, &the_clip->MaxX, &the_clip->MaxY);
		
		the_window->clip_count_++;
	
		DEBUG_OUT(("%s %d: win '%s' got new dmg rect, now has %i cliprects; new dmg rect (l) is %i, %i : %i, %i", __func__, __LINE__, the_window->title_, the_window->clip_count_, the_clip->MinX, the_clip->MinY, the_clip->MaxX, the_clip->MaxY));
		
		return true;
	}
	
	return false;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}






// **** BUILT-IN PSEUDO CONTROL MANAGEMENT functions *****

//! Checks if the passed coordinate is within one of the draggable event zones
//! Draggable event zones include the title bar, 4 single-direction resize zones on the window edges, and the lower-right traditional resize zone
//! @param	the_window: reference to a valid Window object.
//! @param	x: window-local horizontal coordinate
//! @param	y: window-local vertical coordinate
//@ return	Returns mouseFree if the coordinates are in anything but a draggable region. Otherwise returns mouseResizeUp, etc., as appropriate.
MouseMode Window_CheckForDragZone(Window* the_window, int16_t x, int16_t y)
{
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (General_PointInRect(x, y, the_window->title_drag_rect_) == true)
	{
		// NOTE: check for title drag last, because title rect includes a bit of the resize rects
		DEBUG_OUT(("%s %d: **** DRAG in TITLE detected", __func__ , __LINE__));
		DEBUG_OUT(("%s %d: x, y=%i, %i; drag rect=%i, %i x %i, %i", __func__ , __LINE__, x, y, the_window->title_drag_rect_.MinX, the_window->title_drag_rect_.MinY, the_window->title_drag_rect_.MaxX, the_window->title_drag_rect_.MaxY));
		return mouseDragTitle;
	}
	else if (General_PointInRect(x, y, the_window->grow_bottom_right_rect_) == true)
	{
		DEBUG_OUT(("%s %d: **** RESIZE down-RIGHT detected", __func__ , __LINE__));
		DEBUG_OUT(("%s %d: x, y=%i, %i; drag rect=%i, %i x %i, %i", __func__ , __LINE__, x, y, the_window->grow_bottom_right_rect_.MinX, the_window->grow_bottom_right_rect_.MinY, the_window->grow_bottom_right_rect_.MaxX, the_window->grow_bottom_right_rect_.MaxY));
		return mouseResizeDownRight;
	}
	else if (General_PointInRect(x, y, the_window->grow_right_rect_) == true)
	{
		DEBUG_OUT(("%s %d: **** RESIZE RIGHT detected", __func__ , __LINE__));
		DEBUG_OUT(("%s %d: x, y=%i, %i; drag rect=%i, %i x %i, %i", __func__ , __LINE__, x, y, the_window->grow_right_rect_.MinX, the_window->grow_right_rect_.MinY, the_window->grow_right_rect_.MaxX, the_window->grow_right_rect_.MaxY));
		return mouseResizeRight;
	}
	else if (General_PointInRect(x, y, the_window->grow_bottom_rect_) == true)
	{
		DEBUG_OUT(("%s %d: **** RESIZE DOWN detected", __func__ , __LINE__));
		DEBUG_OUT(("%s %d: x, y=%i, %i; drag rect=%i, %i x %i, %i", __func__ , __LINE__, x, y, the_window->grow_bottom_rect_.MinX, the_window->grow_bottom_rect_.MinY, the_window->grow_bottom_rect_.MaxX, the_window->grow_bottom_rect_.MaxY));
		return mouseResizeDown;
	}
	else if (General_PointInRect(x, y, the_window->grow_left_rect_) == true)
	{
		DEBUG_OUT(("%s %d: **** RESIZE LEFT detected", __func__ , __LINE__));
		DEBUG_OUT(("%s %d: x, y=%i, %i; drag rect=%i, %i x %i, %i", __func__ , __LINE__, x, y, the_window->grow_left_rect_.MinX, the_window->grow_left_rect_.MinY, the_window->grow_left_rect_.MaxX, the_window->grow_left_rect_.MaxY));
		return mouseResizeLeft;
	}
	else if (General_PointInRect(x, y, the_window->grow_top_rect_) == true)
	{
		DEBUG_OUT(("%s %d: **** RESIZE UP detected", __func__ , __LINE__));
		DEBUG_OUT(("%s %d: x, y=%i, %i; drag rect=%i, %i x %i, %i", __func__ , __LINE__, x, y, the_window->grow_top_rect_.MinX, the_window->grow_top_rect_.MinY, the_window->grow_top_rect_.MaxX, the_window->grow_top_rect_.MaxY));
		return mouseResizeUp;
	}

	return mouseFree;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return mouseFree;
}




// **** CONTROL MANAGEMENT functions *****


//! Sets the passed control as the currently selected control and unselects any previously selected control
//! @param	the_window: reference to a valid Window object.
//! @param	the_control: reference to a valid Control object.
//! @return:	Returns false on any error
bool Window_SetSelectedControl(Window* the_window, Control* the_control)
{
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_window->selected_control_ != NULL)
	{
		Control_SetPressed(the_window->selected_control_, false);
	}
	
	the_window->selected_control_ = the_control;
	Control_SetPressed(the_control, true);
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Adds the passed control to the window's list of controls
//! @param	the_window: reference to a valid Window object.
//! @param	the_control: reference to a valid Control object.
//! @return:	Returns false in any error condition
bool Window_AddControl(Window* the_window, Control* the_control)
{
	Control*	last_control_in_window;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( the_control == NULL)
	{
		LOG_ERR(("%s %d: passed control was null", __func__ , __LINE__));
		goto error;
	}
	
	// The way that controls are associated with a window is to add them as the next_ property to the last control in the window
	if ( (last_control_in_window = Window_GetLastControl(the_window)) != NULL)
	{
		Control_SetNextControl(last_control_in_window, the_control);
		Control_SetNextControl(the_control, NULL);
		Control_SetActive(the_control, CONTROL_ACTIVE);
		//DEBUG_OUT(("%s %d: control (%p, type=%i) added!", __func__, __LINE__, the_control, the_control->type_));
		
		return true;
	}
	
	return false;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Instantiate a new control from the passed template, and add it to the window's list of controls
//! @param	the_window: reference to a valid Window object.
//! @param	the_template: reference to a valid, populated ControlTemplate object. The created control will take most of its properties from this template.
//! @param	the_id: the unique ID (within the specified window) to be assigned to the control. WARNING: assigning multiple controls the same ID will result in undefined behavior.
//! @param	group_id: 1 byte group ID value to be assigned to the control. Pass CONTROL_NO_GROUP if the control is not to be part of a group.
//! @return:	Returns a pointer to the new control, or NULL in any error condition
Control* Window_AddNewControlFromTemplate(Window* the_window, ControlTemplate* the_template, uint16_t the_id, uint16_t group_id)
{
	Control*			the_control;
		
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( the_template == NULL)
	{
		LOG_ERR(("%s %d: passed control template was null", __func__ , __LINE__));
		goto error;
	}
	
	the_control = Control_New(the_template, the_window, &the_window->content_rect_, the_id, group_id);

	if (Window_AddControl(the_window, the_control) == false)
	{
		// we failed to add the control to the window, so need to dispose of it or nothing ever will
		LOG_ERR(("%s %d: control created successfully, but it couldn't be added to the window", __func__ , __LINE__));
		Control_Destroy(&the_control);
		return NULL;
	}
	
	return the_control;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


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
Control* Window_AddNewControl(Window* the_window, control_type the_type, int16_t width, int16_t height, int16_t x_offset, int16_t y_offset, h_align_type the_h_align, v_align_type the_v_align, char* the_caption, uint16_t the_id, uint16_t group_id)
{
	Theme*				the_theme;
	Control*			the_control;
	ControlTemplate*	the_template;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_theme = Sys_GetTheme(global_system);
	
	if ( (the_template = Theme_CreateControlTemplateFlexWidth(the_theme, the_type, width, height, x_offset, y_offset, the_h_align, the_v_align, the_caption)) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create the control template", __func__, __LINE__));
		return NULL;
	}
	
	the_control = Window_AddNewControlFromTemplate(the_window, the_template, the_id, group_id);

	// control template no longer needed
	ControlTemplate_Destroy(&the_template);

	// The way that controls are associated with a window is to add them as the next_ property to the last control in the window
	if ( the_control == NULL)
	{
		LOG_ERR(("%s %d: control template created successfully, but failed to add a control to the window", __func__ , __LINE__));
		return NULL;
	}
	
	return the_control;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Invalidate the title bar and the controls in the title bar
//! Call when switching from inactive to active window, and vice versa, to force controls and title bar to redraw appropriately
//! @param	the_window: reference to a valid Window object.
void Window_InvalidateTitlebar(Window* the_window)
{
	int16_t		i;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->titlebar_invalidated_ = true;

	// calculate available title width
	Window_CalculateTitleSpace(the_window);
	
	// also invalidate any controls in the titlebar, as re-drawing titlebar will draw over them
	// controls in titlebar have IDs 0-3
	for (i = 0; i < MAX_BUILT_IN_WIDGET; i++)
	{
		Control*	the_control;

		the_control = Window_GetControl(the_window, i);
		Control_MarkInvalidated(the_control, true);
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Get the control listed as the currently selected control.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a control pointer, or NULL on any error, or if there is no selected control currently
Control* Window_GetSelectedControl(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->selected_control_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Find and return the last control in the window's chain of controls
//! This corresponds to the first control with a NULL value for next_
Control* Window_GetLastControl(Window* the_window)
{
	Control*	the_control = NULL;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_control = Window_GetRootControl(the_window);
	
	while (the_control)
	{
		if (the_control->next_ == NULL)
		{
			return the_control;
		}
		
		the_control = the_control->next_;
	}
	
	return NULL;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Return a pointer to the control owned by the window that matches the specified ID
//! NOTE: Control IDs 0-3 are reserved by the system for standard controls (close, minimize, etc.). Other control IDs are specified by the programmer for each window. Control IDs are not global, they are window-specific.
//! @param	the_window: reference to a valid Window object.
//! @param	the_control_id: ID of the control that you want to find
//! @return:	Returns a pointer to the control with the ID passed, or NULL if no match found, or on any error condition
Control* Window_GetControl(Window* the_window, uint16_t the_control_id)
{
	Control*	the_control = NULL;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
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
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Return the ID of the control passed, if the window actually owns that control
//! NOTE: Control IDs 0-3 are reserved by the system for standard controls (close, minimize, etc.). Other control IDs are specified by the programmer for each window. Control IDs are not global, they are window-specific.
//! @param	the_window: reference to a valid Window object.
//! @param	the_control: Pointer to the control whose ID you want to find
//! @return:	Returns the ID of the control, or CONTROL_ID_NOT_FOUND if no match found, or CONTROL_ID_ERROR on any error condition
uint16_t Window_GetControlID(Window* the_window, Control* the_control)
{
	Control*	this_control = NULL;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	this_control = Window_GetRootControl(the_window);
	
	while (this_control)
	{
		if (this_control == the_control)
		{
			return Control_GetID(this_control);
		}
		
		this_control = this_control->next_;
	}
	
	return CONTROL_ID_NOT_FOUND;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return CONTROL_ID_ERROR;
}


//! Find the control, if any, located at the specified local coordinates
//! Transparency is not taken into account: if the passed coordinate intersects the control's rectangle at any point, it is considered a match
//! @param	the_window: reference to a valid Window object.
//! @param	x: window-local horizontal coordinate
//! @param	y: window-local vertical coordinate
//! @return:	Returns a pointer to the control at the passed coordinates, or NULL if no match found, or on any error condition
Control* Window_GetControlAtXY(Window* the_window, int16_t x, int16_t y)
{
 	Control*	the_control;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	// LOGIC:
	//   Controls are in a linked list property of the window
	//   Unlike finding window under mouse, for control, we don't care about order
	//   Programmer who allows overlapping controls is doing something wrong anyway!
		
	the_control = the_window->root_control_;

	while (the_control != NULL)
	{
 		bool		in_this_control;
		
		in_this_control = General_PointInRect(x, y, the_control->rect_);
		
		if (in_this_control)
		{
			return the_control;
		}

		the_control = the_control->next_;
	}
	
	return NULL;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}




// **** Render functions *****


//! Draw/re-draw any necessary components, and blit the window (or parts of it, via cliprects) to the screen
//! @param	the_window: reference to a valid Window object.
void Window_Render(Window* the_window)
{
	Theme*	the_theme;
	Bitmap*	the_pattern;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	// LOGIC:
	//   From a rendering point of view, windows are split into "backdrop" and "not-backdrop" windows
	//   Backdrop windows always fill the screen and always are filled with their backdrop pattern and never have borders, controls, etc. 
	//   Non-backdrop windows are built up from overall struct, content area, and controls. 
	//     Except for the first render, the overall struct and content area are generally not cleared/re-rendered. 
	//   For both backdrop and non-backdrop windows, render will only redraw the entire window if either:
	//     A) the window itself is set as invalidated OR
	//     B) there are more than X clip rects that need blitting, so system decides it is faster to blit the window in one go.
	
	the_theme = Sys_GetTheme(global_system);
	the_pattern = Theme_GetDesktopPattern(the_theme);

	if (the_window->visible_ == false)
	{
		return;
	}
	
	if (the_window->is_backdrop_)
	{
		if (the_window->invalidated_ == true)
		{
			// backdrop window: fill it with its pattern. no controls, borders, etc. 
			// tile the default theme's background pattern
			Bitmap_Tile(the_pattern, 0, 0, the_window->bitmap_, the_theme->pattern_width_, the_theme->pattern_height_);
		}
	}
	else
	{
		// non-backdrop window: render borders, titlebars, controls, etc, as appropriate

		// if entire window has been invalidated (resize, etc.), then redraw everything, including all controls
		// if only title bar has been invalidated, redraw structures (will also re-render any invalidated controls)
		// if just rendering otherwise, give controls a chance to render themselves if necessary
		
		if (the_window->invalidated_ == true)
		{
			Window_DrawAll(the_window);
		}
		else
		{
			// TODO: call the programmer-window's event handler, giving it a "draw" event?
			
			if (the_window->titlebar_invalidated_ == true)
			{
				Window_DrawStructure(the_window);
			}
			
			// Re-draw any controls that were invalidated and queue them for render
			Window_DrawControls(the_window, WIN_PARAM_ONLY_REDRAW_INVAL_CONTROLS);
		}
	}

	// blit to screen
	
	// if the entire window has had to be redrawn, then don't bother with individual cliprects, just do one for entire window
	if (the_window->invalidated_ == true || the_window->clip_count_ >= WIN_MAX_CLIP_RECTS)
	{
		the_window->clip_count_ = 0;
		Bitmap_BlitRect(the_window->bitmap_, the_window->overall_rect_, Sys_GetScreenBitmap(global_system, back_layer), the_window->x_, the_window->y_);
		the_window->invalidated_ = false;
	}
	else
	{
		DEBUG_OUT(("%s %d: window '%s' has %i clip rects to render", __func__, __LINE__, the_window->title_, the_window->clip_count_));
		
		Window_BlitClipRects(the_window);
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Updates the current window controls, etc., to match the current system theme 
void Window_UpdateTheme(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	// adjust the rects for titlebar, content, etc., as each theme can change the position, height, etc. 
	Window_ConfigureStructureRects(the_window);

	// have all controls update themselves
	Window_UpdateControlTheme(the_window);
	
	// recalculate available title width
	Window_CalculateTitleSpace(the_window);
	
	Window_Invalidate(the_window);	
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Clears the content area rect by filling it with the theme's backcolor
//! @param	the_window: reference to a valid Window object.
void Window_ClearContent(Window* the_window)
{
	Theme*	the_theme;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	the_theme = Sys_GetTheme(global_system);

	Bitmap_FillBoxRect(the_window->bitmap_, &the_window->content_rect_, Theme_GetContentAreaColor(the_theme));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Mark entire window as invalidated
//! This will cause it to be redrawn and fully reblitted in the next render cycle
void Window_Invalidate(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->invalidated_ = true;
	
	if (the_window->is_backdrop_ == false)
	{
		Window_InvalidateTitlebar(the_window);
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}




// **** Set functions *****



// replace the current window title with the passed string
// Note: the passed string will be copied into storage by the window. The passing function can dispose of the passed string when done.
void Window_SetTitle(Window* the_window, char* the_title)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	if (the_window->title_)
	{
		LOG_ALLOC(("%s %d:	__FREE__	the_window->title_	%p	size	%i		'%s'", __func__ , __LINE__, the_window->title_, General_Strnlen(the_window->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, the_window->title_));
		free(the_window->title_);
	}
	
	if ( (the_window->title_ = General_StrlcpyWithAlloc(the_title, WINDOW_MAX_WINTITLE_SIZE)) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory for the window name string", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_window->title_	%p	size	%i		'%s'", __func__ , __LINE__, the_window->title_, General_Strnlen(the_window->title_, WINDOW_MAX_WINTITLE_SIZE) + 1, the_window->title_));
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}



//! Set the window's visibility flag.
//! NOTE: This does not immediately cause the window to render. The window will be rendered on the next system rendering pass.
//! @param	the_window: reference to a valid Window object.
//! @param	is_visible: set to true if window should be rendered in the next pass, false if not
void Window_SetVisible(Window* the_window, bool is_visible)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->visible_ = is_visible;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the display order of the window
//! NOTE: This does not immediately re-render or change the display order visibly.
//! WARNING: This function is designed to be called by the system only: do not use this
//! @param	the_window: reference to a valid Window object
//! @param	the_display_order: the new display order value for the window
void Window_SetDisplayOrder(Window* the_window, int8_t the_display_order)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->display_order_ = the_display_order;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the passed window's active flag.
//! NOTE: This does not immediately cause the window to render as active or inactive, but it does invalidate the title bar so that it re-renders in the next render pass.
//! @param	the_window: reference to a valid Window object.
//! @param	is_active: set to true if window is now considered the active window, false if not
void Window_SetActive(Window* the_window, bool is_active)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->active_ = is_active;
	
	if (the_window->is_backdrop_ == false)
	{
		Window_InvalidateTitlebar(the_window);
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the window's state (maximized, minimized, etc.)
//! NOTE: This does not immediately cause the window to render in the passed state.
//! @param	the_window: reference to a valid Window object.
//! @param	the_state: the new state
void Window_SetState(Window* the_window, window_state the_state)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->state_ = the_state;
	
	// need to do anything else? 
	// TODO: analyze if setting state function needs to do anything besides simply set the property value
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Evaluate potential change to window position or size, and correct if out of allowed limits
//! Negative value positions will be corrected to 0.
//! @param	the_window: reference to a valid Window object.
//! @param	x: Pointer to the proposed new horizontal position. If less than 0, it will be set to 0.
//! @param	y: Pointer to the proposed new vertical position. If less than 0, it will be set to 0.
//! @param	width: Pointer to the proposed new width. Will be set to window's minimum or maximum if necessary.
//! @param	height: Pointer to the proposed new height. Will be set to window's minimum or maximum if necessary.
void Window_EvaluateWindowChange(Window* the_window, int16_t* x, int16_t* y, int16_t* width, int16_t* height)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	*x = (*x < 0 ? 0 : *x);
	*y = (*y < 0 ? 0 : *y);

	if (*width < the_window->min_width_)
	{
		DEBUG_OUT(("%s %d: width (%i) < min_width (%i) ; changing", __func__, __LINE__, *width, the_window->min_width_));
		*width = the_window->min_width_;
	}
	else if (*width > the_window->max_width_)
	{
		DEBUG_OUT(("%s %d: width (%i) > max_width (%i); changing", __func__, __LINE__, *width, the_window->max_width_));
		*width = the_window->max_width_;
	}
	
	if (*height < the_window->min_height_)
	{
		DEBUG_OUT(("%s %d: height (%i) < min_height_ (%i); changing", __func__, __LINE__, *height, the_window->min_height_));
		*height = the_window->min_height_;
	}
	else if (*height > the_window->max_height_)
	{
		DEBUG_OUT(("%s %d: height (%i) > max_height (%i); changing", __func__, __LINE__, *height, the_window->max_height_));
		*height = the_window->max_height_;
	}
	
	return;

error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}

//! Change position and/or size of window
//! NOTE: passed x, y will be checked against the window's min/max values
//! Will also adjust the position of the built-in maximize/minimize/normsize controls
//! @param	the_window: reference to a valid Window object.
//! @param	x: The new global horizontal position
//! @param	y: The new global vertical position
//! @param	width: The new width
//! @param	height: The new height
//! @param	update_norm: if true, the window's normal x/y/width/height properties will be updated to match the passed values. Pass false if setting maximize size, etc.
void Window_ChangeWindow(Window* the_window, int16_t x, int16_t y, int16_t width, int16_t height, bool update_norm)
{
	bool		width_changed = false;
	Rectangle	the_old_rect; //! will contain global rect of window before resize/move
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// check if anything actually changed
	if (the_window->x_ != x || the_window->y_ != y || the_window->width_ != width || the_window->height_ != height)
	{
		// ensure width/height are within min/max, and x/y are not negative
		Window_EvaluateWindowChange(the_window, &x, &y, &width, &height);

		if (the_window->width_ != width)
		{
			width_changed = true;
		}
		
		if (update_norm)
		{
			the_window->norm_x_ = x;
			the_window->norm_y_ = y;
			the_window->norm_width_ = width;
			the_window->norm_height_ = height;
		}
		
		// get copy of window rect before changing it, for use with calculating damage rects
		General_CopyRect(&the_old_rect, &the_window->global_rect_);
		
		the_window->x_ = x;
		the_window->y_ = y;
		the_window->width_ = width;
		the_window->height_ = height;
		the_window->global_rect_.MinX = the_window->x_;
		the_window->global_rect_.MaxX = the_window->x_ + the_window->width_ - 1;
		the_window->global_rect_.MinY = the_window->y_;
		the_window->global_rect_.MaxY = the_window->y_ + the_window->height_ - 1;

		// create damage rects at this point - does not percolate them anywhere, or do any rendering
		Window_GenerateDamageRects(the_window, &the_old_rect);
		Sys_IssueDamageRects(global_system);
		
		// set up the rects for titlebar, content, etc. 
		Window_ConfigureStructureRects(the_window);
	
		// invalidate the window and titlebar so they redraw
		the_window->invalidated_ = true;
		Window_InvalidateTitlebar(the_window);

		// get bigger storage if necessary
		if (Bitmap_Resize(the_window->bitmap_, width, height) == false)
		{
			LOG_ERR(("%s %d: could not resize window storage!", __func__ , __LINE__));
			goto error;
		}		

		// when width changes, controls in titlebar, and in content area, need to get re-aligned to new width
		if (width_changed)
		{
			Control*	the_control = NULL;
			
			// Note: recalculating title space and moving titlebar widgets if width changed is probably slow
			if ( the_window->is_backdrop_ == false)
			{
				// calculate available title width
				Window_CalculateTitleSpace(the_window);
			}
		
			// have all controls re-align themselves
			the_control = Window_GetRootControl(the_window);

			while (the_control)
			{
				// skip aligning title bar controls if this window doesn't even have a titlebar
				if (Control_GetID(the_control) >= MAX_BUILT_IN_WIDGET || the_window->is_backdrop_ == false)
				{
					Control_AlignToParentRect(the_control);
				}
				
				the_control = the_control->next_;						
			}		
		}
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the window to full-screen size (maximize mode)
//! Sets window's x, y, width, height parameters to match those of the screen
void Window_Maximize(Window* the_window)
{
	Screen*		the_screen;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	//   when a window is set to maximize size, we take the screen's width/height 
	//   values and set window width/height to them (x/y to 0,0)
	//   also need to set the state to WIN_MAXIMIZED
	//   actual resizing can be accomplished by calling Window_ChangeWindow, passing the screen size values
	//   TODO: figure out what is responsible for setting window visible=true when coming out of minimized mode.
	//   TODO: figure out what future actions need to take place when a window is set to maximized size

	the_screen = Sys_GetScreen(global_system, ID_CHANNEL_B);

	Window_SetState(the_window, WIN_MAXIMIZED);
	Window_ChangeWindow(the_window, 0, 0, the_screen->width_, the_screen->height_, WIN_PARAM_DO_NOT_UPDATE_NORM_SIZE);
	Sys_Render(global_system);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the window to normal size (window-size mode)
//! Sets window's x, y, width, height parameters to those stored in norm_x, etc.
void Window_NormSize(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   when a window is set to normal/window size, we take the norm_x/norm_y etc 
	//   values and set x/y/width/height to them
	//   also need to set the state to WIN_NORMAL
	//   actual resizing can be accomplished by calling Window_ChangeWindow, passing the norm values
	//   TODO: figure out what is responsible for setting window visible=true when coming out of minimized mode.
	//   TODO: figure out what future actions need to take place when a window is set to normal size

	Window_SetState(the_window, WIN_NORMAL);
	Window_ChangeWindow(the_window, the_window->norm_x_, the_window->norm_y_, the_window->norm_width_, the_window->norm_height_, WIN_PARAM_DO_NOT_UPDATE_NORM_SIZE);
	Sys_Render(global_system);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Hides the window (minimize mode)
//! Does not change the window's x, y, width, height parameters, it just makes it invisible
void Window_Minimize(Window* the_window)
{
	Rectangle	the_new_rect;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   when a window is minimized, it is not visible at all (no icon on desktop, nothing)
	//   to access it, the contextual menu or a keyboard shortcut needs to be used
	//   so we need to set state to minimized, and also make it invisible.
	//   TODO: figure out what future actions need to take place when a window is minimized

	
	// before hiding the window, calculate and distribute any damage rects that may result from it being removed from screen
	the_new_rect.MinX = -2;
	the_new_rect.MinY = -2;
	the_new_rect.MaxX = -1;
	the_new_rect.MaxY = -1;
	Window_GenerateDamageRects(the_window, &the_new_rect);
	Sys_IssueDamageRects(global_system);
	
	Window_SetState(the_window, WIN_MINIMIZED);
	Window_SetVisible(the_window, false);
	Sys_Render(global_system);
	
	DEBUG_OUT(("%s %d: window '%s' has been minimized", __func__, __LINE__, the_window->title_));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


// typedef enum window_state
// {
// 	WIN_HIDDEN			= 0,
// 	WIN_MINIMIZED 		= 1,
// 	WIN_NORMAL			= 2,
// 	WIN_MAXIMIZED		= 3,
// 	WIN_UNKNOWN_STATE,
// } window_state;






// **** Get functions *****



//! Get a pointer to the current window title
//! Note: It is not guaranteed that every window will have a title. Backdrop windows, for example, do not have a title.
//! Note: the window title is maintained by the window. Do not free the string pointer returned by this function!
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a pointer to the title string. Returns NULL in any error condition.
char* Window_GetTitle(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->title_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Get the value stored in the user data field of the window.
//! NOTE: this field is for the exclusive use of application programs. The system will not act on this data in any way: you are free to store whatever 4-byte value you want here.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns an unsigned 32 bit value. Returns 0 in any error condition.
uint32_t Window_GetUserData(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->user_data_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return 0;
}


//! Get the window's type (normal, backdrop, dialog, etc.)
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a window_type enum. Returns WIN_UNKNOWN_TYPE in any error condition.
window_type Window_GetType(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->type_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return WIN_UNKNOWN_TYPE;
}


//! Get the window's state (maximized, minimized, etc.)
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a window_state enum. Returns WIN_UNKNOWN_STATE in any error condition.
window_state Window_GetState(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->state_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return WIN_UNKNOWN_STATE;
}


//! Get the bitmap object used as the offscreen buffer for the window
//! NOTE: this is not a pointer into VRAM, or directly to the screen.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns a pointer to the bitmap used by the window. Returns NULL in any error condition.
Bitmap* Window_GetBitmap(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->bitmap_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Get the global horizontal coordinate of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns the horizontal portion of the upper-left coordinate of the window. Returns -1 in any error condition.
int16_t Window_GetX(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->x_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


//! Get the global vertical coordinate of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns the vertical portion of the upper-left coordinate of the window. Returns -1 in any error condition.
int16_t Window_GetY(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->y_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


//! Get the width of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns -1 in any error condition.
int16_t Window_GetWidth(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->width_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


//! Get the height of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns -1 in any error condition.
int16_t Window_GetHeight(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->height_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return -1;
}


//! Check if a window is a backdrop window or a regular window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if backdrop, false if not
bool Window_IsBackdrop(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	return the_window->is_backdrop_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Check if a window should be visible or not
//! NOTE: this does not necessarily mean the window isn't currently rendered to the screen. This indicates if it will or won't be after the next render pass.
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if window should be rendered, false if not
bool Window_IsVisible(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	return the_window->visible_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Get the active/inactive condition of the window
//! @param	the_window: reference to a valid Window object.
//! @return:	Returns true if window is active, false if not
bool Window_IsActive(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_window->active_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}





// **** DRAWING functions *****


//! Convert the passed x, y global coordinates to local (to window) coordinates
void Window_GlobalToLocal(Window* the_window, int16_t* x, int16_t* y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	*x = *x - the_window->x_;
	*y = *y - the_window->y_;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Convert the passed x, y local coordinates to global coordinates
void Window_LocalToGlobal(Window* the_window, int16_t* x, int16_t* y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	*x = *x + the_window->x_;
	*y = *y + the_window->y_;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


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
		goto error;
	}
	
	if (the_font == NULL)
	{
		LOG_WARN(("%s %d: passed font was null", __func__ , __LINE__));
		goto error;
	}
	
	the_window->pen_font_ = the_font;
	the_window->bitmap_->font_ = the_font;
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
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
		goto error;
	}
	
	the_window->pen_color_ = the_color;
	the_window->bitmap_->color_ = the_color;
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Set the local "pen" position within the window, based on global coordinates
//! This also sets the pen position of the window's bitmap
//! This is the location that the next pen-based graphics function will use for a starting location
//! @param	the_window: reference to a valid Window object.
//! @param	x: the global horizontal position to be converted to window-local. Will be clipped to the edges.
//! @param	y: the global vertical position to be converted to window-local. Will be clipped to the edges.
//! @return Returns false on any error condition
bool Window_SetPenXYFromGlobal(Window* the_window, int16_t x, int16_t y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	//DEBUG_OUT(("%s %d: window global rect: %i, %i - %i, %i", __func__ , __LINE__, the_window->global_rect_.MinX, the_window->global_rect_.MinY, the_window->global_rect_.MaxX, the_window->global_rect_.MaxY));
	//DEBUG_OUT(("%s %d: x/y before making local: %i, %i", __func__ , __LINE__, x, y));
	x -= the_window->x_;
	y -= the_window->y_;
	//DEBUG_OUT(("%s %d: x/y after making local: %i, %i", __func__ , __LINE__, x, y));
	//DEBUG_OUT(("%s %d: content rect: %i, %i - %i, %i", __func__ , __LINE__, the_window->content_rect_.MinX, the_window->content_rect_.MinY, the_window->content_rect_.MaxX, the_window->content_rect_.MaxY));
	
	return Window_SetPenXY(the_window, x, y);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Set the "pen" position within the content area
//! This also sets the pen position of the window's bitmap
//! This is the location that the next pen-based graphics function will use for a starting location
//! @param	the_window: reference to a valid Window object.
//! @param	x: the horizontal position within the content area of the window. Will be clipped to the edges.
//! @param	y: the vertical position within the content area of the window. Will be clipped to the edges.
//! @return Returns false on any error condition
bool Window_SetPenXY(Window* the_window, int16_t x, int16_t y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
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
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Blit from source bitmap to the window's content area, at the window's current pen coordinate
//! The source bitmap can be the window's bitmap: you can use this to copy a chunk of pixels from one part of a window to another. If the destination location cannot fit the entirety of the copied rectangle, the copy will be truncated, but will not return an error. 
//! @param	the_window: reference to a valid Window object.
//! @param src_bm: the source bitmap. It must have a valid address within the VRAM memory space.
//! @param src_x, src_y: the upper left coordinate within the source bitmap, for the rectangle you want to copy. May be negative.
//! @param width, height: the scope of the copy, in pixels.
bool Window_Blit(Window* the_window, Bitmap* src_bm, int16_t src_x, int16_t src_y, int16_t width, int16_t height)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_Blit(src_bm, src_x, src_y, the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Fill a rectangle drawn from the current pen location, for the passed width/height
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_FillBox(Window* the_window, int16_t width, int16_t height, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_FillBox(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_FillBoxRect(Window* the_window, Rectangle* the_coords, uint8_t the_color)
{
	int16_t		x1;
	int16_t		y1;
	int16_t		x2;
	int16_t		y2;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// localize to content area
	x1 = the_coords->MinX + the_window->content_rect_.MinX;
	y1 = the_coords->MinY + the_window->content_rect_.MinY;
	x2 = the_coords->MaxX + the_window->content_rect_.MinX;
	y2 = the_coords->MaxY + the_window->content_rect_.MinY;
	
	return Bitmap_FillBox(the_window->bitmap_, x1, y1, x2 - x1, y2 - y1, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Set the color of the pixel at the current pen location
//! @param	the_window: reference to a valid Window object.
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_SetPixel(Window* the_window, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_SetPixelAtXY(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a line between 2 passed coordinates.
//! Use for any line that is not perfectly vertical or perfectly horizontal
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! @param	the_color: a 1-byte index to the current color LUT
//! Based on http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C. Used in C128 Lich King. 
bool Window_DrawLine(Window* the_window, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// localize to content area
	x1 += the_window->content_rect_.MinX;
	y1 += the_window->content_rect_.MinY;
	x2 += the_window->content_rect_.MinX;
	y2 += the_window->content_rect_.MinY;
	
	return Bitmap_DrawLine(the_window->bitmap_, x1, y1, x2, y2, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a horizontal line from the current pen location, for n pixels, using the specified pixel value
//! @param	the_window: reference to a valid Window object.
//! @param	the_line_len: The total length of the line, in pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawHLine(Window* the_window, int16_t the_line_len, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_DrawHLine(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, the_line_len, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a vertical line from specified coords, for n pixels
//! @param	the_window: reference to a valid Window object.
//! @param	the_line_len: The total length of the line, in pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawVLine(Window* the_window, int16_t the_line_len, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_DrawVLine(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, the_line_len, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawBoxRect(Window* the_window, Rectangle* the_coords, uint8_t the_color)
{
	int16_t		x1;
	int16_t		y1;
	int16_t		x2;
	int16_t		y2;
	
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// localize to content area
	x1 = the_coords->MinX + the_window->content_rect_.MinX;
	y1 = the_coords->MinY + the_window->content_rect_.MinY;
	x2 = the_coords->MaxX + the_window->content_rect_.MinX;
	y2 = the_coords->MaxY + the_window->content_rect_.MinY;
	
	return Bitmap_DrawBoxCoords(the_window->bitmap_, x1, y1, x2, y2, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a rectangle based on 2 sets of coords, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	x1: the starting horizontal position within the content area of the window
//! @param	y1: the starting vertical position within the content area of the window
//! @param	x2: the ending horizontal position within the content area of the window
//! @param	y2: the ending vertical position within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return:	returns false on any error/invalid input.
bool Window_DrawBoxCoords(Window* the_window, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	// localize to content area
	x1 += the_window->content_rect_.MinX;
	y1 += the_window->content_rect_.MinY;
	x2 += the_window->content_rect_.MinX;
	y2 += the_window->content_rect_.MinY;
	
	return Bitmap_DrawBoxCoords(the_window->bitmap_, x1, y1, x2, y2, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a rectangle based on start coords and width/height, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return:	returns false on any error/invalid input.
bool Window_DrawBox(Window* the_window, int16_t width, int16_t height, uint8_t the_color, bool do_fill)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_DrawBox(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height, the_color, do_fill);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draws a rounded rectangle from the current pen location, with the specified size and radius, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return:	returns false on any error/invalid input.
bool Window_DrawRoundBox(Window* the_window, int16_t width, int16_t height, int16_t radius, uint8_t the_color, bool do_fill)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_DrawRoundBox(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height, radius, the_color, do_fill);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Draw a circle centered on the current pen location
//! Based on http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
//! @param	the_window: reference to a valid Window object.
//! @param	radius: radius, in pixels, measured from the window's current pen location
//! @param	the_color: a 1-byte index to the current color LUT
bool Window_DrawCircle(Window* the_window, int16_t radius, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Bitmap_DrawCircle(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, radius, the_color);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// Draw a string at the current "pen" location, using the current pen color of the Window
// Truncate, but still draw the string if it is too long to display on the line it started.
// No word wrap is performed. 
// If max_chars is less than the string length, only that many characters will be drawn (as space allows)
// If max_chars is -1, then the full string length will be drawn, as space allows.
bool Window_DrawString(Window* the_window, char* the_string, int16_t max_chars)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return Font_DrawString(the_window->bitmap_, the_string, max_chars);
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


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
char* Window_DrawStringInBox(Window* the_window, int16_t width, int16_t height, char* the_string, int16_t num_chars, char** wrap_buffer, bool (* continue_function)(void))
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
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
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}




// **** Get functions *****






// **** xxx functions *****




