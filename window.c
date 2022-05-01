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
//! @return	Returns -1 in event of error, or the calculated width
static int16_t Window_CalculateTitleSpace(Window* the_window);
	
// **** Private RENDER functions *****

// draws or redraws the structure area of the windows (excluding the content area and controls)
static void Window_DrawStructure(Window* the_window);

// draws or redraws the entire window, including clearing the content area
static void Window_DrawAll(Window* the_window);

// draws or redraws the window controls
static void Window_DrawControls(Window* the_window);

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
	the_window->grow_left_rect_.MinX = 0;
	the_window->grow_left_rect_.MinY = 0;
	the_window->grow_left_rect_.MaxX = WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_left_rect_.MaxY = the_window->height_ - 1;

	the_window->grow_right_rect_.MinX = the_window->width_ - WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_right_rect_.MinY = 0;
	the_window->grow_right_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_right_rect_.MaxY = the_window->height_ - 10;	// cutout at bottom for the traditional lower/right corner drag zone

	the_window->grow_top_rect_.MinX = 0;
	the_window->grow_top_rect_.MinY = 0;
	the_window->grow_top_rect_.MaxX = the_window->width_ - 1;
	the_window->grow_top_rect_.MaxY = WIN_DEFAULT_DRAG_ZONE_SIZE - 1;

	the_window->grow_bottom_rect_.MinX = 0;
	the_window->grow_bottom_rect_.MinY = the_window->height_ - WIN_DEFAULT_DRAG_ZONE_SIZE - 1;
	the_window->grow_bottom_rect_.MaxX = the_window->width_ - 10;	// cutout at right for the traditional lower/right corner drag zone
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
	
	// LOGIC: 
	//   In final model, the location and height of the content and titlebar rects will be configurable
	//   In this early prototype, they are going to be hard coded here
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		return;
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
		the_window->titlebar_rect_.MaxX = the_window->width_;
	

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
		return;
	}
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		return;
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
}

//! Determine available pixel width for the title, based on theme's title x offset and left-most control
//! Run after every re-size, or after a theme change
//! Note: this returns the result space and sets the window's avail_title_width_ property. It does not force any re-rendering.
//! @return	Returns -1 in event of error, or the calculated width
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
		return -1;
	}
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		return -1;
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
	
	return the_window->avail_title_width_;
}



// **** Private RENDER functions *****


// draws or redraws the structure area of the windows (excluding the content area and controls)
static void Window_DrawStructure(Window* the_window)
{
	Theme*	the_theme;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
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
}


// draws or redraws the entire window, including clearing the content area
static void Window_DrawAll(Window* the_window)
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


// draws or redraws the window controls
static void Window_DrawControls(Window* the_window)
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
	
		if (this_control->invalidated_)
		{
			Control_Render(this_control);
		}
		
		this_control = this_control->next_;
	}
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
		Sys_Destroy(&global_system);	// crash early, crash often
	}
	
	available_width = the_window->avail_title_width_;
	chars_that_fit = Font_MeasureStringWidth(new_font, the_window->title_, GEN_NO_STRLEN_CAP, available_width, 0, &pixels_used);
	//DEBUG_OUT(("%s %d: available_width=%i, chars_that_fit=%i", __func__, __LINE__, available_width, chars_that_fit));
	
	if (the_window->active_)
	{
		Bitmap_SetColor(the_window->bitmap_, Theme_GetTitleColor(the_theme));
	}
	else
	{
		Bitmap_SetColor(the_window->bitmap_, Theme_GetInactiveForeColor(the_theme));
	}
	
	Bitmap_SetXY(the_window->bitmap_, the_window->titlebar_rect_.MinX + the_theme->title_x_offset_, the_window->titlebar_rect_.MinY + (the_theme->titlebar_height_ + new_font->nDescent) / 2 - 1);

	if (Font_DrawString(the_window->bitmap_, the_window->title_, chars_that_fit) == false)
	{
	}

	if (Bitmap_SetFont(the_window->bitmap_, old_font) == false)
	{
		DEBUG_OUT(("%s %d: Couldn't set the bitmap's font back to what it had been", __func__, __LINE__));
		Sys_Destroy(&global_system);	// crash early, crash often
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
	
	if ( (the_window = (Window*)f_calloc(1, sizeof(Window), MEM_STANDARD) ) == NULL)
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
	the_window->global_rect_.MinX = the_window->x_;
	the_window->global_rect_.MaxX = the_window->x_ + the_window->width_;
	the_window->global_rect_.MinY = the_window->y_;
	the_window->global_rect_.MaxY = the_window->y_ + the_window->height_;
	the_window->min_width_ = the_win_template->min_width_;
	the_window->min_height_ = the_win_template->min_height_;
	the_window->max_width_ = the_win_template->max_width_;
	the_window->max_height_ = the_win_template->max_height_;

	the_window->user_data_ = the_win_template->user_data_;
	the_window->buffer_bitmap_ = the_win_template->buffer_bitmap_;
	the_window->show_iconbar_ = the_win_template->show_iconbar_;
	the_window->is_backdrop_ = the_win_template->is_backdrop_;
	the_window->can_resize_ = the_win_template->can_resize_;
	the_window->clip_rect_ = NULL;
	the_window->event_handler_ = event_handler;

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
}







