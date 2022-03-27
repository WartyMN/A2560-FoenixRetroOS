//! @file control_template.h

/*
 * control_template.h
 *
*  Created on: Mar 26, 2022
 *      Author: micahbly
 */

#ifndef CONTROL_TEMPLATE_H_
#define CONTROL_TEMPLATE_H_


/* about this class: Control Template
 *
 * Provides structures and functions for creating and destroying templates for controls such as close widgets, buttons, sliders, etc.
 *
 *** things this class needs to be able to do
 * Create a proto-control (control template)
 * Destroy a proto-control (control template)
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

// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
#include <mb/bitmap.h>
#include <mb/window.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

//! A structure that can be used to instantiate a ControlTemplate object instance in a window
//! The ControlTemplate contains just those fields that are not specific to a particular window, and the control's placement within that window
//! A typical use case would be for creating controls that will be displayed more than once on the same window, and vary only in location.
struct ControlTemplate
{
	control_type			type_;							//! button vs checkbox vs radio button, etc. 
	h_align_type			h_align_;						//! whether the control should be positioned relative to the left side, right side, or centered
	v_align_type			v_align_;						//! whether the control should be positioned relative to the top edge, bottom edge, or centered
	signed int				x_offset_;						//! horizontal coordinate relative to the parent window's left or right edge. If h_align_ is H_ALIGN_CENTER, this value will be ignored.
	signed int				y_offset_;						//! vertical coordinate relative to the parent window's top or bottom edge. If v_align_ is V_ALIGN_CENTER, this value will be ignored.
	signed int				width_;							//! width of the control
	signed int				height_;						//! height of the control
	int16_t					min_;							//! minimum allowed value
	int16_t					max_;							//! maximum allowed value
	Bitmap*					image_inactive_;				//! image of the control in inactive state. size must match the length and width defined in the rect_. If not supplied, the control will be effectively invisible.
	Bitmap*					image_active_up_;				//! image of the control when active, and not clicked/pressed. size must match the length and width defined in the rect_. If not supplied, the control will be effectively invisible.
	Bitmap*					image_active_down_;				//! image of the control when active, and clicked/depressed. size must match the length and width defined in the rect_. If not supplied, the control will be effectively invisible.
	char*					caption_;						//! optional string to draw centered horizontally and vertically on the control. Typical use case is for a button.
// 	char*					hover_text_;					//! optional string to show in help/hover-text situations
};




/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate a ControlTemplate object
ControlTemplate* ControlTemplate_New(void);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
boolean ControlTemplate_Destroy(ControlTemplate** the_template);






// **** xxx functions *****




// **** Set xxx functions *****




// **** Get xxx functions *****





// **** xxx functions *****




// **** xxx functions *****




#endif /* CONTROL_TEMPLATE_H_ */


