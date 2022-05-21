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


// C includes
#include <stdbool.h>


// A2560 includes
#include <mcp/syscalls.h>
#include "a2560_platform.h"
#include "general.h"
#include "text.h"
#include "bitmap.h"
#include "window.h"


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
	int16_t					x_offset_;						//! horizontal coordinate relative to the parent window's left or right edge. If h_align_ is H_ALIGN_CENTER, this value will be ignored.
	int16_t					y_offset_;						//! vertical coordinate relative to the parent window's top or bottom edge. If v_align_ is V_ALIGN_CENTER, this value will be ignored.
	int16_t					width_;							//! width of the control
	int16_t					height_;						//! height of the control
	int16_t					min_;							//! minimum allowed value
	int16_t					max_;							//! maximum allowed value
	Bitmap*					image_[2][2];					//! 4 image state bitmaps: [active yes/no][pushed down yes/no]
	char*					caption_;						//! optional string to draw centered horizontally and vertically on the control. Typical use case is for a button.
	uint16_t				avail_text_width_;				//! number of pixels available for writing text. For flexible width buttons, etc., this excludes the left/right segments. 
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
bool ControlTemplate_Destroy(ControlTemplate** the_template);






// **** xxx functions *****




// **** Set xxx functions *****




// **** Get xxx functions *****





// **** xxx functions *****




// **** xxx functions *****


// **** Debug functions *****

void ControlTemplate_Print(ControlTemplate* the_template);


#endif /* CONTROL_TEMPLATE_H_ */


