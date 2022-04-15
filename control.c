/*
 * control.c
 *
 *  Created on: Mar 20, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "window.h"
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

//! Align the control to its parent window
//! No checking done on inputs
void Control_AlignToWindow(Control* the_control);

//! Draws the control's caption text into the control's available space using the parent window's bitmaps' current font
//! If the caption cannot fit in its entirety, it will be truncated
//! This is a private function; it is the calling function's responsibility to ensure the window's bitmap is set to the desired font before calling.
//! @param	the_control: a valid pointer to a Control with a non-NULL caption
static void Control_DrawCaption(Control* the_control);


// **** Debug functions *****

void Control_Print(Control* the_control);

/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

//! Align the control to its parent window
//! No checking done on inputs
void Control_AlignToWindow(Control* the_control)
{
	// LOGIC:
	//   Controls have an h align and v align choice, and x and y offsets.
	//   Based on those choices, and the parent window's dimensions, 
	//     the control needs its' rect_ property adjusted to match where it should render
	//   rect_ values are relative to the control's parent window, but calculated relative to the parent rect
	//     for close/max/min/norm, this will be the window's titlebar rect
	//       (not to the window itself -- this allows them to render at bottom of window if that's where titlebar is)
	//     for iconbar controls, that will be the iconbar_rect_
	//     for any other control, that will be the contentarea_rect_
	
	if (the_control->h_align_ == H_ALIGN_LEFT)
	{
		the_control->rect_.MinX = the_control->parent_rect_->MinX + the_control->x_offset_;
	}
	else if (the_control->h_align_ == H_ALIGN_RIGHT)
	{
		the_control->rect_.MinX = the_control->parent_rect_->MaxX - the_control->x_offset_;
	}
	else
	{
		// center
		the_control->rect_.MinX = the_control->parent_rect_->MinX + ((the_control->parent_rect_->MaxX - the_control->parent_rect_->MinX - the_control->width_) / 2);		
	}
	
	the_control->rect_.MaxX = the_control->rect_.MinX + the_control->width_;
	
	if (the_control->v_align_ == V_ALIGN_TOP)
	{
		the_control->rect_.MinY = the_control->parent_rect_->MinY + the_control->y_offset_;
	}
	else if (the_control->v_align_ == V_ALIGN_BOTTOM)
	{
		the_control->rect_.MinY = the_control->parent_rect_->MaxY - the_control->y_offset_;
	}
	else
	{
		// center
		the_control->rect_.MinY = the_control->parent_rect_->MinY + ((the_control->parent_rect_->MaxY - the_control->parent_rect_->MinY - the_control->height_) / 2);		
	}
	
	the_control->rect_.MaxY = the_control->rect_.MinY + the_control->height_;
	
	//DEBUG_OUT(("%s %d: Control after AlignToWindow...", __func__, __LINE__));
	//Control_Print(the_control);
}


//! Draws the control's caption text into the control's available space using the parent window's bitmaps' current font
//! If the caption cannot fit in its entirety, it will be truncated
//! This is a private function; it is the calling function's responsibility to ensure the window's bitmap is set to the desired font before calling.
//! @param	the_control: a valid pointer to a Control with a non-NULL caption
static void Control_DrawCaption(Control* the_control)
{
	Theme*		the_theme;
	Font*		the_font;
	Font*		old_font;
	int16_t		available_width;
	int16_t		x_offset;
	int16_t		x;
	int16_t		y;
	int16_t		chars_that_fit;
	signed int	pixels_used;
	uint8_t		font_color;

	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		Sys_Destroy(&global_system); // crash early, crash often
	}

	// Draw control caption with parent window's current font. 
// 	// Assumption is that all controls with text are going to be rendered one after another, so getting/setting font each time is wasteful.
// 	//   If this assumption proves to be false, re-enable the font selection here. 
	// caption is to be drawn centered vertically within the control Rect
	// caption is to be clipped if too long to fit horizontally

	the_theme = Sys_GetTheme(global_system);
	the_font = Sys_GetSystemFont(global_system);

	if (Bitmap_SetFont(the_control->parent_win_->bitmap_, the_font) == false)
	{
		DEBUG_OUT(("%s %d: Couldn't get the system font and assign it to bitmap", __func__, __LINE__));
		Sys_Destroy(&global_system);	// crash early, crash often
	}
	
	available_width = the_control->avail_text_width_;	
	chars_that_fit = Font_MeasureStringWidth(the_font, the_control->caption_, GEN_NO_STRLEN_CAP, available_width, 0, &pixels_used);
	//DEBUG_OUT(("%s %d: available_width=%i, chars_that_fit=%i", __func__, __LINE__, available_width, chars_that_fit));

	x_offset = the_control->rect_.MinX + (the_control->width_ - the_control->avail_text_width_) / 2; // potentially, this could be problematic if a theme designer set up a theme with right width 10, left width 2. 
	x = x_offset + (available_width - pixels_used) / 2;
	y = the_control->rect_.MinY + (the_control->rect_.MaxY - the_control->rect_.MinY + the_font->nDescent) / 2 - 1;
	//DEBUG_OUT(("%s %d: available_width=%i, x_offset=%i, x=%i, y=%i", __func__, __LINE__, available_width, x_offset, x, y));

	if (the_control->active_)
	{
		if (the_control->pressed_)
		{
			font_color = the_theme->standard_back_color_; // fore/back colors expected to be inversed when pressed.
		}
		else
		{
			font_color = the_theme->standard_fore_color_;
		}
	}
	else
	{
		if (the_control->pressed_)
		{
			font_color = the_theme->highlight_fore_color_;
		}
		else
		{
			font_color = the_theme->inactive_fore_color_;
		}
	}
	
	Bitmap_SetColor(the_control->parent_win_->bitmap_, font_color);
	Bitmap_SetXY(the_control->parent_win_->bitmap_, x, y);

	if (Font_DrawString(the_control->parent_win_->bitmap_, the_control->caption_, chars_that_fit) == false)
	{
	}

// 	if (Bitmap_SetFont(the_control->parent_win_->bitmap_, old_font) == false)
// 	{
// 		DEBUG_OUT(("%s %d: Couldn't set the bitmap's font back to what it had been", __func__, __LINE__));
// 		Sys_Destroy(&global_system);	// crash early, crash often
// 	}
}




// **** Debug functions *****

void Control_Print(Control* the_control)
{
	DEBUG_OUT(("Control print out:"));
	DEBUG_OUT(("  id_: %u",	 				the_control->id_));
	DEBUG_OUT(("  type_: %i",	 			the_control->type_));
	DEBUG_OUT(("  group_: %i",	 			the_control->group_));
	DEBUG_OUT(("  next_: %p",				the_control->next_));
	DEBUG_OUT(("  parent_win_: %p",			the_control->parent_win_));
	DEBUG_OUT(("  parent_rect_: %i, %i, %i, %i",	the_control->parent_rect_->MinX, the_control->parent_rect_->MinY, the_control->parent_rect_->MaxX, the_control->parent_rect_->MaxY));
	DEBUG_OUT(("  rect_: %i, %i, %i, %i",	the_control->rect_.MinX, the_control->rect_.MinY, the_control->rect_.MaxX, the_control->rect_.MaxY));
	DEBUG_OUT(("  h_align_: %i", 			the_control->h_align_));
	DEBUG_OUT(("  v_align_: %i",			the_control->v_align_));
	DEBUG_OUT(("  x_offset_: %i",			the_control->x_offset_));
	DEBUG_OUT(("  y_offset_: %i", 			the_control->y_offset_));
	DEBUG_OUT(("  width_: %i", 				the_control->width_));
	DEBUG_OUT(("  height_: %i", 			the_control->height_));
	DEBUG_OUT(("  visible_: %i", 			the_control->visible_));
	DEBUG_OUT(("  active_: %i", 			the_control->active_));
	DEBUG_OUT(("  enabled_: %i", 			the_control->enabled_));
	DEBUG_OUT(("  pressed_: %i", 			the_control->pressed_));
	DEBUG_OUT(("  value_: %i",	 			the_control->value_));
	DEBUG_OUT(("  min_: %i",	 			the_control->min_));
	DEBUG_OUT(("  max_: %i", 				the_control->max_));
	DEBUG_OUT(("  inactive image up: %p", 	the_control->image_[CONTROL_INACTIVE][CONTROL_UP]));
	DEBUG_OUT(("  inactive image dn: %p", 	the_control->image_[CONTROL_INACTIVE][CONTROL_DOWN]));
	DEBUG_OUT(("  active image up: %p", 	the_control->image_[CONTROL_ACTIVE][CONTROL_UP]));
	DEBUG_OUT(("  active image dn: %p", 	the_control->image_[CONTROL_ACTIVE][CONTROL_DOWN]));
	DEBUG_OUT(("  caption_: %p", 			the_control->caption_));	
	DEBUG_OUT(("  avail_text_width_: %i", 	the_control->avail_text_width_));
}



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
Control* Control_New(ControlTemplate* the_template, Window* the_window, Rectangle* the_parent_rect, uint16_t the_id, int8_t the_group)
{
	Control*		the_control;

	if ( the_template == NULL)
	{
		LOG_ERR(("%s %d: passed template was NULL", __func__ , __LINE__));
		goto error;
	}

	if ( the_window == NULL)
	{
		LOG_ERR(("%s %d: passed parent window was NULL", __func__ , __LINE__));
		goto error;
	}

	if ( the_parent_rect == NULL)
	{
		LOG_ERR(("%s %d: passed parent rect was NULL", __func__ , __LINE__));
		goto error;
	}
		
	// LOGIC:
	//   the control template will contain most of the information needed to establish the Control
	//   to personalize the control for a given window, the parent window is needed
	//   the final location of the control is calculated based on the offset info in the template + the size of the parent window
	
	if ( (the_control = (Control*)f_calloc(1, sizeof(Control), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new font record", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_control	%p	size	%i", __func__ , __LINE__, the_control, sizeof(Control)));

	// copy template info in before localizing to the window
	the_control->type_ = the_template->type_;
	the_control->h_align_ = the_template->h_align_;
	the_control->v_align_ = the_template->v_align_;
	the_control->x_offset_ = the_template->x_offset_;
	the_control->y_offset_ = the_template->y_offset_;
	the_control->width_ = the_template->width_;
	the_control->height_ = the_template->height_;
	the_control->min_ = the_template->min_;
	the_control->max_ = the_template->max_;
	the_control->image_[CONTROL_INACTIVE][CONTROL_UP] = the_template->image_[CONTROL_INACTIVE][CONTROL_UP];
	the_control->image_[CONTROL_INACTIVE][CONTROL_DOWN] = the_template->image_[CONTROL_INACTIVE][CONTROL_DOWN];
	the_control->image_[CONTROL_ACTIVE][CONTROL_UP] = the_template->image_[CONTROL_ACTIVE][CONTROL_UP];
	the_control->image_[CONTROL_ACTIVE][CONTROL_DOWN] = the_template->image_[CONTROL_ACTIVE][CONTROL_DOWN];
	the_control->caption_ = the_template->caption_;
	the_control->avail_text_width_ = the_template->avail_text_width_;
	
	// at start, all new controls are inactive, value 0, disabled, not-pressed, and invisible
	the_control->visible_ = false;
	the_control->active_ = false;
	the_control->enabled_ = false;
	the_control->pressed_ = false;
	the_control->value_ = 0;
	
	// localize to the parent window
	the_control->id_ = the_id;
	the_control->parent_win_ = the_window;
	the_control->parent_rect_ = the_parent_rect;
	Control_AlignToWindow(the_control);
	
	//Control_Print(the_control);
	
	return the_control;
	
error:
	if (the_control)					Control_Destroy(&the_control);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Control_Destroy(Control** the_control)
{
	if (*the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	LOG_ALLOC(("%s %d:	__FREE__	*the_control	%p	size	%i", __func__ , __LINE__, *the_control, sizeof(Control)));
	f_free(*the_control, MEM_STANDARD);
	*the_control = NULL;
	
	return true;
}





// **** xxx functions *****

//! Updates the passed control with new theme info from the passed control template
//! Call this when the theme has been changed
//! It allows existing controls to be updated in place, without having to free them and create new theme controls
bool Control_UpdateFromTemplate(Control* the_control, ControlTemplate* the_template)
{
	if ( the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was NULL", __func__ , __LINE__));
		return false;
	}

	if ( the_template == NULL)
	{
		LOG_ERR(("%s %d: passed template was NULL", __func__ , __LINE__));
		return false;
	}

	// LOGIC:
	//   the control template will contain most of the information needed to establish the Control
	//   we already have a working control, and we don't need to change the type, visibility, active/inactive, etc. 
	//   we want to change only the properties that might have changed due to a change in theme.
	
	// copy template info in before localizing to the window
	the_control->h_align_ = the_template->h_align_;
	the_control->v_align_ = the_template->v_align_;
	the_control->x_offset_ = the_template->x_offset_;
	the_control->y_offset_ = the_template->y_offset_;
	the_control->width_ = the_template->width_;
	the_control->height_ = the_template->height_;
	the_control->avail_text_width_ = the_template->avail_text_width_;

	// do NOT free old images, they didn't really belong to the control, they belonged to the previous theme
	the_control->image_[CONTROL_INACTIVE][CONTROL_UP] = the_template->image_[CONTROL_INACTIVE][CONTROL_UP];
	the_control->image_[CONTROL_INACTIVE][CONTROL_DOWN] = the_template->image_[CONTROL_INACTIVE][CONTROL_DOWN];
	the_control->image_[CONTROL_ACTIVE][CONTROL_UP] = the_template->image_[CONTROL_ACTIVE][CONTROL_UP];
	the_control->image_[CONTROL_ACTIVE][CONTROL_DOWN] = the_template->image_[CONTROL_ACTIVE][CONTROL_DOWN];
		
	// localize to the parent window
	Control_AlignToWindow(the_control);
	
	//Control_Print(the_control);
	
	return true;
}



// **** Set functions *****

bool Control_SetNextControl(Control* the_control, Control* the_next_control)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	the_control->next_ = the_next_control;
	
	return true;
}


//! Set the control's active/inactive state
void Control_SetActive(Control* the_control, bool is_active)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	the_control->active_ = is_active;
}


//! Set the control's pressed/unpressed state
void Control_SetPressed(Control* the_control, bool is_pressed)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	the_control->pressed_ = is_pressed;
}



// **** Get functions *****


//! Get the ID of the control
//! @return	Returns the ID, or -1 in any error condition
uint16_t Control_GetID(Control* the_control)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return -1;
	}
	
	return the_control->id_;
}


//! Get the next control in the chain
//! @Return	returns NULL on any error, or if this is the last control in the chain
Control* Control_GetNextControl(Control* the_control)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}
	
	return the_control->next_;
}


//! Get the control type
//! @return	Returns CONTROL_TYPE_ERROR (-1) on any error, or the control_type value
control_type Control_GetType(Control* the_control)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return CONTROL_TYPE_ERROR;
	}
	
	return the_control->type_;
}


//! Compare the control's right-edge coordinate to the passed value
//! If the control is more to the right than the passed value, the passed value is updated with the control's right edge
//! @return	Returns true if the control is further to the right than the passed value.
bool Control_IsRighter(Control* the_control, int16_t* x)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	if (the_control->rect_.MaxX > *x)
	{
		*x = the_control->rect_.MaxX;
		return true;
	}
	
	return false;
}

//! Compare the control's left-edge coordinate to the passed value
//! If the control is more to the left than the passed value, the passed value is updated with the control's left edge
//! @return	Returns true if the control is further to the left than the passed value.
bool Control_IsLefter(Control* the_control, int16_t* x)
{
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}
	
	if (the_control->rect_.MinX < *x)
	{
		*x = the_control->rect_.MinX;
		return true;
	}
	
	return false;
}


// **** Render functions *****

void Control_Render(Control* the_control)
{
	Bitmap*		the_bitmap;
	
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	if (the_control->parent_win_ == NULL)
	{
		LOG_ERR(("%s %d: control's parent window object was null", __func__ , __LINE__));
		return;
	}
	
	// LOGIC: 
	//   For an individual control, "render" consists of blitting it's bitmap onto the parent window's bitmap
	//   Depending on the state of the control, one of 3 bitmaps will be blitted to the parent window
	//   If control is set to invisible, none will be rendered
	
	if (the_control->visible_ == false)
	{
		return;
	}
	
	the_bitmap = the_control->image_[the_control->active_][the_control->pressed_];
	//the_bitmap = the_control->image_[1][1];

	//DEBUG_OUT(("%s %d: about to blit control %p to parent window bitmap", __func__, __LINE__, the_control));
	//DEBUG_OUT(("%s %d: pbitmap w/h=%i, %i; this MinX/MinY=%i, %i", __func__, __LINE__, the_control->parent_->bitmap_->width_, the_control->parent_->bitmap_->height_, the_control->rect_.MinX, the_control->rect_.MinY));
	//DEBUG_OUT(("%s %d: control type=%i, active=%i, pressed=%i", __func__, __LINE__, the_control->type_, the_control->active_, the_control->pressed_));
	//Control_Print(the_control);
	
	Bitmap_Blit(the_bitmap, 0, 0, 
				the_control->parent_win_->bitmap_, 
				the_control->rect_.MinX, 
				the_control->rect_.MinY, 
				the_control->width_, 
				the_control->height_
				);
				
	// some controls have captions. if present, draw them directly to the parent bitmap
	// (leave the control's bitmaps clean, so text can be changed, font changed, etc.)
	if (the_control->caption_ != NULL)
	{
		Control_DrawCaption(the_control);
	}
}