// **** CLIP RECT MANAGEMENT functions *****

//! Remove and free any clip rects attached to the window
void Window_ClearClipRects(Window* the_window)
{
	ClipRect*	the_clip;
	ClipRect*	next_clip;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	//DEBUG_OUT(("%s %d: reached", __func__, __LINE__));
	
	the_clip = the_window->clip_rect_;
	
	while (the_clip)
	{
		next_clip = the_clip->next_;
		
		DEBUG_OUT(("%s %d: clearing cliprect %p (%i, %i -- %i, %i)", __func__, __LINE__, the_clip, the_clip->x_, the_clip->y_, the_clip->width_, the_clip->height_));
		
		f_free(the_clip, MEM_STANDARD);
		
		the_clip = next_clip;
	}
	
	the_window->clip_rect_ = NULL;
	
	return;
}


//! Copy the passed rectangle to the window's clip rect collection
bool Window_AddClipRect(Window* the_window, Rectangle* new_rect)
{
	ClipRect*	first_clip;
	ClipRect*	the_clip;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	if ( new_rect == NULL)
	{
		LOG_ERR(("%s %d: passed rect was null", __func__ , __LINE__));
		return false;
	}
	
	if ( (the_clip = (ClipRect*)f_calloc(1, sizeof(ClipRect), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new ClipRect", __func__ , __LINE__));
		return false;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_clip	%p	size	%i", __func__ , __LINE__, the_clip, sizeof(ClipRect)));

	// LOGIC:
	//   controls / etc pass on their window-local coords
	//   but to blit to screen, we need to correct to global coords

// 	the_clip->rect_.MinX = new_rect->MinX + the_window->x_;
// 	the_clip->rect_.MaxX = new_rect->MaxX + the_window->x_;
// 	the_clip->rect_.MinY = new_rect->MinY + the_window->y_;
// 	the_clip->rect_.MaxY = new_rect->MaxY + the_window->y_;
	the_clip->x_ = new_rect->MinX;
	the_clip->y_ = new_rect->MinY;
	the_clip->width_ = new_rect->MaxX - new_rect->MinX;
	the_clip->height_ = new_rect->MaxY - new_rect->MinY;
	
	//DEBUG_OUT(("%s %d: reached", __func__, __LINE__));
	
	first_clip = the_window->clip_rect_;
	
	if (first_clip != NULL)
	{
		// one or more clips exist, we will insert ours at the head of the list
		the_clip->next_ = first_clip;
	}
	else
	{
		the_clip->next_ = NULL;
	}
	
	the_window->clip_rect_ = the_clip;
	
	return true;
}


//! Merge and de-duplicate clip rects
//! Consolidating the clip rects will happen reduce unnecessary reblitting
//bool Window_MergeClipRects(Window* the_window);
// need to study more before trying to implement this; looks hairy. 


//! Blit each clip rect to the screen, and clear all clip rects when done
//! This is the actual mechanics of rendering the window to the screen
bool Window_BlitClipRects(Window* the_window)
{
	ClipRect*	the_clip;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	the_clip = the_window->clip_rect_;
	
	while (the_clip)
	{
		DEBUG_OUT(("%s %d: blitting cliprect %p (%i, %i -- %i, %i)", __func__, __LINE__, the_clip, the_clip->x_, the_clip->y_, the_clip->width_, the_clip->height_));
	
		//Bitmap_BlitRect(the_window->bitmap_, the_clip->rect_, global_system->screen_[ID_CHANNEL_B]->bitmap_, the_window->x_, the_window->y_);
		//Bitmap_BlitRect(the_window->bitmap_, the_window->overall_rect_, global_system->screen_[ID_CHANNEL_B]->bitmap_, the_window->x_, the_window->y_);
		Bitmap_Blit(the_window->bitmap_, 
					the_clip->x_, 
					the_clip->y_, 
					global_system->screen_[ID_CHANNEL_B]->bitmap_, 
					the_clip->x_ + the_window->x_, 
					the_clip->y_ + the_window->y_, 
					the_clip->width_, 
					the_clip->height_
					);
		
		the_clip = the_clip->next_;
	}
	
	// LOGIC: 
	//   clip rects are one-time usage: once we have blitted them, we never want to blit them again
	//   we want to clear the decks for the next set of updates
	
	Window_ClearClipRects(the_window);
	
	return true;
}






// **** BUILT-IN PSEUDO CONTROL MANAGEMENT functions *****

//! Checks if the passed coordinate is within one of the draggable event zones
//! Draggable event zones include the title bar, 4 single-direction resize zones on the window edges, and the lower-right traditional resize zone
//! @param	the_window: reference to a valid Window object.
//! @param	x: window-local horizontal coordinate
//! @param	y: window-local vertical coordinate
//@ return	Returns mouseFree if the coordinates are in anything but a draggable region. Otherwise returns mouseResizeUp, etc., as appropriate.
mouse_mode Window_CheckForDragZone(Window* the_window, int16_t x, int16_t y)
{
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return mouseFree;
	}
	
	if (General_PointInRect(x, y, the_window->titlebar_rect_) == true)
	{
		return mouseDragTitle;
	}
	else if (General_PointInRect(x, y, the_window->grow_bottom_right_rect_) == true)
	{
		return mouseResizeDownRight;
	}
	else if (General_PointInRect(x, y, the_window->grow_right_rect_) == true)
	{
		return mouseResizeRight;
	}
	else if (General_PointInRect(x, y, the_window->grow_bottom_rect_) == true)
	{
		return mouseResizeDown;
	}
	else if (General_PointInRect(x, y, the_window->grow_left_rect_) == true)
	{
		return mouseResizeLeft;
	}
	else if (General_PointInRect(x, y, the_window->grow_top_rect_) == true)
	{
		return mouseResizeUp;
	}
	
	return mouseFree;
}




// **** CONTROL MANAGEMENT functions *****

bool Window_SetControlState(Window* the_window, uint16_t the_control_id);


//! Set the passed control as the currently selected control and unselect any previously selected control
//! @return	Returns false on any error
bool Window_SetSelectedControl(Window* the_window, Control* the_control)
{
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	if (the_window->selected_control_ != NULL)
	{
		Control_SetPressed(the_window->selected_control_, false);
	}
	
	the_window->selected_control_ = the_control;
	Control_SetPressed(the_control, true);
	
	return true;
}


//! Add the passed control to the window's list of controls
//! @return	Returns false in any error condition
bool Window_AddControl(Window* the_window, Control* the_control)
{
	Control*	last_control_in_window;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	if ( the_control == NULL)
	{
		LOG_ERR(("%s %d: passed control was null", __func__ , __LINE__));
		return false;
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
}


//! Instantiate a new control from the passed template, and add it to the window's list of controls
//! @return	Returns a pointer to the new control, or NULL in any error condition
Control* Window_AddNewControlFromTemplate(Window* the_window, ControlTemplate* the_template, uint16_t the_id, uint16_t group_id)
{
	Control*			the_control;
	
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	if ( the_template == NULL)
	{
		LOG_ERR(("%s %d: passed control template was null", __func__ , __LINE__));
		return NULL;
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
}


//! Instantiate a new control of the type specified, and add it to the window's list of controls
//! @return	Returns a pointer to the new control, or NULL in any error condition
Control* Window_AddNewControl(Window* the_window, control_type the_type, int16_t width, int16_t height, int16_t x_offset, int16_t y_offset, h_align_type the_h_align, v_align_type the_v_align, char* the_caption, uint16_t the_id, uint16_t group_id)
{
	Theme*				the_theme;
	Control*			the_control;
	ControlTemplate*	the_template;
	
	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
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
}


Control* Window_GetRootControl(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	return the_window->root_control_;
}


//! Get the control listed as the currently selected control.
//! @return	Returns a control pointer, or NULL on any error, or if there is no selected control currently
Control* Window_GetSelectedControl(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	return the_window->selected_control_;
}


//! Find and return the last control in the window's chain of controls
//! This corresponds to the first control with a NULL value for next_
Control* Window_GetLastControl(Window* the_window)
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
		if (the_control->next_ == NULL)
		{
			return the_control;
		}
		
		the_control = the_control->next_;
	}
	
	return NULL;
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


//! Find the Control under the mouse
Control* Window_GetControlAtXY(Window* the_window, int16_t x, int16_t y)
{
 	Control*	the_control;

	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
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

	if (the_window->visible_ == false)
	{
		return;
	}
	
	if (the_window->is_backdrop_ && the_window->invalidated_ == true)
	{
		// backdrop window: fill it with its pattern. no controls, borders, etc. 
		// tile the default theme's background pattern
		Bitmap_Tile(the_pattern, 0, 0, the_window->bitmap_, the_theme->pattern_width_, the_theme->pattern_height_);
		the_window->invalidated_ = false;
		
		return;
	}
	else
	{
		// non-backdrop window: render borders, titlebars, controls, etc, as appropriate

		// Re-draw titlebar if invalidated and queue for render
		// also covers the overall outline of the window if window itself is invalidated
		if (the_window->invalidated_ == true || the_window->titlebar_invalidated_ == true)
		{
			Window_DrawStructure(the_window);
		}

		// Re-draw any controls that were invalidated and queue them for render
		Window_DrawControls(the_window);
	}

	// blit to screen
	//Bitmap_BlitRect(the_window->bitmap_, the_window->overall_rect_, global_system->screen_[ID_CHANNEL_B]->bitmap_, the_window->x_, the_window->y_);
	//Window_AddClipRect(the_window, &the_window->overall_rect_);
	
	// if the entire window has had to be redrawn, then don't bother with individual cliprects, just do one for entire window
	if (the_window->invalidated_ == true)
	{
		Window_ClearClipRects(the_window);
		//Window_AddClipRect(the_window, &the_window->overall_rect_);
		Bitmap_BlitRect(the_window->bitmap_, the_window->overall_rect_, global_system->screen_[ID_CHANNEL_B]->bitmap_, the_window->x_, the_window->y_);
		the_window->invalidated_ = false;
	}
	else
	{
		Window_BlitClipRects(the_window);
	}
	
}


//! Updates the current window controls, etc., to match the current system theme 
void Window_UpdateTheme(Window* the_window)
{
	// adjust the rects for titlebar, content, etc., as each theme can change the position, height, etc. 
	Window_ConfigureStructureRects(the_window);

	// have all controls update themselves
	Window_UpdateControlTheme(the_window);
	
	// recalculate available title width
	Window_CalculateTitleSpace(the_window);
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


//! Set the display order of the window
//! Consider this a system-only function: do not use this
void Window_SetDisplayOrder(Window* the_window, int8_t the_display_order)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	the_window->display_order_ = the_display_order;
}


//! Set the passed window's active flag.
void Window_SetActive(Window* the_window, bool is_active)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	the_window->active_ = is_active;
	the_window->titlebar_invalidated_ = true;
}




// **** Get functions *****



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


int16_t Window_GetX(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->x_;
}


int16_t Window_GetY(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->y_;
}


int16_t Window_GetWidth(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_window->width_;
}


int16_t Window_GetHeight(Window* the_window)
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


// Get the active/inactive condition of the window
window_state Window_IsActive(Window* the_window)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return WIN_UNKNOWN_STATE;
	}
	
	return the_window->active_;
}





