/*
 * lib_sys.c
 *
 *  Created on: Mar 22, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "lib_sys.h"

#include "a2560_platform.h"
#include "bitmap.h"
#include "event.h"
#include "font.h"
#include "general.h"
#include "list.h"
#include "menu.h"
#include "text.h"
#include "theme.h"
#include "window.h"
#include "startup.h"
//#include "mcp_code/dev/ps2.h"

// C includes
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A2560 includes
#include <mcp/syscalls.h>
#include <mcp/interrupt.h>


/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

System*			global_system;

// MCP / previous interrupt handler functions for restore on exit
// p_int_handler	global_old_keyboard_interrupt;
// p_int_handler	global_old_mouse_interrupt;
// p_int_handler is defined in mcp/interrupt.h as typedef void (*p_int_handler)();

// VGA colors, used for both fore- and background colors in Text mode
// in C256, these are 8 bit values; in A2560s, they are 32 bit values, and endianness matters
#ifdef _C256_FMX_
	static uint8_t standard_text_color_lut[64] = 
	{
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xAA, 0x00,
		0x00, 0xAA, 0x00, 0x00,
		0x00, 0x55, 0xAA, 0x00,
		0xAA, 0x00, 0x00, 0x00,
		0xAA, 0x00, 0xAA, 0x00,
		0xAA, 0xAA, 0x00, 0x00,
		0xAA, 0xAA, 0xAA, 0x00,
		0x55, 0x55, 0x55, 0x00,
		0x55, 0x55, 0xFF, 0x00,
		0x55, 0xFF, 0x55, 0x00,
		0x55, 0xFF, 0xFF, 0x00,
		0xFF, 0x55, 0x55, 0x00,
		0xFF, 0x55, 0xFF, 0x00,
		0xFF, 0xFF, 0x55, 0x00,
		0xFF, 0xFF, 0xFF, 0x00,			
	};
#else
	static uint8_t standard_text_color_lut[64] = 
	{
		0x00, 0x00, 0x00, 0x00,
		0x00, 0xAA, 0x00, 0x00,
		0x00, 0x00, 0xAA, 0x00,
		0x00, 0xAA, 0x55, 0x00,
		0x00, 0x00, 0x00, 0xAA,
		0x00, 0xAA, 0x00, 0xAA,
		0x00, 0x00, 0xAA, 0xAA,
		0x00, 0xAA, 0xAA, 0xAA,
		0x00, 0x55, 0x55, 0x55,
		0x00, 0xFF, 0x55, 0x55,
		0x00, 0x55, 0xFF, 0x55,
		0x00, 0xFF, 0xFF, 0x55,
		0x00, 0x55, 0x55, 0xFF,
		0x00, 0xFF, 0x55, 0xFF,
		0x00, 0x55, 0xFF, 0xFF,
		0x00, 0xFF, 0xFF, 0xFF,			
	};
#endif


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/


//! Initialize the system (primary entry point for all system initialization activity) for use with C256 systems
//! Primary difference is that no backdrop window is created (Windows with bitmaps not supported); 
//! Starts up the memory manager, creates the global system object, runs autoconfigure to check the system hardware, loads system and application fonts, allocates a bitmap for the screen.
bool Sys_InitSystemC256(void);

// Instruct all windows to close / clean themselves up
void Sys_DestroyAllWindows(System* the_system);

//! Instruct every window to update itself and its controls to match the system's current theme
//! This is called as part of Sys_SetTheme().
void Sys_UpdateWindowTheme(System* the_system);

void Sys_RenumberWindows(System* the_system);

// enable or disable the gamma correction 
bool Sys_SetGammaMode(System* the_system, Screen* the_screen, bool enable_it);

//! Event handler for the backdrop window
void Window_BackdropWinEventHandler(EventRecord* the_event);




/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/




//! Initialize the system (primary entry point for all system initialization activity) for use with C256 systems
//! Primary difference is that no backdrop window is created (Windows with bitmaps not supported) and no theme is created (saving memory)
//! Starts up the memory manager, creates the global system object, runs autoconfigure to check the system hardware, loads system and application fonts, allocates a bitmap for the screen.
bool Sys_InitSystemC256(void)
{
	Font*		the_system_font;
	Font*		the_icon_font;
	Theme*		the_theme;
	int16_t		i;
	
	
	DEBUG_OUT(("%s %d: Initializing System...", __func__, __LINE__));
	
	// initialize the system object
	if ((global_system = Sys_New()) == NULL)
	{
		LOG_ERR(("%s %d: Couldn't instantiate system object", __func__, __LINE__));
		goto error;
	}

	DEBUG_OUT(("%s %d: System object created ok. Initiating list of windows...", __func__, __LINE__));
	
// 	// set the global variable that other classes/libraries need access to.
// 	global_system = the_system;

	DEBUG_OUT(("%s %d: Running Autoconfigure...", __func__, __LINE__));
	
	if (Sys_AutoConfigure(global_system) == false)
	{
		LOG_ERR(("%s %d: Auto configure failed", __func__, __LINE__));
		goto error;
	}

	// LOGIC:
	//   load default theme so that fonts are available
	//   having system fonts in lib sys so they are guaranteed is good, but once a theme is loaded it replaces theme
	//   so default theme needs to know how to reload them in case user switches back
	//   better to have it consistent: theme is responsible for loading and setting system fonts
	
	DEBUG_OUT(("%s %d: loading default theme...", __func__, __LINE__));
	
	if ( (the_theme = Theme_CreateDefaultTheme(true) ) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create default system theme", __func__, __LINE__));
		goto error;
	}
	
	Theme_Activate(the_theme);
// 	
// 	DEBUG_OUT(("%s %d: Default theme loaded ok. Creating menu manager...", __func__ , __LINE__));
// 	
// 	// menu manager
// 	if ( (global_system->menu_manager_ = Menu_New() ) == NULL)
// 	{
// 		LOG_ERR(("%s %d: could not allocate memory to create the menu manager", __func__ , __LINE__));
// 		goto error;
// 	}	
// 
// 	DEBUG_OUT(("%s %d: allocating screen bitmap...", __func__, __LINE__));
	
	// allocate the foreground and background bitmaps, then assign them fixed locations in VRAM
	
	// LOGIC: 
	//   The only bitmaps we want pointing to VRAM locations are the system's layer0 and layer1 bitmaps for the screen
	//   Only 1 screen has bitmapped graphics
	//   We assign them fixed spaces in VRAM, 800*600 apart, so that the addresses are good even on a screen resolution change. 
	
	for (i = 0; i < 2; i++)
	{
		Bitmap*		the_bitmap;

		if ( (the_bitmap = Bitmap_New(global_system->screen_[ID_CHANNEL_B]->width_, global_system->screen_[ID_CHANNEL_B]->height_, Sys_GetSystemFont(global_system), PARAM_IN_VRAM)) == NULL)
		{
			LOG_ERR(("%s %d: Failed to create bitmap #%i", __func__, __LINE__, i));
			goto error;
		}
	
		the_bitmap->addr_int_ = (uint32_t)VRAM_START + (uint32_t)i * (uint32_t)VRAM_OFFSET_TO_NEXT_SCREEN;
		the_bitmap->addr_ = (unsigned char*)the_bitmap->addr_int_;
		
		//DEBUG_OUT(("%s %d: bitmap addr_int_=%lx, addr_=%p", __func__, __LINE__, the_bitmap->addr_int_, the_bitmap->addr_));
		
		Sys_SetScreenBitmap(global_system, the_bitmap, i);
		
		// clear the bitmap
		Bitmap_FillMemory(the_bitmap, 0x0F);
	}
	

// 	// load the splash screen and progress bar
// 	if (Startup_ShowSplash() == false)
// 	{
// 		LOG_ERR(("%s %d: Failed to load splash screen. Oh, no!", __func__, __LINE__));
// 		// but who cares, just continue on... 
// 	}

	// create the backdrop window and add it to the list of the windows the system tracks
	
	// LOGIC:
	//   Every app will use (or at least have access to) the backdrop window
	//   The backdrop window shares the same bitmap as the Screen
	//   The backdrop window will catch events that drop through the windows in the foreground
	
// 	if ( Sys_CreateBackdropWindow(global_system) == false)
// 	{
// 		LOG_ERR(("%s %d: Failed to create a backdrop window. Fatal error.", __func__, __LINE__));
// 		goto error;
// 	}
	
	// Enable mouse pointer -- no idea if this works, f68 emulator doesn't support mouse yet. 
	//R32(VICKYB_MOUSE_CTRL_A2560K) = 1;
	
	// set interrupt handlers
//	ps2_init();
//	global_old_keyboard_interrupt = sys_int_register(INT_KBD_PS2, &Sys_InterruptKeyboard);
// 	global_old_keyboard_interrupt = sys_int_register(INT_KBD_A2560K, &Sys_InterruptKeyboard);
// 	global_old_mouse_interrupt = sys_int_register(INT_MOUSE, &Sys_InterruptKeyboard);

	DEBUG_OUT(("%s %d: System initialization complete.", __func__, __LINE__));

	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Instruct every window to update itself and its controls to match the system's current theme
//! This is called as part of Sys_SetTheme().
void Sys_UpdateWindowTheme(System* the_system)
{
 	List*	the_item;
 	
 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		Window_UpdateTheme(this_window);
		
		the_item = the_item->next_item_;
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


void Sys_RenumberWindows(System* the_system)
{
 	List*	the_item;
 	int8_t	win_num = 1;
 	int8_t	this_display_order;
 	
 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return;
 	}
	
// 	DEBUG_OUT(("%s %d: win count=%i", __func__ , __LINE__, the_system->window_count_));
// 	List_Print(the_system->list_windows_, (void*)&Window_PrintBrief);
	
	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		if (this_window == NULL)
		{
			LOG_ERR(("%s %d: this_window was null, the_item=%p, win_num=%i, win count=%i", __func__ , __LINE__, the_item, win_num, the_system->window_count_));
			goto error;
		}
		
		if (this_window->is_backdrop_)
		{
			this_display_order = SYS_WIN_Z_ORDER_BACKDROP;
		}
		else
		{
			this_display_order = SYS_MAX_WINDOWS - win_num++;
		}
		
		Window_SetDisplayOrder(this_window, this_display_order);
		
		the_item = the_item->next_item_;
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Event handler for the backdrop window
void Window_BackdropWinEventHandler(EventRecord* the_event)
{
	return;
}


// // interrupt 1 is PS2 keyboard, interrupt 2 is A2560K keyboard
// void Sys_InterruptKeyboard(void)
// {
// 	kbd_handle_irq();
// }

// 
// // interrupt 4 is PS2 mouse
// void Sys_InterruptMouse(void);


// **** Debug functions *****

void Sys_Print(System* the_system)
{
	DEBUG_OUT(("System print out:"));
	DEBUG_OUT(("  address: %p", 			the_system));
	DEBUG_OUT(("  event_manager_: %p", 		the_system->event_manager_));
	DEBUG_OUT(("  menu_manager_: %p", 		the_system->menu_manager_));
	DEBUG_OUT(("  system_font_: %p", 		the_system->system_font_));
	DEBUG_OUT(("  app_font_: %p",			the_system->app_font_));
	DEBUG_OUT(("  theme_: %p",				the_system->theme_));
	DEBUG_OUT(("  num_screens_: %i",		the_system->num_screens_));
	DEBUG_OUT(("  window_count_: %i",		the_system->window_count_));
	DEBUG_OUT(("  active_window_: %p",		the_system->active_window_));
	DEBUG_OUT(("  model_number_: %i",		the_system->model_number_));
}


void Sys_PrintScreen(Screen* the_screen)
{
	DEBUG_OUT(("Screen print out:"));
	DEBUG_OUT(("  address: %p", 			the_screen));
	DEBUG_OUT(("  id_: %i", 				the_screen->id_));
	DEBUG_OUT(("  vicky_: %p", 				the_screen->vicky_));
	DEBUG_OUT(("  width_: %i", 				the_screen->width_));
	DEBUG_OUT(("  height_: %i", 			the_screen->height_));
	DEBUG_OUT(("  text_cols_vis_: %i", 		the_screen->text_cols_vis_));
	DEBUG_OUT(("  text_rows_vis_: %i", 		the_screen->text_rows_vis_));
	DEBUG_OUT(("  text_mem_cols_: %i", 		the_screen->text_mem_cols_));
	DEBUG_OUT(("  text_mem_rows_: %i", 		the_screen->text_mem_rows_));
	DEBUG_OUT(("  text_ram_: %p", 			the_screen->text_ram_));
	DEBUG_OUT(("  text_attr_ram_: %p", 		the_screen->text_attr_ram_));
	DEBUG_OUT(("  text_font_ram_: %p", 		the_screen->text_font_ram_));
	DEBUG_OUT(("  bitmap_[0]: %p", 			the_screen->bitmap_[0]));
	DEBUG_OUT(("  bitmap_[1]: %p", 			the_screen->bitmap_[1]));
	DEBUG_OUT(("  text_font_height_: %i",	the_screen->text_font_height_));
	DEBUG_OUT(("  text_font_width_: %i",	the_screen->text_font_width_));
}



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
System* Sys_New(void)
{
	System*			the_system;
	int16_t			i;
	
	
	// LOGIC:
	
	if ( (the_system = (System*)calloc(1, sizeof(System)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new system", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_system	%p	size	%i", __func__ , __LINE__, the_system, sizeof(System)));
	
	DEBUG_OUT(("%s %d: System object created ok...", __func__ , __LINE__));

	// event manager -- not currently 65816 compatible
	#ifndef _C256_FMX_
		if ( (the_system->event_manager_ = EventManager_New() ) == NULL)
		{
			LOG_ERR(("%s %d: could not allocate memory to create the event manager", __func__ , __LINE__));
			goto error;
		}
	
		DEBUG_OUT(("%s %d: EventManager created ok. Detecting hardware...", __func__ , __LINE__));
	#endif
	
	// check what kind of hardware the system is running on
	// LOGIC: we need to know how many screens it has before allocating screen objects
	if (Sys_AutoDetectMachine(the_system) == false)
	{
		LOG_ERR(("%s %d: Detected machine hardware is incompatible with this software", __func__ , __LINE__));
		goto error;
	}
	
	DEBUG_OUT(("%s %d: Hardware detected (%u screens). Creating screens...", __func__ , __LINE__, the_system->num_screens_));

	// screens
	for (i = 0; i < the_system->num_screens_; i++)
	{
		if ( (the_system->screen_[i] = (Screen*)calloc(1, sizeof(Screen)) ) == NULL)
		{
			LOG_ERR(("%s %d: could not allocate memory to create screen object", __func__ , __LINE__));
			goto error;
		}
		LOG_ALLOC(("%s %d:	__ALLOC__	the_system->screen_[%i]	%p	size	%i", __func__ , __LINE__, i, the_system->screen_[i], sizeof(Screen)));
		
		the_system->screen_[i]->id_ = i;
	}

	DEBUG_OUT(("%s %d: Screen(s) created ok.", __func__ , __LINE__, i));
	
	// for systems with only one screen, we will point the 2nd screen to the first, so that any call to 2nd screen works as if it was a call to the first. 
	if (the_system->num_screens_ == 1)
	{
		the_system->screen_[1] = the_system->screen_[0];
	}

	DEBUG_OUT(("%s %d: returning to SysInit()...", __func__ , __LINE__, i));
	
	// LOGIC: we don't have font info yet; just want to make it clear these are not set and not rely on compiler behavior
	the_system->system_font_ = NULL;
	the_system->app_font_ = NULL;
	the_system->theme_ = NULL;
	the_system->active_window_ = NULL;
	the_system->window_count_ = 0;
	
	return the_system;
	
error:
	if (the_system)					Sys_Destroy(&the_system);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
//! @param	the_system: valid pointer to system object
bool Sys_Destroy(System** the_system)
{
	int16_t	i;
	
	if (*the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	for (i = 0; i < 2; i++)
	{
		if ((*the_system)->screen_[i])
		{
			LOG_ALLOC(("%s %d:	__FREE__	(*the_system)->screen_[i]	%p	size	%i", __func__ , __LINE__, (*the_system)->screen_[i], sizeof(Screen)));
			free((*the_system)->screen_[i]);
			(*the_system)->screen_[i] = NULL;
		}
	}

	if ((*the_system)->system_font_)
	{
		Font_Destroy(&(*the_system)->system_font_);
	}

	if ((*the_system)->app_font_)
	{
		Font_Destroy(&(*the_system)->app_font_);
	}

	if ((*the_system)->menu_manager_)
	{
		Menu_Destroy(&(*the_system)->menu_manager_);
	}

	if ((*the_system)->event_manager_)
	{
		EventManager_Destroy(&(*the_system)->event_manager_);
	}

	if ((*the_system)->list_windows_)
	{
		Sys_DestroyAllWindows(*the_system);
	}


	LOG_ALLOC(("%s %d:	__FREE__	*the_system	%p	size	%i", __func__ , __LINE__, *the_system, sizeof(System)));
	free(*the_system);
	*the_system = NULL;

	DEBUG_OUT(("%s %d: **** SYSTEM EXIT ON ERROR! ****", __func__, __LINE__));
	
	exit(-1);
	
	return true;
}


//! Instruct all windows to close / clean themselves up
//! @param	the_system: valid pointer to system object
void Sys_DestroyAllWindows(System* the_system)
{
	int16_t		num_nodes = 0;
	List*		the_item;
	
 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	if (the_system->list_windows_ == NULL)
	{
		DEBUG_OUT(("%s %d: the window list was NULL", __func__ , __LINE__));
		goto error;
	}
	
	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		Window_Destroy(&this_window);
		++num_nodes;
		--the_system->window_count_;

		the_item = the_item->next_item_;
	}

	// now free up the list items themselves
	List_Destroy(the_system->list_windows_);

	DEBUG_OUT(("%s %d: %i windows closed", __func__ , __LINE__, num_nodes));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}



// **** System Initialization functions *****

//! Initialize the system (primary entry point for all system initialization activity)
//! Starts up the memory manager, creates the global system object, runs autoconfigure to check the system hardware, loads system and application fonts, allocates a bitmap for the screen.
bool Sys_InitSystem(void)
{
	Font*		the_system_font;
	Font*		the_icon_font;
	Theme*		the_theme;
	int16_t		i;
	
	// C256 needs more limited startup...
	#ifdef _C256_FMX_
		return Sys_InitSystemC256();
	#endif
	
	DEBUG_OUT(("%s %d: Initializing System...", __func__, __LINE__));
	
	// initialize the system object
	if ((global_system = Sys_New()) == NULL)
	{
		LOG_ERR(("%s %d: Couldn't instantiate system object", __func__, __LINE__));
		goto error;
	}

	DEBUG_OUT(("%s %d: System object created ok. Initiating list of windows...", __func__, __LINE__));
	
// 	// set the global variable that other classes/libraries need access to.
// 	global_system = the_system;

	DEBUG_OUT(("%s %d: Running Autoconfigure...", __func__, __LINE__));
	
	if (Sys_AutoConfigure(global_system) == false)
	{
		LOG_ERR(("%s %d: Auto configure failed", __func__, __LINE__));
		goto error;
	}

	// LOGIC:
	//   load default theme so that fonts are available
	//   having system fonts in lib sys so they are guaranteed is good, but once a theme is loaded it replaces theme
	//   so default theme needs to know how to reload them in case user switches back
	//   better to have it consistent: theme is responsible for loading and setting system fonts
	
	DEBUG_OUT(("%s %d: loading default theme...", __func__, __LINE__));
	
	if ( (the_theme = Theme_CreateDefaultTheme(THEME_PARAM_FULL_RESOURCES) ) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create default system theme", __func__, __LINE__));
		goto error;
	}
	
	Theme_Activate(the_theme);
	
	DEBUG_OUT(("%s %d: Default theme loaded ok. Creating menu manager...", __func__ , __LINE__));
	
	// menu manager
	if ( (global_system->menu_manager_ = Menu_New() ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create the menu manager", __func__ , __LINE__));
		goto error;
	}	

	DEBUG_OUT(("%s %d: allocating screen bitmap...", __func__, __LINE__));
	
	// allocate the foreground and background bitmaps, then assign them fixed locations in VRAM
	
	// LOGIC: 
	//   The only bitmaps we want pointing to VRAM locations are the system's layer0 and layer1 bitmaps for the screen
	//   Only 1 screen has bitmapped graphics
	//   We assign them fixed spaces in VRAM, 800*600 apart, so that the addresses are good even on a screen resolution change. 
	
	for (i = 0; i < 2; i++)
	{
		Bitmap*		the_bitmap;

		if ( (the_bitmap = Bitmap_New(global_system->screen_[ID_CHANNEL_B]->width_, global_system->screen_[ID_CHANNEL_B]->height_, Sys_GetSystemFont(global_system), PARAM_IN_VRAM)) == NULL)
		{
			LOG_ERR(("%s %d: Failed to create bitmap #%i", __func__, __LINE__, i));
			goto error;
		}

		the_bitmap->addr_int_ = (uint32_t)VRAM_START + (uint32_t)i * (uint32_t)VRAM_OFFSET_TO_NEXT_SCREEN;
		the_bitmap->addr_ = (unsigned char*)the_bitmap->addr_int_;

		//DEBUG_OUT(("%s %d: bitmap addr_int_=%lx, addr_=%p", __func__, __LINE__, the_bitmap->addr_int_, the_bitmap->addr_));
		
		Sys_SetScreenBitmap(global_system, the_bitmap, i);
		
		// clear the bitmap
		Bitmap_FillMemory(the_bitmap, 0x00);
	}
	

// 	// load the splash screen and progress bar
// 	if (Startup_ShowSplash() == false)
// 	{
// 		LOG_ERR(("%s %d: Failed to load splash screen. Oh, no!", __func__, __LINE__));
// 		// but who cares, just continue on... 
// 	}

	// create the backdrop window and add it to the list of the windows the system tracks
	
	// LOGIC:
	//   Every app will use (or at least have access to) the backdrop window
	//   The backdrop window shares the same bitmap as the Screen
	//   The backdrop window will catch events that drop through the windows in the foreground
	
	if ( Sys_CreateBackdropWindow(global_system) == false)
	{
		LOG_ERR(("%s %d: Failed to create a backdrop window. Fatal error.", __func__, __LINE__));
		goto error;
	}
	
	// Enable mouse pointer -- no idea if this works, f68 emulator doesn't support mouse yet. 
	//R32(VICKYB_MOUSE_CTRL_A2560K) = 1;
	
	// set interrupt handlers
//	ps2_init();
//	global_old_keyboard_interrupt = sys_int_register(INT_KBD_PS2, &Sys_InterruptKeyboard);
// 	global_old_keyboard_interrupt = sys_int_register(INT_KBD_A2560K, &Sys_InterruptKeyboard);
// 	global_old_mouse_interrupt = sys_int_register(INT_MOUSE, &Sys_InterruptKeyboard);

	DEBUG_OUT(("%s %d: System initialization complete.", __func__, __LINE__));

	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}





// **** Event-handling functions *****

// see MCP's ps2.c for real examples once real machine available

// // interrupt 1 is PS2 keyboard, interrupt 2 is A2560K keyboard
// void Sys_InterruptKeyboard(void)
// {
// 	printf("keyboard!\n");
// 	return;
// }
// 
// // interrupt 4 is PS2 mouse
// void Sys_InterruptMouse(void)
// {
// 	printf("mouse!\n");
// 	return;
// }




// **** Screen mode/resolution/size functions *****


//! Find out what kind of machine the software is running on, and determine # of screens available
//! @param	the_system: valid pointer to system object
//! @return	Returns false if the machine is known to be incompatible with this software. 
bool Sys_AutoDetectMachine(System* the_system)
{
	struct s_sys_info	sys_info;
	struct s_sys_info*	the_sys_info = &sys_info; // doing this convoluted thing so that C256 macro can fake having sys_get_info

	#ifdef _C256_FMX_
		uint8_t	the_machine_id;
		
		the_machine_id = (R8(GABE_SYS_STAT) & 0xF0) >> 4;
		DEBUG_OUT(("%s %d: the_machine_id=%u, gabe raw value=%u", __func__, __LINE__, the_machine_id, R8(GABE_SYS_STAT)));
		
		the_sys_info->model = the_machine_id;
		
		if (the_machine_id == MACHINE_C256_FMX)
		{
			DEBUG_OUT(("%s %d: it's a C256 FMX!", __func__, __LINE__));
		}
		else if (the_machine_id == MACHINE_C256_UPLUS)
		{
			DEBUG_OUT(("%s %d: it's a C256U+... you must be Hakan!", __func__, __LINE__));
		}
		
	#else
		sys_get_info(the_sys_info);
	#endif
	
	the_system->model_number_ = the_sys_info->model;
	DEBUG_OUT(("%s %d: the_system->model_number_=%u", __func__, __LINE__, the_system->model_number_));
	
	// temp until Calypsi fix for switch on 65816
	if (the_system->model_number_ == MACHINE_C256_U)
	{
			DEBUG_OUT(("%s %d: I think this is a C256U...", __func__, __LINE__));
			the_system->num_screens_ = 1;
	}
	else if (the_system->model_number_ == MACHINE_C256_GENX)
	{
			DEBUG_OUT(("%s %d: I think this is a C256 GENX...", __func__, __LINE__));
			the_system->num_screens_ = 1;
	}
	else if (the_system->model_number_ == MACHINE_C256_UPLUS)
	{
			DEBUG_OUT(("%s %d: I think this is a C256U+...", __func__, __LINE__));
			the_system->num_screens_ = 1;
	}
	else if (the_system->model_number_ == MACHINE_C256_FMX)
	{
			DEBUG_OUT(("%s %d: I think this is a C256 FMX...", __func__, __LINE__));
			the_system->num_screens_ = 1;
	}
	else if (the_system->model_number_ == MACHINE_A2560U || the_system->model_number_ == MACHINE_A2560U_PLUS)
	{
			DEBUG_OUT(("%s %d: I think this is a A2560U or A2560U+...", __func__, __LINE__));
			the_system->num_screens_ = 1;
	}
	else if (the_system->model_number_ == MACHINE_A2560X || the_system->model_number_ == MACHINE_A2560K)
	{
			DEBUG_OUT(("%s %d: I think this is a A2560K or A2560X...", __func__, __LINE__));
			the_system->num_screens_ = 2;
	}
	else
	{
			DEBUG_OUT(("%s %d: I can't recognize this machine (id=%u). Application will now quit.", __func__, __LINE__, the_system->model_number_));
			return false;
	}
	
// 	switch (the_system->model_number_)
// 	{
// 		case MACHINE_C256_U:
// 			DEBUG_OUT(("%s %d: I think this is a C256U...", __func__, __LINE__));
// 			the_system->num_screens_ = 1;
// 			break;
// 			
// 		case MACHINE_C256_GENX:
// 			DEBUG_OUT(("%s %d: I think this is a C256 GENX...", __func__, __LINE__));
// 			the_system->num_screens_ = 1;
// 			break;
// 			
// 		case MACHINE_C256_UPLUS:
// 			DEBUG_OUT(("%s %d: I think this is a C256U+...", __func__, __LINE__));
// 			the_system->num_screens_ = 1;
// 			break;
// 			
// 		case MACHINE_C256_FMX:
// 			DEBUG_OUT(("%s %d: I think this is a C256 FMX...", __func__, __LINE__));
// 			the_system->num_screens_ = 1;
// 			break;
// 			
// 		case MACHINE_A2560U_PLUS:
// 		case MACHINE_A2560U:
// 			DEBUG_OUT(("%s %d: I think this is a A2560U or A2560U+...", __func__, __LINE__));
// 			the_system->num_screens_ = 1;
// 			break;
// 			
// 		case MACHINE_A2560X:
// 		case MACHINE_A2560K:
// 			DEBUG_OUT(("%s %d: I think this is a A2560K or A2560X...", __func__, __LINE__));
// 			the_system->num_screens_ = 2;		
// 			break;
// 			
// 		default:
// 			DEBUG_OUT(("%s %d: I can't recognize this machine (id=%u). Application will now quit.", __func__, __LINE__, the_system->model_number_));
// 			return false;
// 			break;
// 	}
	
	return true;
}


//! Find out what kind of machine the software is running on, and configure the passed screen accordingly
//! Configures screen settings, RAM addresses, etc. based on known info about machine types
//! Configures screen width, height, total text rows and cols, and visible text rows and cols by checking hardware
//! @param	the_system: valid pointer to system object
//! @return	Returns false if the machine is known to be incompatible with this software. 
bool Sys_AutoConfigure(System* the_system)
{
	int16_t				i;

	// TEMP until bug fix for calypsi on switch below
	if (the_system->model_number_ == MACHINE_C256_U || the_system->model_number_ == MACHINE_C256_GENX || the_system->model_number_ == MACHINE_C256_UPLUS || the_system->model_number_ == MACHINE_C256_FMX)
	{
		DEBUG_OUT(("%s %d: Configuring screens for a C256 (1 screen)", __func__, __LINE__));
		the_system->screen_[ID_CHANNEL_A]->vicky_ = P32(VICKY_C256);
		the_system->screen_[ID_CHANNEL_A]->text_ram_ = TEXT_RAM_C256;
		the_system->screen_[ID_CHANNEL_A]->text_attr_ram_ = TEXT_ATTR_C256;
		the_system->screen_[ID_CHANNEL_A]->text_font_ram_ = FONT_MEMORY_BANK_C256;
		the_system->screen_[ID_CHANNEL_A]->text_color_fore_ram_ = (char*)TEXT_FORE_LUT_C256;
		the_system->screen_[ID_CHANNEL_A]->text_color_back_ram_ = (char*)TEXT_BACK_LUT_C256;

		the_system->screen_[ID_CHANNEL_B]->vicky_ = P32(VICKY_C256);
		the_system->screen_[ID_CHANNEL_B]->text_ram_ = TEXT_RAM_C256;
		the_system->screen_[ID_CHANNEL_B]->text_attr_ram_ = TEXT_ATTR_C256;
		the_system->screen_[ID_CHANNEL_B]->text_font_ram_ = FONT_MEMORY_BANK_C256;
		the_system->screen_[ID_CHANNEL_B]->text_color_fore_ram_ = (char*)TEXT_FORE_LUT_C256;
		the_system->screen_[ID_CHANNEL_B]->text_color_back_ram_ = (char*)TEXT_BACK_LUT_C256;
	}
	else if (the_system->model_number_ == MACHINE_A2560U_PLUS || the_system->model_number_ == MACHINE_A2560U)
	{
		the_system->screen_[ID_CHANNEL_A]->vicky_ = P32(VICKY_A2560U);
		the_system->screen_[ID_CHANNEL_A]->text_ram_ = TEXT_RAM_A2560U;
		the_system->screen_[ID_CHANNEL_A]->text_attr_ram_ = TEXT_ATTR_A2560U;
		the_system->screen_[ID_CHANNEL_A]->text_font_ram_ = FONT_MEMORY_BANK_A2560U;
		the_system->screen_[ID_CHANNEL_A]->text_color_fore_ram_ = (char*)TEXT_FORE_LUT_A2560U;
		the_system->screen_[ID_CHANNEL_A]->text_color_back_ram_ = (char*)TEXT_BACK_LUT_A2560U;

		the_system->screen_[ID_CHANNEL_B]->vicky_ = P32(VICKY_A2560U);
		the_system->screen_[ID_CHANNEL_B]->text_ram_ = TEXT_RAM_A2560U;
		the_system->screen_[ID_CHANNEL_B]->text_attr_ram_ = TEXT_ATTR_A2560U;
		the_system->screen_[ID_CHANNEL_B]->text_font_ram_ = FONT_MEMORY_BANK_A2560U;
		the_system->screen_[ID_CHANNEL_B]->text_color_fore_ram_ = (char*)TEXT_FORE_LUT_A2560U;
		the_system->screen_[ID_CHANNEL_B]->text_color_back_ram_ = (char*)TEXT_BACK_LUT_A2560U;
	}
	else if (the_system->model_number_ == MACHINE_A2560X || the_system->model_number_ == MACHINE_A2560K)
	{
		the_system->screen_[ID_CHANNEL_A]->vicky_ = P32(VICKY_A2560K_A);
		the_system->screen_[ID_CHANNEL_A]->text_ram_ = TEXTA_RAM_A2560K;
		the_system->screen_[ID_CHANNEL_A]->text_attr_ram_ = TEXTA_ATTR_A2560K;
		the_system->screen_[ID_CHANNEL_A]->text_font_ram_ = FONT_MEMORY_BANKA_A2560K;
		the_system->screen_[ID_CHANNEL_A]->text_color_fore_ram_ = (char*)TEXTA_FORE_LUT_A2560K;
		the_system->screen_[ID_CHANNEL_A]->text_color_back_ram_ = (char*)TEXTA_BACK_LUT_A2560K;

		the_system->screen_[ID_CHANNEL_B]->vicky_ = P32(VICKY_A2560K_B);
		the_system->screen_[ID_CHANNEL_B]->text_ram_ = TEXTB_RAM_A2560K;
		the_system->screen_[ID_CHANNEL_B]->text_attr_ram_ = TEXTB_ATTR_A2560K;
		the_system->screen_[ID_CHANNEL_B]->text_font_ram_ = FONT_MEMORY_BANKB_A2560K;
		the_system->screen_[ID_CHANNEL_B]->text_color_fore_ram_ = (char*)TEXTB_FORE_LUT_A2560K;
		the_system->screen_[ID_CHANNEL_B]->text_color_back_ram_ = (char*)TEXTB_BACK_LUT_A2560K;
	}
	else
	{
		DEBUG_OUT(("%s %d: this system %i not supported!", __func__, __LINE__, the_system->model_number_));
	}
	
// 	switch (the_system->model_number_)
// 	{
// 		case MACHINE_C256_U:
// 		case MACHINE_C256_GENX:
// 		case MACHINE_C256_UPLUS:
// 		case MACHINE_C256_FMX:
// 			DEBUG_OUT(("%s %d: Configuring screens for a C256 (1 screen)", __func__, __LINE__));
// 			the_system->screen_[ID_CHANNEL_A]->vicky_ = P32(VICKY_C256);
// 			the_system->screen_[ID_CHANNEL_A]->text_ram_ = TEXT_RAM_C256;
// 			the_system->screen_[ID_CHANNEL_A]->text_attr_ram_ = TEXT_ATTR_C256;
// 			the_system->screen_[ID_CHANNEL_A]->text_font_ram_ = FONT_MEMORY_BANK_C256;
// 			the_system->screen_[ID_CHANNEL_B]->vicky_ = P32(VICKY_C256);
// 			the_system->screen_[ID_CHANNEL_B]->text_ram_ = TEXT_RAM_C256;
// 			the_system->screen_[ID_CHANNEL_B]->text_attr_ram_ = TEXT_ATTR_C256;
// 			the_system->screen_[ID_CHANNEL_B]->text_font_ram_ = FONT_MEMORY_BANK_C256;
// 			break;
// 			
// 		case MACHINE_A2560U_PLUS:
// 		case MACHINE_A2560U:
// 			the_system->screen_[ID_CHANNEL_A]->vicky_ = P32(VICKY_A2560U);
// 			the_system->screen_[ID_CHANNEL_A]->text_ram_ = TEXT_RAM_A2560U;
// 			the_system->screen_[ID_CHANNEL_A]->text_attr_ram_ = TEXT_ATTR_A2560U;
// 			the_system->screen_[ID_CHANNEL_A]->text_font_ram_ = FONT_MEMORY_BANK_A2560U;
// 			the_system->screen_[ID_CHANNEL_B]->vicky_ = P32(VICKY_A2560U);
// 			the_system->screen_[ID_CHANNEL_B]->text_ram_ = TEXT_RAM_A2560U;
// 			the_system->screen_[ID_CHANNEL_B]->text_attr_ram_ = TEXT_ATTR_A2560U;
// 			the_system->screen_[ID_CHANNEL_B]->text_font_ram_ = FONT_MEMORY_BANK_A2560U;
// 			break;
// 			
// 		case MACHINE_A2560X:
// 		case MACHINE_A2560K:			
// 			the_system->screen_[ID_CHANNEL_A]->vicky_ = P32(VICKY_A2560K_A);
// 			the_system->screen_[ID_CHANNEL_A]->text_ram_ = TEXTA_RAM_A2560K;
// 			the_system->screen_[ID_CHANNEL_A]->text_attr_ram_ = TEXTA_ATTR_A2560K;
// 			the_system->screen_[ID_CHANNEL_A]->text_font_ram_ = FONT_MEMORY_BANKA_A2560K;
// 
// 			the_system->screen_[ID_CHANNEL_B]->vicky_ = P32(VICKY_A2560K_B);
// 			the_system->screen_[ID_CHANNEL_B]->text_ram_ = TEXTB_RAM_A2560K;
// 			the_system->screen_[ID_CHANNEL_B]->text_attr_ram_ = TEXTB_ATTR_A2560K;
// 			the_system->screen_[ID_CHANNEL_B]->text_font_ram_ = FONT_MEMORY_BANKB_A2560K;
// 		
// 			break;
// 		
// 		default:
// 			DEBUG_OUT(("%s %d: this application is not compatible with Foenix hardware ID %u.", __func__, __LINE__, the_system->model_number_));
// 			return false;
// 			break;
// 		
// 	}

	// set some things that aren't machine-dependent
	for (i = 0; i < the_system->num_screens_; i++)
	{
		the_system->screen_[i]->rect_.MinX = 0;
		the_system->screen_[i]->rect_.MinY = 0;	
		the_system->screen_[i]->text_temp_buffer_1_[0] = '\0';
		the_system->screen_[i]->text_temp_buffer_2_[0] = '\0';
		the_system->screen_[i]->text_font_width_ = TEXT_FONT_WIDTH_A2560;
		the_system->screen_[i]->text_font_height_ = TEXT_FONT_HEIGHT_A2560;

		// use auto configure to set resolution, text cols, margins, etc
		if (Sys_DetectScreenSize(the_system->screen_[i]) == false)
		{
			LOG_ERR(("%s %d: Unable to auto-configure screen resolution", __func__, __LINE__));
			return false;
		}

		// set standard color LUTs for text mode
		memcpy(the_system->screen_[i]->text_color_fore_ram_, &standard_text_color_lut, 64);
		memcpy(the_system->screen_[i]->text_color_back_ram_, &standard_text_color_lut, 64);
	
		DEBUG_OUT(("%s %d: This screen (id=%i: %i x %i, with %i x %i text (%i x %i visible)", __func__, __LINE__, 
			the_system->screen_[i]->id_,
			the_system->screen_[i]->width_, 
			the_system->screen_[i]->height_, 
			the_system->screen_[i]->text_mem_cols_, 
			the_system->screen_[i]->text_mem_rows_, 
			the_system->screen_[i]->text_cols_vis_, 
			the_system->screen_[i]->text_rows_vis_
			));
	}
	
	// always enable gamma correction
	Sys_SetGammaMode(the_system, the_system->screen_[0], true);
	
	return true;
}


//! Switch machine into graphics mode
//! @param	the_system: valid pointer to system object
bool Sys_SetModeGraphics(System* the_system)
{	
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was NULL", __func__, __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   On an A2560K or X, the only screen that has a text/graphics mode is the Channel B screen
	
	// switch to graphics mode by setting graphics mode bit, and setting bitmap engine enable bit

	// enable bitmap layers 0 and 1
	#ifdef _C256_FMX_
		R8(VICKY_II_MASTER_CTRL_REG_L) = *(uint8_t*)VICKY_II_MASTER_CTRL_REG_L & (GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);
		//R8(the_system->screen_[ID_CHANNEL_B]->vicky_) = *(uint8_t*)the_system->screen_[ID_CHANNEL_B]->vicky_ & (GRAPHICS_MODE_MASK | GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);
		R8(BITMAP_L0_CTRL) = 0x01;
		R8(BITMAP_L1_CTRL) = 0x00; // TEMP... not sure layer 1 is getting cleared.
	#else
		//*the_screen->vicky_ = (*the_screen->vicky_ & GRAPHICS_MODE_MASK | (GRAPHICS_MODE_GRAPHICS) | GRAPHICS_MODE_EN_BITMAP);
		R32(the_system->screen_[ID_CHANNEL_B]->vicky_) = (*the_system->screen_[ID_CHANNEL_B]->vicky_ & GRAPHICS_MODE_MASK | GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);

		R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_CTRL_L) = 0x01;
		#ifndef _f68_	// f68 does not do compositing, so if bitmap layer1 is enabled, it blocks everything else
		R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L1_CTRL_L) = 0x01;
		#else
		R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L1_CTRL_L) = 0x00;
		#endif
	#endif
	
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Switch machine into text mode
//! @param	the_system: valid pointer to system object
//! @param as_overlay: If true, sets text overlay mode (text over graphics). If false, sets full text mode (no graphics);
bool Sys_SetModeText(System* the_system, bool as_overlay)
{	
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was NULL", __func__, __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   On an A2560K or X, the only screen that has a text/graphics mode is the Channel B screen
	
	if (as_overlay)
	{
		// switch to text mode with overlay by setting graphics mode bit, setting bitmap engine enable bit, and setting graphics mode overlay		
		#ifdef _C256_FMX_
			R8(VICKY_II_MASTER_CTRL_REG_L) = (GRAPHICS_MODE_TEXT | GRAPHICS_MODE_TEXT_OVER | GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);
			//R8(VICKY_II_MASTER_CTRL_REG_L) = *(uint8_t*)VICKY_II_MASTER_CTRL_REG_L & (GRAPHICS_MODE_TEXT | GRAPHICS_MODE_TEXT_OVER | GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);
			R8(BITMAP_L0_CTRL) = 0x01;
			R8(BITMAP_L1_CTRL) = 0x00; // TEMP... not sure layer 1 is getting cleared.
			//R8(the_system->screen_[ID_CHANNEL_B]->vicky_) = *(uint8_t*)the_system->screen_[ID_CHANNEL_B]->vicky_ & (GRAPHICS_MODE_MASK | GRAPHICS_MODE_TEXT | GRAPHICS_MODE_TEXT_OVER | GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);
		#else
			R32(the_system->screen_[ID_CHANNEL_B]->vicky_) = (*the_system->screen_[ID_CHANNEL_B]->vicky_ & GRAPHICS_MODE_MASK | GRAPHICS_MODE_TEXT | GRAPHICS_MODE_TEXT_OVER | GRAPHICS_MODE_GRAPHICS | GRAPHICS_MODE_EN_BITMAP);
		#endif
		
		// c256foenix, discord 2022/03/10
		// Normally, for example, if you setup everything to be in bitmap mode, and you download an image in VRAM and you can see it properly... If you turn on overlay, then you will see on top of that same image, your text that you had before.
		// Mstr_Ctrl_Text_Mode_En  = $01       ; Enable the Text Mode
		// Mstr_Ctrl_Text_Overlay  = $02       ; Enable the Overlay of the text mode on top of Graphic Mode (the Background Color is ignored)
		// Mstr_Ctrl_Graph_Mode_En = $04       ; Enable the Graphic Mode
		// Mstr_Ctrl_Bitmap_En     = $08       ; Enable the Bitmap Module In Vicky
		// all of these should be ON
	}
	else
	{
		#ifdef _C256_FMX_
			R8(VICKY_II_MASTER_CTRL_REG_L) = (GRAPHICS_MODE_TEXT);
			//R8(VICKY_II_MASTER_CTRL_REG_L) = *(uint8_t*)VICKY_II_MASTER_CTRL_REG_L & (GRAPHICS_MODE_TEXT);
			//R8(the_system->screen_[ID_CHANNEL_B]->vicky_) = *(uint8_t*)the_system->screen_[ID_CHANNEL_B]->vicky_ & (GRAPHICS_MODE_MASK | GRAPHICS_MODE_TEXT);
			// disable bitmap layers 0 and 1
			R8(BITMAP_L0_CTRL) = 0x00;
			R8(BITMAP_L1_CTRL) = 0x00;
		#else
			R32(the_system->screen_[ID_CHANNEL_B]->vicky_) = (*the_system->screen_[ID_CHANNEL_B]->vicky_ & GRAPHICS_MODE_MASK | GRAPHICS_MODE_TEXT);
			// disable bitmap layers 0 and 1
			R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_CTRL_L) = 0x00;
			R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L1_CTRL_L) = 0x00;
		#endif
	}
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Detect the current screen mode/resolution, and set # of columns, rows, H pixels, V pixels, accordingly
bool Sys_DetectScreenSize(Screen* the_screen)
{
	screen_resolution	new_mode;
	uint32_t			the_vicky_value;
	uint8_t				the_video_mode_bits;
	uint32_t			the_border_control_value;
	int16_t				border_x_cols;
	int16_t				border_y_cols;
	int16_t				border_x_pixels;
	int16_t				border_y_pixels;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		goto error;
	}

	// detect the video mode and set resolution based on it
	
	//v = (unsigned char*)the_screen->vicky_;
	#ifndef _C256_FMX_
		the_vicky_value = *the_screen->vicky_;
		the_video_mode_bits = (the_vicky_value >> 8) & 0xff;
		//DEBUG_OUT(("%s %d: 32bit vicky value=%x, video mode bits=%x", __func__, __LINE__, the_vicky_value, the_video_mode_bits));
	#else
		uint8_t*	vicky_8bit_ptr = (uint8_t*)the_screen->vicky_;
		vicky_8bit_ptr++;
		the_video_mode_bits = *vicky_8bit_ptr;
		//DEBUG_OUT(("%s %d: 8bit vicky ptr 2nd byte=%p, video mode bits=%x", __func__, __LINE__, vicky_8bit_ptr, the_video_mode_bits));
	#endif
	
	if (the_screen->vicky_ == P32(VICKY_A2560K_A))
	{
		// LOGIC: A2560K channel A only has 2 video modes, 1024x768 and 800x600. If bit 11 is set, it's 1024. 
		
		//DEBUG_OUT(("%s %d: vicky identified as VICKY_A2560K_A", __func__, __LINE__));

		if (the_video_mode_bits & VICKY_IIIA_RES_1024X768_FLAGS)
		{
			new_mode = RES_1024X768;
		}
		else
		{
			new_mode = RES_800X600;
		}
	}
	else if (the_screen->vicky_ == P32(VICKY_A2560K_B))
	{
		// LOGIC: 
		//   A2560K channel B only has 3 video modes, 800x600, 640x480, and 640x400 (currently non-functional)
		//   if bit 8 is set, it's 800x600, if bits 8/9 both set, it's 640x400. 

		//DEBUG_OUT(("%s %d: vicky identified as VICKY_A2560K_B", __func__, __LINE__));

		if (the_video_mode_bits & VICKY_IIIB_RES_800X600_FLAGS)
		{
			new_mode = RES_800X600;
		}
		else if (the_video_mode_bits & VICKY_IIIB_RES_640X400_FLAGS)
		{
			new_mode = RES_640X400;
		}
		else
		{
			new_mode = RES_640X480;
		}
	}
	else if (the_screen->vicky_ == P32(VICKY_A2560U))
	{
 		//   A2560U has 1 channel with 3 video modes, 800x600, 640x480, and 640x400 (currently non-functional)
		//   if bit 8 is set, it's 800x600, if bits 8/9 both set, it's 640x400. 

		//DEBUG_OUT(("%s %d: vicky identified as VICKY_A2560U", __func__, __LINE__));

		if (the_video_mode_bits & VICKY_II_RES_800X600_FLAGS)
		{
			new_mode = RES_800X600;
		}
		else if (the_video_mode_bits & VICKY_II_RES_640X400_FLAGS)
		{
			new_mode = RES_640X400;
		}
		else
		{
			new_mode = RES_640X480;
		}
	}
	else if (the_screen->vicky_ == P32(VICKY_C256))
	{
 		//   C256FMX has 1 channel with 2 video modes, 800x600 and 640x480
		//   if bit 8 is set, it's 800x600, if not set it's 640x480. VIDEO_MODE_BIT0/VIDEO_MODE_BIT1
		//   if bit 9 is set, it doubles pixel size, bringing resolution down to 400x300 or 320x200. VICKY_II_PIX_DOUBLER_FLAGS

		DEBUG_OUT(("%s %d: vicky identified as VICKY_C256", __func__, __LINE__));

		if (the_video_mode_bits & VIDEO_MODE_BIT1)
		{
			new_mode = RES_800X600;
		}
		else
		{
			new_mode = RES_640X480;
		}
	}
	else
	{
		LOG_ERR(("%s %d: The VICKY register on this machine (%p) doesn't match one I know of. I won't be able to figure out what the screen size is.", __func__, __LINE__, the_screen->vicky_));
		return false;
	}

	switch (new_mode)
	{
		case RES_640X400:
			the_screen->width_ = 640;	
			the_screen->height_ = 400;
			DEBUG_OUT(("%s %d: set to RES_640X400", __func__, __LINE__));
			break;
			
		case RES_640X480:
			the_screen->width_ = 640;	
			the_screen->height_ = 480;
			DEBUG_OUT(("%s %d: set to RES_640X480", __func__, __LINE__));
			break;
			
		case RES_800X600:
			the_screen->width_ = 800;	
			the_screen->height_ = 600;
			DEBUG_OUT(("%s %d: set to RES_800X600", __func__, __LINE__));
			break;
			
		case RES_1024X768:
			the_screen->width_ = 1024;	
			the_screen->height_ = 768;
			DEBUG_OUT(("%s %d: set to 1024x768", __func__, __LINE__));
			break;		
	}
	
	// detect borders, and set text cols/rows based on resolution modified by borders (if any)
	#ifdef _C256_FMX_
		border_x_pixels = R8(VICKY_II_BORDER_X_SIZE);
		border_y_pixels = R8(VICKY_II_BORDER_Y_SIZE);
		//DEBUG_OUT(("%s %d: border x,y=%i,%i", __func__, __LINE__, R8(VICKY_II_BORDER_X_SIZE), R8(VICKY_II_BORDER_Y_SIZE)));
	#else
		the_border_control_value = R32(the_screen->vicky_ + BORDER_CTRL_OFFSET_L);
		border_x_pixels = (the_border_control_value >> 8) & 0xFF & 0x3F;
		border_y_pixels = (the_border_control_value >> 16) & 0xFF & 0x3F;
	#endif
	
	border_x_cols = border_x_pixels * 2 / the_screen->text_font_width_;
	border_y_cols = border_y_pixels * 2 / the_screen->text_font_height_;
	the_screen->text_mem_cols_ = the_screen->width_ / the_screen->text_font_width_;
	the_screen->text_mem_rows_ = the_screen->height_ / the_screen->text_font_height_;
	the_screen->text_cols_vis_ = the_screen->text_mem_cols_ - border_x_cols;
	the_screen->text_rows_vis_ = the_screen->text_mem_rows_ - border_y_cols;
	the_screen->rect_.MaxX = the_screen->width_;
	the_screen->rect_.MaxY = the_screen->height_;	
	Sys_PrintScreen(the_screen);
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Change video mode to the one passed.
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param	new_mode: One of the enumerated screen_resolution values. Must correspond to a valid VICKY video mode for the host machine. See VICKY_IIIA_RES_800X600_FLAGS, etc. defined in a2560_platform.h
//! @return	returns false on any error/invalid input.
bool Sys_SetVideoMode(Screen* the_screen, screen_resolution new_mode)
{
	unsigned char	new_mode_flag = 0xFF;
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		goto error;
	}

	// TODO: figure out smarter way of knowing which video modes are legal for the machine being run on
	if (the_screen->vicky_ == P32(VICKY_A2560K_A))
	{
		//DEBUG_OUT(("%s %d: vicky identified as VICKY_A2560K_A", __func__, __LINE__));
		
		if (new_mode == RES_800X600)
		{
			new_mode_flag = VICKY_IIIA_RES_800X600_FLAGS;
		}
		else if (new_mode == RES_1024X768)
		{
			new_mode_flag = VICKY_IIIA_RES_1024X768_FLAGS;
		}
	}
	else if (the_screen->vicky_ == P32(VICKY_A2560K_B))
	{
		//DEBUG_OUT(("%s %d: vicky identified as VICKY_A2560K_B", __func__, __LINE__));
		
		if (new_mode == RES_640X400)
		{
			LOG_WARN(("%s %d: 640x400 mode is not yet available in hardware", __func__, __LINE__));
			//new_mode_flag = VICKY_IIIB_RES_640X400_FLAGS;
		}
		else if (new_mode == RES_640X480)
		{
			new_mode_flag = VICKY_IIIB_RES_640X480_FLAGS;
		}
		else if (new_mode == RES_800X600)
		{
// 			DEBUG_OUT(("%s %d: RES_800X600", __func__, __LINE__));
			new_mode_flag = VICKY_IIIB_RES_800X600_FLAGS;
		}
	}
	else if (the_screen->vicky_ == P32(VICKY_A2560U))
	{
 		//DEBUG_OUT(("%s %d: vicky identified as VICKY_A2560U", __func__, __LINE__));
		if (new_mode == RES_640X400)
		{
			new_mode_flag = VICKY_II_RES_640X400_FLAGS;
		}
		else if (new_mode == RES_640X480)
		{
			new_mode_flag = VICKY_II_RES_640X480_FLAGS;
		}
		else if (new_mode == RES_800X600)
		{
			new_mode_flag = VICKY_II_RES_800X600_FLAGS;
		}
	}
	else if (the_screen->vicky_ == P32(VICKY_C256))
	{
 		//DEBUG_OUT(("%s %d: vicky identified as VICKY_C256", __func__, __LINE__));
		if (new_mode == RES_640X480)
		{
			new_mode_flag = VICKY_II_RES_640X480_FLAGS;
		}
		else if (new_mode == RES_800X600)
		{
			new_mode_flag = VICKY_II_RES_800X600_FLAGS;
		}
	}
 	//DEBUG_OUT(("%s %d: specified video mode = %u, flag=%u", __func__, __LINE__, new_mode, new_mode_flag));
	
	if (new_mode_flag == 0xFF)
	{
		LOG_ERR(("%s %d: specified video mode is not legal for this screen %u", __func__, __LINE__, new_mode));
		return false;
	}
	
 	//DEBUG_OUT(("%s %d: vicky before = %x", __func__, __LINE__, *the_screen->vicky_ ));
	*the_screen->vicky_ = (*the_screen->vicky_ & VIDEO_MODE_MASK | (new_mode_flag << 8));
 	//DEBUG_OUT(("%s %d: vicky after = %x", __func__, __LINE__, *the_screen->vicky_ ));
	
	// teach screen about the new settings
	if (Sys_DetectScreenSize(the_screen) == false)
	{
		LOG_ERR(("%s %d: Changed screen resolution, but the selected resolution could not be handled", __func__, __LINE__, new_mode));
		return false;
	}

	// tell the MCP that we changed res so it can update it's internal col sizes, etc.  - this function is not exposed in MCP headers yet
	//sys_text_setsizes();
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// enable or disable the gamma correction 
bool Sys_SetGammaMode(System* the_system, Screen* the_screen, bool enable_it)
{
	unsigned char	new_mode_flag;

	// LOGIC:
	//   both C256s and A2560s have a gamma correction mode
	//   It needs to be hardware enabled by turning DIP switch 7 on the motherboard to ON (I believe)
	//     bit 5 (0x20) of the video mode byte in vicky master control reflects the DIP switch setting, but doesn't change anything if you write to it 
	//   byte 3 of the vicky master control appears to be dedicated to Gamma correction, but not all bits are documented. Stay away from all but the first 2!
	//     gamma correction can be activated by setting the first and 2nd bits of byte 3
	
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		goto error;
	}

	if (enable_it)
	{
		new_mode_flag = 0xFF;
	}
	else
	{
		new_mode_flag = 0x00;
	}

	#ifdef _C256_FMX_
		uint8_t		the_gamma_mode_bits = R8(VICKY_II_GAMMA_CTRL_REG);
		
 		//DEBUG_OUT(("%s %d: vicky byte 3 before gamma change = %x", __func__, __LINE__, the_gamma_mode_bits));
		the_gamma_mode_bits |= (GAMMA_MODE_ONOFF_BITS & new_mode_flag);
		R8(VICKY_II_GAMMA_CTRL_REG) = the_gamma_mode_bits;
 		//DEBUG_OUT(("%s %d: vicky byte 3 after gamma change = %x, %x", __func__, __LINE__, the_gamma_mode_bits, R8(VICKY_II_GAMMA_CTRL_REG)));
 		//DEBUG_OUT(("%s %d: wrote to %x to register at %p", __func__, __LINE__, the_gamma_mode_bits, P8(VICKY_II_GAMMA_CTRL_REG)));
	#else
		uint32_t	the_vicky_value = R32(the_screen->vicky_);
		uint8_t		the_gamma_mode_bits;

		the_gamma_mode_bits = (the_vicky_value >> 24) & 0xff;
		//DEBUG_OUT(("%s %d: before: 32bit vicky value=%x, gamma mode bits=%x", __func__, __LINE__, the_vicky_value, the_gamma_mode_bits));
		the_gamma_mode_bits &= (GAMMA_MODE_ONOFF_BITS & new_mode_flag);
		the_vicky_value &= (GAMMA_MODE_MASK | the_gamma_mode_bits << 24);
 		//DEBUG_OUT(("%s %d: after: 32bit vicky value=%x, gamma mode bits=%x", __func__, __LINE__, the_vicky_value, the_gamma_mode_bits));
		R32(the_screen->vicky_) = the_vicky_value;
	#endif
	
	return true; 
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}



//! Enable or disable the hardware cursor in text mode, for the specified screen
//! @param	the_system: valid pointer to system object
//! @param	the_screen: valid pointer to the target screen to operate on
//! @param enable_it: If true, turns the hardware blinking cursor on. If false, hides the hardware cursor;
bool Sys_EnableTextModeCursor(System* the_system, Screen* the_screen, bool enable_it)
{
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		goto error;
	}

	if (enable_it)
	{
		R32(the_screen->vicky_ + CURSOR_CTRL_OFFSET_L) = 1;
	}
	else
	{
		R32(the_screen->vicky_ + CURSOR_CTRL_OFFSET_L) = 0;
	}
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Set the left/right and top/bottom borders
//! This will reset the visible text columns as a side effect
//! Grotesquely large values will be accepted as is: use at your own risk!
//! @param	border_width: width in pixels of the border on left and right side of the screen. Total border used with be the double of this.
//! @param	border_height: height in pixels of the border on top and bottom of the screen. Total border used with be the double of this.
//! @return	returns false on any error/invalid input.
bool Sys_SetBorderSize(System* the_system, Screen* the_screen, uint8_t border_width, uint8_t border_height)
{
	if (the_screen == NULL)
	{
		LOG_ERR(("%s %d: passed screen was NULL", __func__, __LINE__));
		goto error;
	}
	
	// set borders
	#ifdef _C256_FMX_
		R8(VICKY_II_BORDER_X_SIZE) = border_width;
		R8(VICKY_II_BORDER_Y_SIZE) = border_height;
	#else
		uint32_t the_border_control_value;
		
		the_border_control_value = R32(the_screen->vicky_ + BORDER_CTRL_OFFSET_L);
		the_border_control_value = (the_border_control_value & 0xFFFF00FF) | ((uint32_t)border_width <<  8);
		the_border_control_value = (the_border_control_value & 0xFF00FFFF) | ((uint32_t)border_height <<  16);
		R32(the_screen->vicky_ + BORDER_CTRL_OFFSET_L) = the_border_control_value;
	#endif
	
	// now we need to recalculate how many text cols/rows are visible, because it might have changed
	the_screen->text_cols_vis_ = the_screen->text_mem_cols_ - border_width * 2 / the_screen->text_font_width_;
	the_screen->text_rows_vis_ = the_screen->text_mem_rows_ - border_height * 2 / the_screen->text_font_height_;
		
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}



// **** Window management functions *****


//! Add this window to the list of windows and make it the currently active window
//! @param	the_system: valid pointer to system object
//! @return	Returns false if adding this window would exceed the system's hard cap on the number of available windows
bool Sys_AddToWindowList(System* the_system, Window* the_new_window)
{
	List*	the_new_item;
	int8_t	new_display_order;
	
	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
 	if (the_new_window == NULL)
 	{
		LOG_ERR(("%s %d: passed window object was null", __func__ , __LINE__));
		goto error;
 	}

	// are there too many windows already? 
	if (the_system->window_count_ >= SYS_MAX_WINDOWS)
	{
		LOG_ERR(("%s %d: No more windows can be opened!", __func__ , __LINE__));
		return false;
	}

	the_new_item = List_NewItem((void*)the_new_window);
	
	// have any windows been created yet? If not, need to allocate the pointer to the list
	if (the_system->window_count_ < 1)
	{
		// initiate the list of windows
		if ( (the_system->list_windows_ = (List**)calloc(1, sizeof(List*)) ) == NULL)
		{
			LOG_ERR(("%s %d: could not allocate memory to create new list of windows", __func__ , __LINE__));
			goto error;
		}
		LOG_ALLOC(("%s %d:	__ALLOC__	the_system->list_windows_	%p	size	%i", __func__ , __LINE__, the_system->list_windows_, sizeof(List*)));
		
		*the_system->list_windows_ = the_new_item;
	}
	else
	{
		List_AddItem(the_system->list_windows_, the_new_item);
	}
	
	new_display_order = SYS_MAX_WINDOWS;
	Window_SetDisplayOrder(the_new_window, new_display_order);
	
	++the_system->window_count_;
	
	Sys_SetActiveWindow(the_system, the_new_window);

//  	DEBUG_OUT(("%s %d: window count after=%i", __func__, __LINE__, the_system->window_count_));
// 	List_Print(the_system->list_windows_, (void*)&Window_PrintBrief);
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// create the backdrop window for the system
//! @param	the_system: valid pointer to system object
bool Sys_CreateBackdropWindow(System* the_system)
{
	Screen*				the_screen;
	Window*				the_window;
	NewWinTemplate*		the_win_template;
	static char*		the_win_title = "_backdrop";

 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}

	if ( (the_win_template = Window_GetNewWinTemplate(the_win_title)) == NULL)
	{
		LOG_ERR(("%s %d: Could not get a new window template", __func__ , __LINE__));
		goto error;
	}
		
	the_screen = the_system->screen_[ID_CHANNEL_B];
	
	the_win_template->x_ = 0;
	the_win_template->y_ = 0;
	the_win_template->width_ = the_screen->width_;
	the_win_template->height_ = the_screen->height_;
	the_win_template->min_width_ = the_screen->width_;
	the_win_template->min_height_ = the_screen->height_;
	the_win_template->max_width_ = the_screen->width_;
	the_win_template->max_height_ = the_screen->height_;

	the_win_template->type_ = WIN_BACKDROP;
	the_win_template->is_backdrop_ = true;
	the_win_template->can_resize_ = false;
	
	if ( (the_window = Window_New(the_win_template, &Window_BackdropWinEventHandler)) == NULL)
	{
		DEBUG_OUT(("%s %d: Couldn't instantiate the backdrop window", __func__, __LINE__));
		goto error;
	}

	// make the window visible (Window_New always sets windows to invisible so they don't render before you need them to)
	Window_SetVisible(the_window, true);
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// return the active window
//! @param	the_system: valid pointer to system object
Window* Sys_GetActiveWindow(System* the_system)
{
 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	return the_system->active_window_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// return the backdrop window
//! @param	the_system: valid pointer to system object
Window* Sys_GetBackdropWindow(System* the_system)
{
 	List*	the_item;
 	
 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		if (Window_IsBackdrop(this_window))
		{
			return this_window;
		}

		the_item = the_item->next_item_;
	}
	
	return NULL;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// return a reference to the next window in the system's list, excluding backdrop windows
//! @param	the_system: valid pointer to system object
Window* Sys_GetNextWindow(System* the_system)
{
	Window*		current_window;
	Window*		next_window;
	List*		current_window_item;
	List*		next_window_item;
	bool		ok = false;
	bool		looped = false;

 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	if (the_system->window_count_ < 1)
	{
		return NULL;
	}
	
	current_window = Sys_GetActiveWindow(the_system);

	DEBUG_OUT(("%s %d: window list:", __func__, __LINE__));
	List_Print(the_system->list_windows_, (void*)&Window_PrintBrief);

	// if no active window (possible on a window close), then use the first window as starting point
	// otherwise, use active window as starting point
	if (current_window == NULL)
	{
		current_window_item = List_GetFirst(the_system->list_windows_);
		LOG_ERR(("%s %d: no active window, first item=%p", __func__ , __LINE__, current_window_item));
		looped = true;
	}
	else
	{
		current_window_item = List_FindThisObject(the_system->list_windows_, (void*)current_window);
	}	

	if (current_window_item == NULL)
	{
		LOG_ERR(("%s %d: couldn't find current window in the list of windows", __func__ , __LINE__));
		goto error;
	}
	
	next_window_item = current_window_item->next_item_;
	
	while (!ok)
	{	
		if (next_window_item == NULL)
		{
			// loop back to start of List
			if (looped == false)
			{
				next_window_item = List_GetFirst(the_system->list_windows_);
				looped = true;
				DEBUG_OUT(("%s %d: going back to first item in window list", __func__ , __LINE__));
			}
			else
			{
				DEBUG_OUT(("%s %d: have already looped once; giving up", __func__ , __LINE__));
				current_window = (Window*)current_window_item->payload_; // there is always ONE window until we do Sys_Destroy()
				return current_window;
			}
		}
		else
		{
			// check for backdrop and skip.
			next_window = (Window*)next_window_item->payload_;
			DEBUG_OUT(("%s %d: backdrop check: this window is '%s'", __func__ , __LINE__, next_window->title_));

			if (next_window->is_backdrop_)
			{
				next_window_item = next_window_item->next_item_;
				DEBUG_OUT(("%s %d: was backdrop", __func__ , __LINE__));
			}
			else
			{
				ok = true;
				DEBUG_OUT(("%s %d: ok reached", __func__ , __LINE__));
			}
		}
	}
	
	return next_window;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// return a reference to the previous window in the system's list, excluding backdrop windows
//! @param	the_system: valid pointer to system object
Window* Sys_GetPreviousWindow(System* the_system)
{
	Window*		current_window;
	Window*		next_window;
	List*		current_window_item;
	List*		next_window_item;
	bool		ok = false;

 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	if (the_system->window_count_ < 1)
	{
		return NULL;
	}
	
	current_window = Sys_GetActiveWindow(the_system);

	if (current_window == NULL)
	{
		return NULL;
	}

	if (the_system->window_count_ < 2)
	{
		return current_window;
	}
	
	current_window_item = List_FindThisObject(the_system->list_windows_, (void*)current_window);

	if (current_window_item == NULL)
	{
		LOG_ERR(("%s %d: couldn't find current window in the list of windows", __func__ , __LINE__));
		goto error;
	}
	
	next_window_item = current_window_item->prev_item_;
	
	while (!ok)
	{	
		if (next_window_item == NULL)
		{
			// loop back to start of List
			next_window_item = List_GetLast(the_system->list_windows_);
		}
		else
		{
			// check for backdrop and skip.
			next_window = (Window*)next_window_item->payload_;

			if (next_window->is_backdrop_)
			{
				next_window_item = next_window_item->prev_item_;
			}
			else
			{
				ok = true;
			}
		}
	}
	
	return next_window;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// Find the Window under the mouse -- accounts for z depth (topmost window will be found)
//! @param	the_system: valid pointer to system object
//! @param	x: global horizontal coordinate
//! @param	y: global vertical coordinate
Window* Sys_GetWindowAtXY(System* the_system, int16_t x, int16_t y)
{
 	List*	the_item;

 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	if (the_system->window_count_ < 1)
	{
		return NULL;
	}
	
	if (x < 0 || y < 0)
	{
		return NULL;
	}
	
	// LOGIC:
	//   OS/f windows are all known by the system
	//   each window has a display order property set by the system, from low to high being backmost to frontmost
	//   the system also keeps them sorted in its list, with first item being front most
		
	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
 		bool		in_this_win;
		Window*		this_window = (Window*)(the_item->payload_);
		
		in_this_win = General_PointInRect(x, y, this_window->global_rect_);
		
		if (in_this_win)
		{
			DEBUG_OUT(("%s %d: window at %i, %i = '%s'", __func__, __LINE__, x, y, this_window->title_));
			return this_window;
		}

		the_item = the_item->next_item_;
	}
	
	return NULL;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! Set the passed window to the active window, and marks the previously active window as inactive
//! NOTE: This will resort the list of windows to move the (new) active one to the front
//! NOTE: The exception to this is that the backdrop window is never moved in front of other windows
//! @param	the_system: valid pointer to system object
bool Sys_SetActiveWindow(System* the_system, Window* the_window)
{
	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}

	if (the_system->active_window_ != NULL)
	{
		Window_SetActive(the_system->active_window_, false);
	}
	
	// Before making the new window active, survey any windows in front of it, and add their shapes to this window's damage rects so it can fix itself.
	Sys_CollectDamageRects(the_system, the_window);
	
	the_system->active_window_ = the_window;

	Window_SetActive(the_window, true);

	if (the_window->is_backdrop_)
	{
		return true;
	}
	
	//DEBUG_OUT(("%s %d: window list before change of active window:", __func__, __LINE__));
	//List_Print(the_system->list_windows_, (void*)&Window_PrintBrief);
	
	the_window->display_order_ = SYS_WIN_Z_ORDER_NEWLY_ACTIVE;
	List_InitMergeSort(the_system->list_windows_, &Window_CompareDisplayOrder);
	
	//DEBUG_OUT(("%s %d: window list after change of active window:", __func__, __LINE__));
	//List_Print(the_system->list_windows_, (void*)&Window_PrintBrief);
	
	// that changes their linked order, but doesn't renumber their display_order_; need that too
	Sys_RenumberWindows(the_system);
	
	// temp
	Sys_Render(global_system);
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


// List-sort compatible function for sorting windows by their display order property
bool Window_CompareDisplayOrder(void* first_payload, void* second_payload)
{
	Window*		win_1 = (Window*)first_payload;
	Window*		win_2 = (Window*)second_payload;

	if (win_1->display_order_ < win_2->display_order_)
	{
		//DEBUG_OUT(("%s %d: win1 ('%s') behind win2 ('%s')", __func__, __LINE__, win_1->title_, win_2->title_));
		return true;
	}
	else
	{
		//DEBUG_OUT(("%s %d: win1 ('%s') in front of win2 ('%s')", __func__, __LINE__, win_1->title_, win_2->title_));
		return false;
	}
}


// remove one window from system's list of windows, and close it
void Sys_CloseOneWindow(System* the_system, Window* the_window)
{
	bool		need_different_active_window = false;
	List*		this_window_item;
	Rectangle	the_new_rect;

 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	// check that we have a window
	if (the_window == NULL)
	{
		LOG_ERR(("%s %d: passed a null window -- no window to close", __func__ , __LINE__));
		goto error;
	}

	this_window_item = List_FindThisObject(the_system->list_windows_, (void*)the_window);

	if (this_window_item == NULL)
	{
		LOG_ERR(("%s %d: couldn't find current window in the list of windows", __func__ , __LINE__));
		goto error;
	}
	
	// nullify any upcoming events that reference this Window
	EventManager_RemoveEventsForWindow(the_window);
	
	// before destroying the window, calculate and distribute any damage rects that may result from it being removed from screen
	the_new_rect.MinX = -2;
	the_new_rect.MinY = -2;
	the_new_rect.MaxX = -1;
	the_new_rect.MaxY = -1;
	Window_GenerateDamageRects(the_window, &the_new_rect);
	Sys_IssueDamageRects(the_system);
	
	// if this was active window, we'll need to pick a new one after we delete it
	need_different_active_window = (the_system->active_window_ == the_window);
	DEBUG_OUT(("%s %d: closing active window=%i", __func__ , __LINE__, need_different_active_window));
	
	if (need_different_active_window)
	{
		the_system->active_window_ = NULL;
	}
	
	// destroy the window, making sure to set a new active window
	Window_Destroy(&the_window);
	DEBUG_OUT(("%s %d: window destroyed", __func__ , __LINE__));
	--the_system->window_count_;
	List_RemoveItem(the_system->list_windows_, this_window_item);
	LOG_ALLOC(("%s %d:	__FREE__	the_item	%p	size	%i", __func__ , __LINE__, this_window_item, sizeof(List)));
	free(this_window_item);
	this_window_item = NULL;
	
	if (need_different_active_window)
	{
		Window*		the_next_window;

		DEBUG_OUT(("%s %d: trying to get next window...", __func__ , __LINE__));
		the_next_window = Sys_GetNextWindow(the_system);

		if (the_next_window != NULL)
		{
			Sys_SetActiveWindow(the_system, the_next_window); // this also calls system render
		}

		DEBUG_OUT(("%s %d: new active window='%s'", __func__ , __LINE__, the_system->active_window_->title_));		
	}

	// Re-render all windows
	Sys_Render(global_system);		
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Issue damage rects from the Active Window down to each other window in the system so that they can redraw portions of themselves
//! Note: does not call for system re-render
void Sys_IssueDamageRects(System* the_system)
{
	List*		the_item;
	Window*		the_active_window;
	int16_t		num_rects;
	
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_system->list_windows_ == NULL)
	{
		LOG_ERR(("%s %d: the window list was NULL", __func__ , __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   This will be called when the active window has been resized or moved
	//   The goal of this function is to find out which parts of windows behind the active window were previously not exposed, and now are, so they windows can redraw those portions to Screen
	//   Because this function does not actually re-render every window, the order they are processed here does not matter.
	//   Windows are ordered in the window list, by Z order, from back (head) to front (tail)

	the_active_window = Sys_GetActiveWindow(global_system);
	num_rects = the_active_window->damage_count_;
	
	DEBUG_OUT(("%s %d: active window '%s' has %i damage rects", __func__ , __LINE__, the_active_window->title_, num_rects));

	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		//DEBUG_OUT(("%s %d: this_window '%s' has %i clip rects", __func__ , __LINE__, this_window->title_, this_window->clip_count_));

		if (this_window != the_active_window)
		{
			int16_t		i;
			
			for (i = 0; i < num_rects; i++)
			{
				if (Window_AcceptDamageRect(this_window, &the_active_window->damage_rect_[i]) == false)
				{
				}
			}			
		}

		the_item = the_item->next_item_;
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Issue damage rects from the menu down to every other window in the system so that they can redraw portions of themselves when menu closes
//! Note: does not call for system re-render
void Sys_IssueMenuDamageRects(System* the_system)
{
	List*		the_item;
	
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_system->list_windows_ == NULL)
	{
		LOG_ERR(("%s %d: the window list was NULL", __func__ , __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   This will be called when the menu is closed
	//   The goal of this function is to find out which parts of windows behind the menu were covered by the menu and need to be redrawn
	//   Because this function does not actually re-render every window, the order they are processed here does not matter.
	//   Windows are ordered in the window list, by Z order, from back (head) to front (tail)

	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		//DEBUG_OUT(("%s %d: this_window '%s' has %i clip rects", __func__ , __LINE__, this_window->title_, this_window->clip_count_));
		if (Window_AcceptDamageRect(this_window, &the_system->menu_manager_->global_rect_) == false)
		{
			LOG_ERR(("%s %d: Failed to apply menu damage rect to window '%s'", __func__ , __LINE__, this_window->title_));
			//goto error;
		}

		the_item = the_item->next_item_;
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Collect damage rects for a window that is about to be made the active (foremost) window, so it can redraw portions of itself that may have been covered up by other windows
//! Note: does not call for system re-render
void Sys_CollectDamageRects(System* the_system, Window* the_future_active_window)
{
	List*		the_item;
	
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_system->list_windows_ == NULL)
	{
		LOG_ERR(("%s %d: the window list was NULL", __func__ , __LINE__));
		goto error;
	}

	if (the_future_active_window == NULL)
	{
		LOG_ERR(("%s %d: passed window object was null", __func__ , __LINE__));
		goto error;
	}
	
	
	// LOGIC:
	//   This will be called when a non-active window is being brought to the foreground and made the active window
	//   The goal of this function is to find out which parts of that window have been behind the active window and any other windows, so those portions can be redrawn
	//   Windows are ordered in the window list, by Z order, from back (head) to front (tail)
	//   Because this function does not actually re-render every window, the order they are processed here does not matter.
	//     However, we do need to not include damage rects from windows that were under the window being brought forward
	//     This can be accomplished by walking through window list until we hit the window in question, then start collecting from the next window after that
	
	DEBUG_OUT(("%s %d: future active win '%s' has display order of %i", __func__ , __LINE__, the_future_active_window->title_, the_future_active_window->display_order_));

	// LOGIC:
	//   A backdrop window never needs to collect damage rects from a window above it on active/inactive change
	//   That is because no matter what the order is, the backdrop window is always at the very back. 
	//   Windows only need to collect damage rects from windows above them if they can actually get in front of those windows
	if (the_future_active_window->is_backdrop_)
	{
		return;
	}
	
	the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		DEBUG_OUT(("%s %d: this_window '%s' has display order of %i", __func__ , __LINE__, this_window->title_, this_window->display_order_));

		if (this_window->display_order_ > the_future_active_window->display_order_)
		{
			if (Window_AcceptDamageRect(the_future_active_window, &this_window->global_rect_) == false)
			{
			}
		}

		the_item = the_item->next_item_;
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}




// **** Other GET functions *****


//! @param	the_system: valid pointer to system object
Theme* Sys_GetTheme(System* the_system)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_system->theme_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}

//! @param	the_system: valid pointer to system object
Font* Sys_GetSystemFont(System* the_system)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_system->system_font_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! @param	the_system: valid pointer to system object
Font* Sys_GetAppFont(System* the_system)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_system->app_font_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! @param	the_system: valid pointer to system object
Screen* Sys_GetScreen(System* the_system, int16_t channel_id)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (channel_id != ID_CHANNEL_A && channel_id != ID_CHANNEL_B)
	{
		LOG_ERR(("%s %d: passed channel_id (%i) was invalid", __func__ , __LINE__, channel_id));
		goto error;
	}
	
	return the_system->screen_[channel_id];
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! @param	the_system: valid pointer to system object
Menu* Sys_GetMenu(System* the_system)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_system->menu_manager_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! NOTE: Foenix systems only have 1 screen with bitmap graphics, even if the system has 2 screens overall. The bitmap returned will always be from the appropriate channel (A or B).
//! @param	the_system: valid pointer to system object
Bitmap* Sys_GetScreenBitmap(System* the_system, bitmap_layer the_layer)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_layer > fore_layer)
	{
		LOG_ERR(("%s %d: passed layer (%i) was invalid", __func__ , __LINE__, the_layer));
		goto error;
	}
	
	return the_system->screen_[ID_CHANNEL_B]->bitmap_[the_layer];
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


//! @param	the_system: valid pointer to system object
EventManager* Sys_GetEventManager(System* the_system)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	return the_system->event_manager_;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}





// **** Other SET functions *****

//! @param	the_system: valid pointer to system object
void Sys_SetSystemFont(System* the_system, Font* the_font)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_system->system_font_ = the_font;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! @param	the_system: valid pointer to system object
void Sys_SetAppFont(System* the_system, Font* the_font)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_system->app_font_ = the_font;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! @param	the_system: valid pointer to system object
void Sys_SetScreen(System* the_system, int16_t channel_id, Screen* the_screen)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (channel_id != ID_CHANNEL_A && channel_id != ID_CHANNEL_B)
	{
		LOG_ERR(("%s %d: passed channel_id (%i) was invalid", __func__ , __LINE__, channel_id));
		goto error;
	}
	
	the_system->screen_[channel_id] = the_screen;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! NOTE: Foenix systems only have 1 screen with bitmap graphics, even if the system has 2 screens overall. The bitmap returned will always be from the appropriate channel (A or B).
//! @param	the_system: valid pointer to system object
void Sys_SetScreenBitmap(System* the_system, Bitmap* the_bitmap, bitmap_layer the_layer)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_bitmap == NULL)
	{
		LOG_WARN(("%s %d: passed bitmap object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_layer > fore_layer)
	{
		LOG_ERR(("%s %d: passed layer (%i) was invalid", __func__ , __LINE__, the_layer));
		goto error;
	}
	
	the_system->screen_[ID_CHANNEL_B]->bitmap_[the_layer] = the_bitmap;
	
	Sys_SetVRAMAddr(the_system, the_layer, the_bitmap->addr_);
	
	DEBUG_OUT(("%s %d: layer=%i, bitmap_[the_layer]=%p", __func__, __LINE__, the_layer, the_bitmap->addr_));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the passed theme as the System's current theme
//! Note: this will dispose of the current theme after setting the new one
//! @param	the_system: valid pointer to system object
//! @return	Returns false on any error condition
bool Sys_SetTheme(System* the_system, Theme* the_theme)
{
	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_theme == NULL)
	{
		LOG_WARN(("%s %d: passed theme object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_system->theme_ != NULL)
	{
		Theme_Destroy(&the_system->theme_);
	}
	
	the_system->theme_ = the_theme;
	
	Sys_SetSystemFont(the_system, the_theme->control_font_);
	Sys_SetAppFont(the_system, the_theme->icon_font_);
	
	// if a menu has been loaded, have it change its font
	if (the_system->menu_manager_)
	{
		Menu_SetFont(the_system->menu_manager_, the_theme->icon_font_);
	}

	// have all windows update their controls / etc with info from new theme
	if (the_system->window_count_ > 0)
	{
		Sys_UpdateWindowTheme(the_system);
	
		// force re-render
		Sys_Render(the_system);
	}
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}





// **** xxx functions *****


//! Tell the VICKY to use a different address for the specified bitmap layer
//! @param	the_system: valid pointer to system object
//! @param	the_bitmap_layer: 0 or 1, the bitmap layer to get a new address
//! @param	the_address: The address within the VRAM zone that the bitmap layer should be repointed to
bool Sys_SetVRAMAddr(System* the_system, uint8_t the_bitmap_layer, unsigned char* the_address)
{
	uint32_t			new_vicky_bitmap_vram_value;

return true;

	if (the_system == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_bitmap_layer > 1)
	{
		LOG_ERR(("%s %d: passed bitmap layer number (%u) was invalid", __func__ , __LINE__, the_bitmap_layer));
		goto error;
	}
	
	DEBUG_OUT(("%s %d: VICKY VRAM for bitmap layer 0: want to point to bitmap at %p", __func__, __LINE__, the_address));

	//DEBUG_OUT(("%s %d: VICKY VRAM for bitmap layer 0 before change: 0x%x (with offset=0x%x)", __func__, __LINE__, R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_VRAM_ADDR_OFFSET_L), (uint32_t)VRAM_START + R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_VRAM_ADDR_OFFSET_L)));
	DEBUG_OUT(("%s %d: VICKY VRAM for bitmap layer %i before change: %x / %x / %x", __func__, __LINE__, the_bitmap_layer, R8(BITMAP_L0_VRAM_ADDR_L), R8(BITMAP_L0_VRAM_ADDR_M), R8(BITMAP_L0_VRAM_ADDR_H)));
	
	new_vicky_bitmap_vram_value = (uint32_t)the_address - (uint32_t)VRAM_START;		

	DEBUG_OUT(("%s %d: VICKY VRAM for bitmap layer %i about to change to: %p (with offset=%p)", __func__, __LINE__, the_bitmap_layer, P32(new_vicky_bitmap_vram_value), P32(the_address)));

	if (the_bitmap_layer == 0)
	{
		#ifdef _C256_FMX_
			R8(BITMAP_L0_VRAM_ADDR_L) = (new_vicky_bitmap_vram_value >> (8*1)) & 0xff;
			R8(BITMAP_L0_VRAM_ADDR_M) = (new_vicky_bitmap_vram_value >> (8*2)) & 0xff;
			R8(BITMAP_L0_VRAM_ADDR_H) = (new_vicky_bitmap_vram_value >> (8*3)) & 0xff;
			DEBUG_OUT(("%s %d: VICKY VRAM for bitmap layer %i AFTER change: %x / %x / %x", __func__, __LINE__, the_bitmap_layer, R8(BITMAP_L0_VRAM_ADDR_L), R8(BITMAP_L0_VRAM_ADDR_M), R8(BITMAP_L0_VRAM_ADDR_H)));
		#else
			R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_VRAM_ADDR_OFFSET_L) = new_vicky_bitmap_vram_value;
		#endif
	}
	else
	{
		#ifdef _C256_FMX_
			R8(BITMAP_L1_VRAM_ADDR_L) = (new_vicky_bitmap_vram_value >> (8*1)) & 0xff;
			R8(BITMAP_L1_VRAM_ADDR_M) = (new_vicky_bitmap_vram_value >> (8*2)) & 0xff;
			R8(BITMAP_L1_VRAM_ADDR_H) = (new_vicky_bitmap_vram_value >> (8*3)) & 0xff;
// 			R8(P8(the_system->screen_[ID_CHANNEL_B]->vicky_) + BITMAP_L1_VRAM_ADDR_L_B) = (new_vicky_bitmap_vram_value >> (8*1)) & 0xff;
// 			R8(P8(the_system->screen_[ID_CHANNEL_B]->vicky_) + BITMAP_L1_VRAM_ADDR_M_B) = (new_vicky_bitmap_vram_value >> (8*2)) & 0xff;
// 			R8(P8(the_system->screen_[ID_CHANNEL_B]->vicky_) + BITMAP_L1_VRAM_ADDR_H_B) = (new_vicky_bitmap_vram_value >> (8*3)) & 0xff;
		#else
			R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L1_VRAM_ADDR_OFFSET_L) = new_vicky_bitmap_vram_value;
		#endif
	}


	
	//DEBUG_OUT(("%s %d: VICKY VRAM for bitmap layer 0 now set to 0x%x (with offset=0x%x)", __func__, __LINE__, R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_VRAM_ADDR_OFFSET_L), (uint32_t)VRAM_START + R32(the_system->screen_[ID_CHANNEL_B]->vicky_ + BITMAP_L0_VRAM_ADDR_OFFSET_L)));
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}






// **** Render functions *****


//! Render all visible windows
//! NOTE: this will move to a private Sys function later, once event handling is available
//! @param	the_system: valid pointer to system object
void Sys_Render(System* the_system)
{
	int16_t		num_nodes = 0;
	List*		the_item;

 	if (the_system == NULL)
 	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
 	}
	
	// LOGIC:
	//   as we do not have regions and layers set up yet, we have to render every single window in its entirely, including the backdrop
	//   therefore, it is critical that the rendering take place in the order of back to front
	//   display order is built into the system's window list: the first item is the foremost, and the last is the backmost
	//   need to render from back of list towards front of list, so they built up over each other in right order.
	
	// have each window (re)render its controls/content/etc to its bitmap, and blit itself to the main screen/backdrop window bitmap
	
	if (the_system->list_windows_ == NULL)
	{
		DEBUG_OUT(("%s %d: the window list was NULL", __func__ , __LINE__));
		goto error;
	}
	
	//List_Print(the_system->list_windows_, (void*)&Window_PrintBrief);
	the_item = List_GetLast(the_system->list_windows_);
	//the_item = *(the_system->list_windows_);

	while (the_item != NULL)
	{
		Window*		this_window = (Window*)(the_item->payload_);
		
		//DEBUG_OUT(("%s %d: rendering window '%s'", __func__ , __LINE__, this_window->title_));
		
		if (Window_IsVisible(this_window) == true)
		{
			++num_nodes;
			Window_Render(this_window);

// 			// blit to screen
// 			Bitmap_Blit(this_window->bitmap_, 0, 0, the_system->screen_[ID_CHANNEL_B]->bitmap_, this_window->x_, this_window->y_, this_window->width_, this_window->height_);
		}

		the_item = the_item->prev_item_;
	}

	//DEBUG_OUT(("%s %d: %i windows rendered out of %i total window", __func__ , __LINE__, num_nodes, the_system->window_count_));
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}

