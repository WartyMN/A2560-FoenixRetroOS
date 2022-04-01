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
	//   rect_ values are relative to the window 0,0 (upper left)
	
	if (the_control->h_align_ == H_ALIGN_LEFT)
	{
		the_control->rect_.MinX = the_control->x_offset_;
	}
	else if (the_control->h_align_ == H_ALIGN_RIGHT)
	{
		the_control->rect_.MinX = the_control->parent_->overall_rect_.MaxX - the_control->x_offset_;
	}
	else
	{
		// center
		the_control->rect_.MinX = the_control->parent_->overall_rect_.MinX + ((the_control->parent_->width_ - the_control->width_) / 2);		
	}
	
	the_control->rect_.MaxX = the_control->rect_.MinX + the_control->width_;
	
	if (the_control->v_align_ == V_ALIGN_TOP)
	{
		the_control->rect_.MinY = the_control->y_offset_;
	}
	else if (the_control->v_align_ == V_ALIGN_BOTTOM)
	{
		the_control->rect_.MinY = the_control->parent_->overall_rect_.MaxY - the_control->y_offset_;
	}
	else
	{
		// center
		the_control->rect_.MinY = the_control->parent_->overall_rect_.MinY + ((the_control->parent_->height_ - the_control->height_) / 2);		
	}
	
	the_control->rect_.MaxY = the_control->rect_.MinY + the_control->height_;	
}




// **** Debug functions *****

void Control_Print(Control* the_control)
{
	DEBUG_OUT(("Control print out:"));
	DEBUG_OUT(("  id_: %u",	 				the_control->id_));
	DEBUG_OUT(("  type_: %i",	 			the_control->type_));
	DEBUG_OUT(("  group_: %i",	 			the_control->group_));
	DEBUG_OUT(("  next_: %p",				the_control->next_));
	DEBUG_OUT(("  parent_: %p",				the_control->parent_));
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
	DEBUG_OUT(("  value_: %i",	 			the_control->value_));
	DEBUG_OUT(("  min_: %i",	 			the_control->min_));
	DEBUG_OUT(("  max_: %i", 				the_control->max_));
	DEBUG_OUT(("  image_inactive_: %p",		the_control->image_inactive_));
	DEBUG_OUT(("  image_active_up_: %p", 	the_control->image_active_up_));
	DEBUG_OUT(("  image_active_down_: %p",	the_control->image_active_down_));
	DEBUG_OUT(("  caption_: %p", 			the_control->caption_));	
}



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
Control* Control_New(ControlTemplate* the_template, Window* the_window, uint16_t the_id, int8_t the_group)
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
		
	// LOGIC:
	//   the control template will contain most of the information needed to establish the Control
	//   to personalize the control for a given window, the parent window is needed
	//   the final location of the control is calculated based on the offset info in the template + the size of the parent window
	
	if ( (the_control = (Control*)calloc(1, sizeof(Control)) ) == NULL)
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
	the_control->image_inactive_ = the_template->image_inactive_;
	the_control->image_active_up_ = the_template->image_active_up_;
	the_control->image_active_down_ = the_template->image_active_down_;
	the_control->caption_ = the_template->caption_;
	
	// at start, all new controls are inactive, value 0, disabled, and invisible
	the_control->visible_ = false;
	the_control->active_ = false;
	the_control->enabled_ = false;
	the_control->value_ = 0;
	
	// localize to the parent window
	the_control->id_ = the_id;
	the_control->parent_ = the_window;
	Control_AlignToWindow(the_control);
	
	Control_Print(the_control);
	
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
	free(*the_control);
	*the_control = NULL;
	
	return true;
}





// **** xxx functions *****




// **** Set xxx functions *****

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




// **** Get xxx functions *****





// **** Render functions *****

void Control_Render(Control* the_control)
{
	Bitmap*		the_bitmap;
	
	if (the_control == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
	}
	
	if (the_control->parent_ == NULL)
	{
		LOG_ERR(("%s %d: control's parent window object was null", __func__ , __LINE__));
		return;
	}
	
	// LOGIC: 
	//   For an individual control, "render" consists of blitting it's bitmap onto the parent window's bitmap
	//   Depending on the state of the control, one of 3 bitmaps will be blitted to the parent window
	//   If control is set to invisible, none will be rendered
	
	if (!the_control->visible_)
	{
		return;
	}
	
	if (!the_control->active_)
	{
		the_bitmap = the_control->image_inactive_;
	}
	else if (the_control->active_)
	{
		the_bitmap = the_control->image_active_down_;
	}
	else
	{
		the_bitmap = the_control->image_active_up_;
	}

	DEBUG_OUT(("%s %d: about to blit control %p to parent window bitmap", __func__, __LINE__, the_control));
	DEBUG_OUT(("%s %d: pbitmap w/h=%i, %i; this MinX/MinY=%i, %i", __func__, __LINE__, the_control->parent_->bitmap_->width_, the_control->parent_->bitmap_->height_, the_control->rect_.MinX, the_control->rect_.MinY));
	
	Bitmap_Blit(the_bitmap, 0, 0, 
						the_control->parent_->bitmap_, 
						the_control->rect_.MinX, 
						the_control->rect_.MinY, 
						the_control->width_, 
						the_control->height_
						);
}