// **** DRAWING functions *****


//! Convert the passed x, y global coordinates to local (to window) coordinates
void Window_GlobalToLocal(Window* the_window, int16_t* x, int16_t* y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	*x = *x - the_window->x_;
	*y = *y - the_window->y_;
}


//! Convert the passed x, y local coordinates to global coordinates
void Window_LocalToGlobal(Window* the_window, int16_t* x, int16_t* y)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	*x = *x + the_window->x_;
	*y = *y + the_window->y_;
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
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	if (the_font == NULL)
	{
		LOG_WARN(("%s %d: passed font was null", __func__ , __LINE__));
		return false;
	}
	
	the_window->pen_font_ = the_font;
	the_window->bitmap_->font_ = the_font;
	
	return true;
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
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	//DEBUG_OUT(("%s %d: window global rect: %i, %i - %i, %i", __func__ , __LINE__, the_window->global_rect_.MinX, the_window->global_rect_.MinY, the_window->global_rect_.MaxX, the_window->global_rect_.MaxY));
	//DEBUG_OUT(("%s %d: x/y before making local: %i, %i", __func__ , __LINE__, x, y));
	x -= the_window->x_;
	y -= the_window->y_;
	//DEBUG_OUT(("%s %d: x/y after making local: %i, %i", __func__ , __LINE__, x, y));
	//DEBUG_OUT(("%s %d: content rect: %i, %i - %i, %i", __func__ , __LINE__, the_window->content_rect_.MinX, the_window->content_rect_.MinY, the_window->content_rect_.MaxX, the_window->content_rect_.MaxY));
	
	return Window_SetPenXY(the_window, x, y);
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
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_Blit(src_bm, src_x, src_y, the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height);
}


