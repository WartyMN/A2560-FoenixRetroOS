//! @file theme.h

/*
 * theme.h
 *
*  Created on: Mar 26, 2022
 *      Author: micahbly
 */

#ifndef LIB_THEME_H_
#define LIB_THEME_H_


/* about this class: Theme
 *
 * This provides functionality for working with visual themes for windows
 * 
 *** things this library needs to be able to do
 * Provide system default theme control templates for close, minimize, normal size, and maximize buttons
 * Provide system default font objects for app font and system font
 * Read a theme from disk, and make it the global theme
 * Provide information about the current global theme
 * Provide the control templates, colors, desktop patterns, etc., from the theme object
 * Allow a calling method to update the control templates, colors, etc, of the current theme object??? TBD
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
#include <mb/lib_sys.h>
#include <mb/window.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define WIN_DEFAULT_OUTLINE_COLOR		SYS_DEF_COLOR_WINFRAME
#define WIN_DEFAULT_OUTLINE_SIZE		1
#define WIN_DEFAULT_TITLEBAR_HEIGHT		18
#define WIN_DEFAULT_TITLEBAR_Y			1
#define WIN_DEFAULT_TITLEBAR_COLOR		SYS_DEF_COLOR_WINTITLE_BACK
#define WIN_DEFAULT_ICONBAR_HEIGHT		16
#define WIN_DEFAULT_ICONBAR_Y			WIN_DEFAULT_TITLEBAR_Y + WIN_DEFAULT_TITLEBAR_HEIGHT
#define WIN_DEFAULT_ICONBAR_COLOR		SYS_DEF_COLOR_ICONBAR_BACK
#define WIN_DEFAULT_CONTENTAREA_Y		WIN_DEFAULT_ICONBAR_Y + WIN_DEFAULT_ICONBAR_HEIGHT
#define WIN_DEFAULT_CONTENTAREA_COLOR	SYS_DEF_COLOR_CONTENT_BACK
#define WIN_DEFAULT_DESKTOP_COLOR		SYS_DEF_COLOR_DESKTOP


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

struct Theme
{
	Font*					icon_font_;
	Font*					control_font_;					//! Font for controls (incuding titlebar)
	uint8_t*				clut_;
	uint8_t					outline_size_;					//! Thickness of border around window, in pixels. 0 is acceptable. Border is drawn from window rect inwards (not outwards)
	uint8_t					outline_color_;					//! Index to the color LUT
	uint8_t					titlebar_height_;				//! Height of titlebar. Cannot be smaller than height of control font.
	signed int				titlebar_y_;					//! Positive numbers position the titlebar relative to top edge of window. Negative numbers position it relative to the bottom edge of the window.
	uint8_t					titlebar_color_;				//! Index to the color LUT	
	uint8_t					iconbar_height_;				//! Height of iconbar (when displayed).
	signed int				iconbar_y_;						//! Positive numbers position the iconbar relative to top edge of window. Negative numbers position it relative to the bottom edge of the window.
	uint8_t					iconbar_color_;					//! Index to the color LUT
	signed int				contentarea_y_;					//! Positive numbers position the content area relative to top edge of window. Negative numbers position it relative to the bottom edge of the window.	
	uint8_t					contentarea_color_;				//! Index to the color LUT
	uint8_t					desktop_color_;					//! Required LUT index for the background color; Used when no pattern bitmap
	Bitmap*					desktop_pattern_;				//! Optional bitmap to be tiled into a desktop pattern
	ControlTemplate*		control_t_close_;
	ControlTemplate*		control_t_minimize_;
	ControlTemplate*		control_t_norm_size_;
	ControlTemplate*		control_t_maximize_;
};



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate a Theme object
Theme* Theme_New(void);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
boolean Theme_Destroy(Theme** the_theme);



//! create default Theme.
//! used in cases where a custom theme is not specified or is not available
Theme* Theme_CreateDefaultTheme(void);






// **** xxx functions *****




// **** Set xxx functions *****





// **** Get xxx functions *****

ControlTemplate* Theme_GetCloseControlTemplate(Theme* the_theme);
ControlTemplate* Theme_GetMinimizeControlTemplate(Theme* the_theme);
ControlTemplate* Theme_GetNormSizeControlTemplate(Theme* the_theme);
ControlTemplate* Theme_GetMaximizeControlTemplate(Theme* the_theme);



// **** xxx functions *****




// **** xxx functions *****





// **** Debug functions *****

void Theme_Print(Theme* the_theme);



#endif /* LIB_THEME_H_ */


