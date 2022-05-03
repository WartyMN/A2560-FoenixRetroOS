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

// C includes
#include <stdbool.h>


// A2560 includes
#include <mcp/syscalls.h>
#include "a2560_platform.h"
#include "general.h"
#include "text.h"
#include "bitmap.h"
#include "lib_sys.h"
#include "window.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define WIN_DEFAULT_OUTLINE_COLOR				SYS_DEF_COLOR_WINFRAME
#define WIN_DEFAULT_OUTLINE_SIZE				1
#define WIN_DEFAULT_FLOW_FROM_BOTTOM			false
#define WIN_DEFAULT_TITLEBAR_HEIGHT				18
#define WIN_DEFAULT_TITLEBAR_COLOR				SYS_DEF_COLOR_WINTITLE_BACK
#define WIN_DEFAULT_TITLEBAR_FONT_COLOR			SYS_COLOR_WHITE
#define WIN_DEFAULT_TITLEBAR_ACTIVE_ACCENT		SYS_COLOR_PURPLEBLUEHL
#define WIN_DEFAULT_TITLEBAR_INACTIVE_ACCENT	SYS_COLOR_PURPLEBLUEINACT
#define WIN_DEFAULT_TITLEBAR_HAS_OUTLINE		false
#define WIN_DEFAULT_ICONBAR_HEIGHT				16
#define WIN_DEFAULT_ICONBAR_COLOR				SYS_DEF_COLOR_ICONBAR_BACK
#define WIN_DEFAULT_ICONBAR_HAS_OUTLINE			false
#define WIN_DEFAULT_STANDARD_BACK_COLOR			SYS_DEF_COLOR_CONTENT_BACK
#define WIN_DEFAULT_STANDARD_FORE_COLOR			SYS_COLOR_GRAY10
#define WIN_DEFAULT_INACTIVE_BACK_COLOR			SYS_COLOR_WHITE
#define WIN_DEFAULT_INACTIVE_FORE_COLOR			SYS_COLOR_GRAY4
#define WIN_DEFAULT_HIGHLIGHT_BACK_COLOR		SYS_COLOR_TETRA_3
#define WIN_DEFAULT_HIGHLIGHT_FORE_COLOR		SYS_COLOR_WHITE
#define WIN_DEFAULT_DESKTOP_COLOR				SYS_DEF_COLOR_DESKTOP
#define WIN_DEFAULT_DESKTOP_WIDTH				16
#define WIN_DEFAULT_DESKTOP_HEIGHT				16
#define WIN_DEFAULT_BACKGROUND_BGRA				0xCCCCCC00;
#define WIN_DEFAULT_BORDER_BGRA					0x33333300;

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


struct ControlBackdrop
{
	Bitmap*					image_left_[2][2];			//! left side of 4 image state bitmaps: [active yes/no][pushed down yes/no]
	Bitmap*					image_mid_[2][2];			//! 4 image state bitmaps: [active yes/no][pushed down yes/no]
	Bitmap*					image_right_[2][2];			//! 4 image state bitmaps: [active yes/no][pushed down yes/no]
	int16_t					left_width_;				//! width of the left image
	int16_t					mid_width_;					//! width of the mid image
	int16_t					right_width_;				//! width of the right image
	int16_t					height_;					//! height of the images (all must have same height)
};