//! Fill a rectangle drawn from the current pen location, for the passed width/height
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_FillBox(Window* the_window, int16_t width, int16_t height, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_FillBox(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height, the_color);
}


//! Fill pixel values for the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_FillBoxRect(Window* the_window, Rectangle* the_coords, uint8_t the_color)
{
	int16_t		x1;
	int16_t		y1;
	int16_t		x2;
	int16_t		y2;
	
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
bool Window_SetPixel(Window* the_window, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_SetPixelAtXY(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, the_color);
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
//! @param	the_line_len: The total length of the line, in pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawHLine(Window* the_window, int16_t the_line_len, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawHLine(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, the_line_len, the_color);
}


//! Draws a vertical line from specified coords, for n pixels
//! @param	the_window: reference to a valid Window object.
//! @param	the_line_len: The total length of the line, in pixels
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawVLine(Window* the_window, int16_t the_line_len, uint8_t the_color)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawVLine(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, the_line_len, the_color);
}


//! Draws a rectangle based on the passed Rectangle object, using the specified LUT value
//! @param	the_window: reference to a valid Window object.
//! @param	the_coords: the starting and ending coordinates within the content area of the window
//! @param	the_color: a 1-byte index to the current LUT
//! @return	returns false on any error/invalid input.
bool Window_DrawBoxRect(Window* the_window, Rectangle* the_coords, uint8_t the_color)
{
	int16_t		x1;
	int16_t		y1;
	int16_t		x2;
	int16_t		y2;
	
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
bool Window_DrawBoxCoords(Window* the_window, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t the_color)
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
bool Window_DrawBox(Window* the_window, int16_t width, int16_t height, uint8_t the_color, bool do_fill)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawBox(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height, the_color, do_fill);
}


