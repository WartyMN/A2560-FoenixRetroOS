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



/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

//! Align the control to its parent window
//! No checking done on inputs
void Control_AlignToWindow(Control* the_control);


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
	
	Bitmap_Blit(the_bitmap, 0, 0, 
				the_control->parent_win_->bitmap_, 
				the_control->rect_.MinX, 
				the_control->rect_.MinY, 
				the_control->width_, 
				the_control->height_
				);
}



