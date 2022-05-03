//! @file lib_sys.h

/*
 * lib_sys.h
 *
*  Created on: Mar 22, 2022
 *      Author: micahbly
 */

#ifndef LIB_SYS_H_
#define LIB_SYS_H_


/* about this library: System
 *
 * This provides overall system level functionality
 *
 *** things this library needs to be able to do
 * Manage global system resources such as fonts, screens, mouse pointer, etc. 
 * Provide event handling
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
#include "list.h"
#include "theme.h"
#include "control_template.h"


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

#define SYS_MAX_WINDOWS					32
#define SYS_WIN_Z_ORDER_BACKDROP		-127
#define SYS_WIN_Z_ORDER_NEWLY_ACTIVE	SYS_MAX_WINDOWS + 1


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

struct System
{
	EventManager*	event_manager_;
	Font*			system_font_;
	Font*			app_font_;
	Screen*			screen_[2];
	Theme*			theme_;
	uint8_t			num_screens_;
	List**			list_windows_;
	Window*			active_window_;
	uint8_t			window_count_;
	uint16_t		model_number_;
};





/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate a System object
System* Sys_New(void);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
//! @param	the_system: valid pointer to system object
bool Sys_Destroy(System** the_system);



// **** System Initialization functions *****

//! Initialize the system (primary entry point for all system initialization activity)
//! Starts up the memory manager, creates the global system object, runs autoconfigure to check the system hardware, loads system and application fonts, allocates a bitmap for the screen.
bool Sys_InitSystem(void);




// **** Event-handling functions *****




// **** Screen mode/resolution/size functions *****

//! Find out what kind of machine the software is running on, and determine # of screens available
//! @param	the_system: valid pointer to system object
//! @return	Returns false if the machine is known to be incompatible with this software. 
bool Sys_AutoDetectMachine(System* the_system);

//! Find out what kind of machine the software is running on, and configure the passed screen accordingly
//! Configures screen settings, RAM addresses, etc. based on known info about machine types
//! Configures screen width, height, total text rows and cols, and visible text rows and cols by checking hardware
//! @param	the_system: valid pointer to system object
//! @return	Returns false if the machine is known to be incompatible with this software. 
bool Sys_AutoConfigure(System* the_system);

//! Detect the current screen mode/resolution, and set # of columns, rows, H pixels, V pixels, accordingly
//! @param	the_screen: valid pointer to the target screen to operate on
//! @return	returns false on any error/invalid input.
bool Sys_DetectScreenSize(Screen* the_screen);

//! Change video mode to the one passed.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	new_mode: One of the enumerated screen_resolution values. Must correspond to a valid VICKY video mode for the host machine. See VICKY_IIIA_RES_800X600_FLAGS, etc. defined in a2560_platform.h
//! @return	returns false on any error/invalid input.
bool Sys_SetVideoMode(Screen* the_screen, screen_resolution new_mode);

//! Switch machine into graphics mode
//! @param	the_system: valid pointer to system object
bool Sys_SetModeGraphics(System* the_system);

//! Switch machine into text mode
//! @param	the_system: valid pointer to system object
//! @param as_overlay: If true, sets text overlay mode (text over graphics). If false, sets full text mode (no graphics);
bool Sys_SetModeText(System* the_system, bool as_overlay);

//! Enable or disable the hardware cursor in text mode, for the specified screen
//! @param	the_system: valid pointer to system object
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param enable_it: If true, turns the hardware blinking cursor on. If false, hides the hardware cursor;
bool Sys_EnableTextModeCursor(System* the_system, Screen* the_screen, bool enable_it);



// **** Window management functions *****

//! Add this window to the list of windows and make it the currently active window
//! @param	the_system: valid pointer to system object
//! @return	Returns false if adding this window would exceed the system's hard cap on the number of available windows
bool Sys_AddToWindowList(System* the_system, Window* the_new_window);

// create the backdrop window for the system
//! @param	the_system: valid pointer to system object
bool Sys_CreateBackdropWindow(System* the_system);

// return the active window
//! @param	the_system: valid pointer to system object
Window* Sys_GetActiveWindow(System* the_system);

// return the backdrop window
//! @param	the_system: valid pointer to system object
Window* Sys_GetBackdropWindow(System* the_system);

// return a reference to the next window in the system's list, excluding backdrop windows
//! @param	the_system: valid pointer to system object
Window* Sys_GetNextWindow(System* the_system);

// return a reference to the previous window in the system's list, excluding backdrop windows
//! @param	the_system: valid pointer to system object
Window* Sys_GetPreviousWindow(System* the_system);

// Find the Window under the mouse -- accounts for z depth (topmost window will be found)
//! @param	the_system: valid pointer to system object
//! @param	x: global horizontal coordinate
//! @param	y: global vertical coordinate
Window* Sys_GetWindowAtXY(System* the_system, int16_t x, int16_t y);

//! Set the passed window to the active window, and marks the previously active window as inactive
//! NOTE: This will resort the list of windows to move the (new) active one to the front
//! NOTE: The exception to this is that the backdrop window is never moved in front of other windows
//! @param	the_system: valid pointer to system object
bool Sys_SetActiveWindow(System* the_system, Window* the_window);

// List-sort compatible function for sorting windows by their display order property
bool Window_CompareDisplayOrder(void* first_payload, void* second_payload);


// **** Other GET functions *****

//! @param	the_system: valid pointer to system object
Font* Sys_GetSystemFont(System* the_system);

//! @param	the_system: valid pointer to system object
Font* Sys_GetAppFont(System* the_system);

//! @param	the_system: valid pointer to system object
Screen* Sys_GetScreen(System* the_system, int16_t channel_id);

//! @param	the_system: valid pointer to system object
Theme* Sys_GetTheme(System* the_system);

//! NOTE: Foenix systems only have 1 screen with bitmap graphics, even if the system has 2 screens overall. The bitmap returned will always be from the appropriate channel (A or B).
//! @param	the_system: valid pointer to system object
Bitmap* Sys_GetScreenBitmap(System* the_system, bitmap_layer the_layer);

//! @param	the_system: valid pointer to system object
EventManager* Sys_GetEventManager(System* the_system);



// **** Other SET functions *****

//! @param	the_system: valid pointer to system object
void Sys_SetSystemFont(System* the_system, Font* the_font);

//! @param	the_system: valid pointer to system object
void Sys_SetAppFont(System* the_system, Font* the_font);

//! @param	the_system: valid pointer to system object
void Sys_SetScreen(System* the_system, int16_t channel_id, Screen* the_screen);

//! NOTE: Foenix systems only have 1 screen with bitmap graphics, even if the system has 2 screens overall. The bitmap returned will always be from the appropriate channel (A or B).
//! @param	the_system: valid pointer to system object
void Sys_SetScreenBitmap(System* the_system, Bitmap* the_bitmap, bitmap_layer the_layer);

//! Set the passed theme as the System's current theme
//! Note: this will dispose of the current theme after setting the new one
//! @param	the_system: valid pointer to system object
//! @return	Returns false on any error condition
bool Sys_SetTheme(System* the_system, Theme* the_theme);



// **** xxx functions *****

//! Tell the VICKY to use a different address for the specified bitmap layer
//! @param	the_system: valid pointer to system object
//! @param	the_bitmap_layer: 0 or 1, the bitmap layer to get a new address
//! @param	the_address: The address within the VRAM zone that the bitmap layer should be repointed to
bool Sys_SetVRAMAddr(System* the_system, uint8_t the_bitmap_layer, unsigned char* the_address);



// **** xxx functions *****







//
// Discord, 3022/03/10
//
// PJW
// Generally speaking (the order does not really matter on this):
// 1. Set the mode bits in the master control register (GRAPHICS + BITMAP), as well as the resolution.
// 2. Load the color table for the bitmap into one of the graphics LUTs (start at $B4:2000 on the A2560U, $FEC8:2000 on the A2560K)
// 3. Load the bitmap pixel data into video RAM (starts at $C0:0000 on the A2560U, $0080:0000 on the A2560K)
// 4. Set the enable bit and LUT # in the bitmap control register for the layer you want
// 5. Set the address of the pixel data in video RAM in the VRAM address pointer for the layer you want. This must be set relative to the beginning of VRAM, so if you're using the A2560U and the pixel data starts at $C2:0000, you'd store $02:0000 in the address register.
// Paul Scott Robson — Today at 12:45 PM
// TextInitialise:
//         clr.l     d0
//         lea     vicky3,a0                     ; start register writes
//         move.l     #$0000000C,(a0)             ; graphics mode & bitmap on.
//         move.l     d0,$4(a0)                     ; zero border size/scroll/colour
//         move.l     d0,$8(a0)                     ; clear border colour
//         move.l     d0,$C(a0)
//         move.l     d0,$10(a0)                     ; cursor off
//         move.l     d0,$14(a0)                     ; no line interrupts
//         move.l     d0,$18(a0)
// 
//         move.l     #$00000001,$100(a0)         ; LUT 0, BMP 1 on
//         move.l     d0,$104(a0)                 ; BMP 1 address
//         move.l     d0,$108(a0)                    ; BMP 2 off
// 
//         move.l     #$0000FF00,$2004(a0)         ; colour 1 red
//         move.l     #$00FF00FF,$2008(a0)         ; colour 2 green
//         move.l     #$00FFFFFF,$200C(a0)         ; colour 3 yellow
//         move.l     #$FF0000FF,$2010(a0)         ; colour 4 blue
//         move.l     #$FF00FFFF,$2014(a0)         ; colour 5 magenta
//         move.l     #$FFFF00FF,$2018(a0)         ; colour 6 cyan
//         move.l     #$FFFFFFFF,$201C(a0)         ; colour 7 white
// 
//         rts
// Bit Q&D but I think it's right


// **** TEMP font loading *****

// create a font object and populate with modified chicago 14 pt regular
// this is a temporary function until we get file handling and can load from disk
Font* Sys_LoadSystemFont(void);

// create a font object and populate with helvetica 9 pt regular
// this is a temporary function until we get file handling and can load from disk
Font* Sys_LoadAppFont(void);



// **** Render functions *****


//! Render all visible windows
//! NOTE: this will move to a private Sys function later, once event handling is available
//! @param	the_system: valid pointer to system object
void Sys_Render(System* the_system);




#endif /* LIB_SYS_H_ */