//! Draws a rounded rectangle from the current pen location, with the specified size and radius, and optionally fills the rectangle.
//! @param	the_window: reference to a valid Window object.
//! @param	width: width, in pixels, of the rectangle to be drawn
//! @param	height: height, in pixels, of the rectangle to be drawn
//! @param	radius: radius, in pixels, of the arc to be applied to the rectangle's corners. Minimum 3, maximum 20.
//! @param	the_color: a 1-byte index to the current color LUT
//! @param	do_fill: If true, the box will be filled with the provided color. If false, the box will only draw the outline.
//! @return	returns false on any error/invalid input.
bool Window_DrawRoundBox(Window* the_window, int16_t width, int16_t height, int16_t radius, uint8_t the_color, bool do_fill)
{
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawRoundBox(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, width, height, radius, the_color, do_fill);
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
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Bitmap_DrawCircle(the_window->bitmap_, the_window->pen_x_, the_window->pen_y_, radius, the_color);
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
		Sys_Destroy(&global_system); // crash early, crash often
	}
	
	return Font_DrawString(the_window->bitmap_, the_string, max_chars);
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
//! @return	returns a pointer to the first character in the string after which it stopped processing (if string is too long to be displayed in its entirety). Returns the original string if the entire string was processed successfully. Returns NULL in the event of any error.
char* Window_DrawStringInBox(Window* the_window, int16_t width, int16_t height, char* the_string, int16_t num_chars, char** wrap_buffer, bool (* continue_function)(void))
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




