
/*
 * startup.h
 *
*  Created on: Apr 12, 2022
 *      Author: micahbly
 */

#ifndef STARTUP_H_
#define STARTUP_H_


/* about this class: Startup
 *
 * Some flashy stuff to show on the screen when starting up the system. eye candy.
 *
 *** things this class needs to be able to do
 * 
 * clear the screen to all black
 * load a custom CLUT
 * load and display a logo graphic in center of screen
 * show a progress bar
 * flash meaningless messages under the progress bar. ideally random. 
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
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
#include <mb/bitmap.h>
#include <mb/window.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define SPLASH_WIDTH	272
#define SPLASH_HEIGHT	400

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/





/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****






// **** xxx functions *****

bool Startup_ShowSplash(void);



// **** Set xxx functions *****




// **** Get xxx functions *****





// **** xxx functions *****




// **** xxx functions *****




#endif /* STARTUP_H_ */


