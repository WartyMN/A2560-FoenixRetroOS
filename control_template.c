/*
 * control_template.c
 *
 *  Created on: Mar 26, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "control_template.h"

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
#include <mb/window.h>
#include <mb/memory_manager.h>


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



// **** Debug functions *****

void ControlTemplate_Print(ControlTemplate* the_template);

/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/



// **** Debug functions *****

void ControlTemplate_Print(ControlTemplate* the_template)
{
	DEBUG_OUT(("ControlTemplate print out:"));
	DEBUG_OUT(("  type_: %i", the_template->type_));
	DEBUG_OUT(("  h_align_: %i", the_template->h_align_));
	DEBUG_OUT(("  v_align_: %i", the_template->v_align_));
	DEBUG_OUT(("  x_offset_: %i", the_template->x_offset_));
	DEBUG_OUT(("  y_offset_: %i", the_template->y_offset_));
	DEBUG_OUT(("  width_: %i", the_template->width_));
	DEBUG_OUT(("  height_: %i", the_template->height_));
	DEBUG_OUT(("  min_: %i", the_template->min_));
	DEBUG_OUT(("  max_: %i", the_template->max_));
	DEBUG_OUT(("  image_inactive_: %p", the_template->image_inactive_));
	DEBUG_OUT(("  image_active_up_: %p", the_template->image_active_up_));
	DEBUG_OUT(("  image_active_down_: %p", the_template->image_active_down_));
	DEBUG_OUT(("  caption_: %p", the_template->caption_));	
}




/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Create a new Control Template object
ControlTemplate* ControlTemplate_New(void)
{
	ControlTemplate*	the_template;
	
	if ( (the_template = (ControlTemplate*)f_calloc(1, sizeof(ControlTemplate), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new ControlTemplate", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_template	%p	size	%i", __func__ , __LINE__, the_template, sizeof(ControlTemplate)));

		
	return the_template;
	
error:
	if (the_template)					ControlTemplate_Destroy(&the_template);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool ControlTemplate_Destroy(ControlTemplate** the_template)
{
	if (*the_template == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	LOG_ALLOC(("%s %d:	__FREE__	*the_template	%p	size	%i", __func__ , __LINE__, *the_template, sizeof(ControlTemplate)));
	f_free(*the_template, MEM_STANDARD);
	*the_template = NULL;
	
	return true;
}





// **** xxx functions *****




// **** Set xxx functions *****





// **** Get xxx functions *****





// **** xxx functions *****