struct Theme
{
	Font*					icon_font_;
	Font*					control_font_;					//! Font for controls (incuding titlebar)
	uint8_t*				clut_;
	uint8_t					outline_size_;					//! Thickness of border around window, in pixels. 0 is acceptable. Border is drawn from window rect inwards (not outwards)
	uint8_t					outline_color_;					//! Index to the color LUT
	bool					flow_from_bottom_;				//! Controls the vertical flow of elements: if true, order from top will be content area->iconbar->titlebar
	uint8_t					titlebar_height_;				//! Height of titlebar. Cannot be smaller than height of control font.
	uint8_t					titlebar_color_;				//! Background color of the title bar when window is active - Index to the color LUT	
	bool					titlebar_outline_;				//! Draw an outline using the outline_color_, around the titlebar?
	uint8_t					title_color_;					//! Foreground color of the title bar when window is active - Index to the color LUT	
	h_align_type			title_h_align_;					//! whether the title text should be positioned relative to the left side, right side, or centered
	int16_t					title_x_offset_;				//! horizontal coordinate relative to the window's left or right edge. If title_h_align_ is H_ALIGN_CENTER, this value will be ignored.
	uint8_t					iconbar_height_;				//! Height of iconbar (when displayed).
	uint8_t					iconbar_color_;					//! Index to the color LUT
	bool					iconbar_outline_;				//! Draw an outline using the outline_color_, around the iconbar?
	uint8_t					standard_back_color_;			//! Index to the color LUT
	uint8_t					standard_fore_color_;			//! Index to the color LUT	
	uint8_t					inactive_back_color_;			//! Index to the color LUT
	uint8_t					inactive_fore_color_;			//! Index to the color LUT	
	uint8_t					highlight_back_color_;			//! Index to the color LUT
	uint8_t					highlight_fore_color_;			//! Index to the color LUT	
	uint8_t					desktop_color_;					//! Required LUT index for the desktop color; Used when no pattern bitmap
	Bitmap*					desktop_pattern_;				//! Optional bitmap to be tiled into a desktop pattern
	uint8_t					pattern_width_;					//! Width of the desktop pattern
	uint8_t					pattern_height_;				//! Height of the desktop pattern
	uint32_t				background_color_;				//! 32bit BGRA (A is non-functional). Used for the base background layer in the VICKY.
	uint32_t				border_color_;					//! 32bit BGRA (A is non-functional). Used for the border color in the VICKY.
	ControlTemplate*		control_t_close_;
	ControlTemplate*		control_t_minimize_;
	ControlTemplate*		control_t_norm_size_;
	ControlTemplate*		control_t_maximize_;
	ControlBackdrop			flex_width_backdrops_[2];		//! structs to hold pointers to the background left/mid/right graphics for varying-width controls like buttons

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
bool Theme_Destroy(Theme** the_theme);



//! create default Theme.
//! used in cases where a custom theme is not specified or is not available
Theme* Theme_CreateDefaultTheme(void);






// **** xxx functions *****

//! Make the passed theme into the active Theme
//! Note: this will change the VICKY's CLUT, inform the system to change its current theme (and in future: load new icons, etc) 
bool Theme_Activate(Theme* the_theme);


// **** Set xxx functions *****





// **** Get xxx functions *****

ControlTemplate* Theme_GetCloseControlTemplate(Theme* the_theme);
ControlTemplate* Theme_GetMinimizeControlTemplate(Theme* the_theme);
ControlTemplate* Theme_GetNormSizeControlTemplate(Theme* the_theme);
ControlTemplate* Theme_GetMaximizeControlTemplate(Theme* the_theme);

//! Create a control template for a flexible-width control
ControlTemplate* Theme_CreateControlTemplateFlexWidth(Theme* the_theme, control_type the_type, int16_t width, int16_t height, int16_t x_offset, int16_t y_offset, h_align_type h_align, v_align_type v_align, char* caption);

Bitmap* Theme_GetDesktopPattern(Theme* the_theme);

ColorIdx Theme_GetTitlebarColor(Theme* the_theme);
ColorIdx Theme_GetTitleColor(Theme* the_theme);
ColorIdx Theme_GetIconbarColor(Theme* the_theme);
ColorIdx Theme_GetOutlineColor(Theme* the_theme);
ColorIdx Theme_GetContentAreaColor(Theme* the_theme);
ColorIdx Theme_GetDesktopColor(Theme* the_theme);
ColorIdx Theme_GetInactiveBackColor(Theme* the_theme);
ColorIdx Theme_GetInactiveForeColor(Theme* the_theme);



// **** xxx functions *****




// **** xxx functions *****




// TEMP UNTIL DISK AVAIL
//! create default Theme.
//! used in cases where a custom theme is not specified or is not available
Theme* Theme_CreateGreenTheme(void);




// **** Debug functions *****

void Theme_Print(Theme* the_theme);



#endif /* LIB_THEME_H_ */


