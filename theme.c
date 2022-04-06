/*
 * theme.c
 *
 *  Created on: Mar 26, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "theme.h"
#include "lib_sys.h"

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


/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern System*			global_system;

// ***** temporary storage here, until disk loading is available: base control graphics for 2 themes

// ** Default theme
	//byte array representing the picture - each byte is a CLUT index

	static const unsigned char def_theme_desktop_pattern[] = 
	{
		0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 
		0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 0xF6, 0xF6, 0xF5, 0xF6, 
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6,
	};

	static const unsigned char def_theme_close_btn_active_up[] = 
	{
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F,
		0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	};

	static const unsigned char def_theme_close_btn_active_dn[] = 
	{
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F,
		0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 
		0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	};

	static const unsigned char def_theme_close_btn_inactive_up[] = 
	{
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F,
		0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	};

	static const unsigned char def_theme_close_btn_inactive_dn[] = 
	{
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F,
		0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 
		0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	};
	
	static const unsigned char* def_theme_close_btn[2][2] = { {def_theme_close_btn_inactive_up, def_theme_close_btn_inactive_dn}, {def_theme_close_btn_active_up, def_theme_close_btn_active_dn}};


	static const unsigned char def_theme_minimize_btn_active_up[] = 
	{
		0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 
	};

	static const unsigned char def_theme_minimize_btn_active_dn[] = 
	{
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 
	};

	static const unsigned char def_theme_minimize_btn_inactive_up[] = 
	{
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	};

	static const unsigned char def_theme_minimize_btn_inactive_dn[] = 
	{
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	};
	
	static const unsigned char* def_theme_minimize_btn[2][2] = { {def_theme_minimize_btn_inactive_up, def_theme_minimize_btn_inactive_dn}, {def_theme_minimize_btn_active_up, def_theme_minimize_btn_active_dn}};
	

	static const unsigned char def_theme_normsize_btn_active_up[] = 
	{
		0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 
	};

	static const unsigned char def_theme_normsize_btn_active_dn[] = 
	{
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 
	};

	static const unsigned char def_theme_normsize_btn_inactive_up[] = 
	{
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	};

	static const unsigned char def_theme_normsize_btn_inactive_dn[] = 
	{
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	};
	
	static const unsigned char* def_theme_normsize_btn[2][2] = { {def_theme_normsize_btn_inactive_up, def_theme_normsize_btn_inactive_dn}, {def_theme_normsize_btn_active_up, def_theme_normsize_btn_active_dn}};


	static const unsigned char def_theme_maximize_btn_active_up[] = 
	{
		0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x7F, 0x2B, 
		0x2B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x2B, 
		0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 
	};

	static const unsigned char def_theme_maximize_btn_active_dn[] = 
	{
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x7F, 0xF4, 
		0xF4, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xF4, 
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 
	};

	static const unsigned char def_theme_maximize_btn_inactive_up[] = 
	{
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	};

	static const unsigned char def_theme_maximize_btn_inactive_dn[] = 
	{
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F, 0x55, 
		0x55, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x55, 
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	};
	
	static const unsigned char* def_theme_maximize_btn[2][2] = { {def_theme_maximize_btn_inactive_up, def_theme_maximize_btn_inactive_dn}, {def_theme_maximize_btn_active_up, def_theme_maximize_btn_active_dn}};


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/


//! Generate a desktop pattern bitmap
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
Bitmap* Theme_CreateDefaultDesktopPattern(void);

//! Generate a control template for the window Close button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateClose(void);

//! Generate a control template for the window Minimize button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateMinimize(void);

//! Generate a control template for the window Normal (window) size button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateNormSize(void);

//! Generate a control template for the window Maximize button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateMaximize(void);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/




//! Generate a desktop pattern bitmap
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
Bitmap* Theme_CreateDefaultDesktopPattern(void)
{
	Bitmap*		the_bitmap;
	signed int	width = 16;
	signed int	height = 16;
	
	if ( (the_bitmap = Bitmap_New(width, height, NULL) ) == NULL)
	{
		LOG_ERR(("%s %d: could not create new Bitmap", __func__ , __LINE__));
		return NULL;
	}

	// LOGIC:
	//   until we have a file system in f68/MCP, need to load from ROM (memory)
	//   perhaps even after we have file system, in case user borks their system resources
	//   image is saved from GraphicConverter in "byte array header file" (.h) format, then adjusted to desired CLUT index manually

	memcpy(the_bitmap->addr_, def_theme_desktop_pattern, width * height);
	
	return the_bitmap;
	
}


//! Generate a control template for the window Close button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateClose(void)
{
	ControlTemplate*	the_template;
	signed int			width = 12;		// TODO: replace with theme-property so size can be whatever designer says
	signed int			height = 12;	// TODO: replace with theme-property so size can be whatever designer says
	int					is_active;
	int					is_pushed;

	// LOGIC:
	//   until we have a file system in f68/MCP, need to load from ROM (memory)
	//   perhaps even after we have file system, in case user borks their system resources
	//   image is saved from GraphicConverter in "byte array header file" (.h) format, then adjusted to desired CLUT index manually
	
	if ( (the_template = ControlTemplate_New()) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new ControlTemplate", __func__ , __LINE__));
		return NULL;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_template	%p	size	%i", __func__ , __LINE__, the_template, sizeof(ControlTemplate)));
	
	for (is_active = 0; is_active < 2; is_active++)
	{
		for (is_pushed = 0; is_pushed < 2; is_pushed++)
		{
			Bitmap*		the_bitmap;

			if ( (the_bitmap = Bitmap_New(width, height, NULL) ) == NULL)
			{
				LOG_ERR(("%s %d: could not create new Bitmap", __func__ , __LINE__));
				return NULL;
			}

			memcpy(the_bitmap->addr_, def_theme_close_btn[is_active][is_pushed], width * height);
			the_template->image_[is_active][is_pushed] = the_bitmap;
		}
	}	

	the_template->type_ = CLOSE_WIDGET;
	the_template->h_align_ = H_ALIGN_LEFT;
	the_template->v_align_ = V_ALIGN_TOP;
	the_template->x_offset_ = 4;
	the_template->y_offset_ = 4;
	the_template->width_ = width;
	the_template->height_ = height;
	the_template->min_ = 0;
	the_template->max_ = 1;
	the_template->caption_ = NULL;
	
	return the_template;
}


//! Generate a control template for the window Minimize button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateMinimize(void)
{
	ControlTemplate*	the_template;
	Bitmap*				the_bitmap;
	signed int			width = 12;
	signed int			height = 12;
	int					is_active;
	int					is_pushed;

	// LOGIC:
	//   until we have a file system in f68/MCP, need to load from ROM (memory)
	//   perhaps even after we have file system, in case user borks their system resources
	//   image is saved from GraphicConverter in "byte array header file" (.h) format, then adjusted to desired CLUT index manually
	//   we also don't have inactive and pushed states, so just doing regular state for now.
	
	if ( (the_template = ControlTemplate_New()) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new ControlTemplate", __func__ , __LINE__));
		return NULL;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_template	%p	size	%i", __func__ , __LINE__, the_template, sizeof(ControlTemplate)));
	
	for (is_active = 0; is_active < 2; is_active++)
	{
		for (is_pushed = 0; is_pushed < 2; is_pushed++)
		{
			Bitmap*		the_bitmap;

			if ( (the_bitmap = Bitmap_New(width, height, NULL) ) == NULL)
			{
				LOG_ERR(("%s %d: could not create new Bitmap", __func__ , __LINE__));
				return NULL;
			}

			memcpy(the_bitmap->addr_, def_theme_minimize_btn[is_active][is_pushed], width * height);
			the_template->image_[is_active][is_pushed] = the_bitmap;
		}
	}

	the_template->type_ = SIZE_MINIMIZE;
	the_template->h_align_ = H_ALIGN_RIGHT;
	the_template->v_align_ = V_ALIGN_TOP;
	the_template->x_offset_ = 46;
	the_template->y_offset_ = 4;
	the_template->width_ = width;
	the_template->height_ = height;
	the_template->min_ = 0;
	the_template->max_ = 1;
	the_template->caption_ = NULL;
	
	return the_template;
}


//! Generate a control template for the window NormSize button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateNormSize(void)
{
	ControlTemplate*	the_template;
	Bitmap*				the_bitmap;
	signed int			width = 12;
	signed int			height = 12;
	int					is_active;
	int					is_pushed;

	
	// LOGIC:
	//   until we have a file system in f68/MCP, need to load from ROM (memory)
	//   perhaps even after we have file system, in case user borks their system resources
	//   image is saved from GraphicConverter in "byte array header file" (.h) format, then adjusted to desired CLUT index manually
	//   we also don't have inactive and pushed states, so just doing regular state for now.
	
	if ( (the_template = ControlTemplate_New()) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new ControlTemplate", __func__ , __LINE__));
		return NULL;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_template	%p	size	%i", __func__ , __LINE__, the_template, sizeof(ControlTemplate)));
	
	for (is_active = 0; is_active < 2; is_active++)
	{
		for (is_pushed = 0; is_pushed < 2; is_pushed++)
		{
			Bitmap*		the_bitmap;

			if ( (the_bitmap = Bitmap_New(width, height, NULL) ) == NULL)
			{
				LOG_ERR(("%s %d: could not create new Bitmap", __func__ , __LINE__));
				return NULL;
			}

			memcpy(the_bitmap->addr_, def_theme_normsize_btn[is_active][is_pushed], width * height);
			the_template->image_[is_active][is_pushed] = the_bitmap;
		}
	}

	the_template->type_ = SIZE_NORMAL;
	the_template->h_align_ = H_ALIGN_RIGHT;
	the_template->v_align_ = V_ALIGN_TOP;
	the_template->x_offset_ = 31;
	the_template->y_offset_ = 4;
	the_template->width_ = width;
	the_template->height_ = height;
	the_template->min_ = 0;
	the_template->max_ = 1;
	the_template->caption_ = NULL;
	
	return the_template;
}


//! Generate a control template for the window Maximize button
//! This is guaranteed to be available to the system, even if user destroys their system resources on disk
ControlTemplate* Theme_CreateDefaultControlTemplateMaximize(void)
{
	ControlTemplate*	the_template;
	Bitmap*				the_bitmap;
	signed int			width = 12;
	signed int			height = 12;
	int					is_active;
	int					is_pushed;

	// LOGIC:
	//   until we have a file system in f68/MCP, need to load from ROM (memory)
	//   perhaps even after we have file system, in case user borks their system resources
	//   image is saved from GraphicConverter in "byte array header file" (.h) format, then adjusted to desired CLUT index manually
	//   we also don't have inactive and pushed states, so just doing regular state for now.
	
	if ( (the_template = ControlTemplate_New()) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new ControlTemplate", __func__ , __LINE__));
		return NULL;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_template	%p	size	%i", __func__ , __LINE__, the_template, sizeof(ControlTemplate)));
	
	for (is_active = 0; is_active < 2; is_active++)
	{
		for (is_pushed = 0; is_pushed < 2; is_pushed++)
		{
			Bitmap*		the_bitmap;

			if ( (the_bitmap = Bitmap_New(width, height, NULL) ) == NULL)
			{
				LOG_ERR(("%s %d: could not create new Bitmap", __func__ , __LINE__));
				return NULL;
			}

			memcpy(the_bitmap->addr_, def_theme_maximize_btn[is_active][is_pushed], width * height);
			the_template->image_[is_active][is_pushed] = the_bitmap;
		}
	}

	the_template->type_ = SIZE_MAXIMIZE;
	the_template->h_align_ = H_ALIGN_RIGHT;
	the_template->v_align_ = V_ALIGN_TOP;
	the_template->x_offset_ = 16;
	the_template->y_offset_ = 4;
	the_template->width_ = width;
	the_template->height_ = height;
	the_template->min_ = 0;
	the_template->max_ = 1;
	the_template->caption_ = NULL;
	
	return the_template;
}


// this is kind of dumb. you might as well just not do this, with all the params passed. 
// better to just make a Bitmap_NewFromMemory() function that wraps around Bitmap_New, and copies in. but even then, you aren't saving much.
// maybe need to have a routine that simply loads and parses an entire control from disk? .ini would have all the params here, plus paths to 4 bitmaps to load from disk. 
// //! Generate a control template based on the passed parameters
// ControlTemplate* Theme_CreateControlTemplate(control_type the_type, signed int width, signed int height, signed int x_offset, signed int y_offset, h_align_type the_h_align, v_align_type the_v_align, int16_t min, int16_t max, char* the_caption, const unsigned char* img_inact_up, const unsigned char* img_inact_dn, const unsigned char* img_act_up, const unsigned char* img_act_dn)
// {
// 	ControlTemplate*	the_template;
// // 	signed int			width = 12;
// // 	signed int			height = 12;
// 	int					is_active;
// 	int					is_pushed;
// 
// 	if ( (the_template = ControlTemplate_New()) == NULL)
// 	{
// 		LOG_ERR(("%s %d: could not allocate memory to create new ControlTemplate", __func__ , __LINE__));
// 		return NULL;
// 	}
// 	LOG_ALLOC(("%s %d:	__ALLOC__	the_template	%p	size	%i", __func__ , __LINE__, the_template, sizeof(ControlTemplate)));
// 	
// 	for (is_active = 0; is_active < 2; is_active++)
// 	{
// 		for (is_pushed = 0; is_pushed < 2; is_pushed++)
// 		{
// 			Bitmap*		the_bitmap;
// 
// 			if ( (the_bitmap = Bitmap_New(width, height, NULL) ) == NULL)
// 			{
// 				LOG_ERR(("%s %d: could not create new Bitmap", __func__ , __LINE__));
// 				return NULL;
// 			}
// 
// 			memcpy(the_bitmap->addr_, def_theme_maximize_btn[is_active][is_pushed], width * height);
// 			the_template->image_[is_active][is_pushed] = the_bitmap;
// 		}
// 	}
// 
// 	the_template->type_ = the_type;
// 	the_template->h_align_ = the_h_align;
// 	the_template->v_align_ = the_v_align;
// 	the_template->x_offset_ = x_offset;
// 	the_template->y_offset_ = y_offset;
// 	the_template->width_ = width;
// 	the_template->height_ = height;
// 	the_template->min_ = min;
// 	the_template->max_ = max;
// 	the_template->caption_ = the_caption;
// 	
// 	return the_template;
// }







// **** Debug functions *****

void Theme_Print(Theme* the_theme)
{
	DEBUG_OUT(("Theme print out:"));
	DEBUG_OUT(("  address: %p", 			the_theme));
	DEBUG_OUT(("  icon_font_: %p",			the_theme->icon_font_));
	DEBUG_OUT(("  control_font_: %p",		the_theme->control_font_));
	DEBUG_OUT(("  clut_: %p",				the_theme->clut_));
	DEBUG_OUT(("  outline_size_: %u", 		the_theme->outline_size_));
	DEBUG_OUT(("  outline_color_: %u", 		the_theme->outline_color_));
	DEBUG_OUT(("  titlebar_height_: %u", 	the_theme->titlebar_height_));
	DEBUG_OUT(("  titlebar_y_: %i", 		the_theme->titlebar_y_));
	DEBUG_OUT(("  titlebar_color_: %u",		the_theme->titlebar_color_));
	DEBUG_OUT(("  iconbar_height_: %u",		the_theme->iconbar_height_));
	DEBUG_OUT(("  iconbar_y_: %i", 			the_theme->iconbar_y_));
	DEBUG_OUT(("  iconbar_color_: %u", 		the_theme->iconbar_color_));
	DEBUG_OUT(("  contentarea_y_: %i", 		the_theme->contentarea_y_));
	DEBUG_OUT(("  contentarea_color_: %u", 	the_theme->contentarea_color_));
	DEBUG_OUT(("  desktop_color_: %u",	 	the_theme->desktop_color_));
	DEBUG_OUT(("  desktop_pattern_: %p",	the_theme->desktop_pattern_));
	DEBUG_OUT(("  control_t_close_: %p",	the_theme->control_t_close_));
	DEBUG_OUT(("  control_t_minimize_: %p",	the_theme->control_t_minimize_));
	DEBUG_OUT(("  control_t_norm_size_: %p",	the_theme->control_t_norm_size_));
	DEBUG_OUT(("  control_t_maximize_: %p",	the_theme->control_t_maximize_));
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate a Theme object
Theme* Theme_New(void)
{
	Theme*			the_theme;
	
	// LOGIC: 
	//   For now, we don't have disk ability, so not point in passing a file path
	//   For now, this will only create a default theme
	//   In future, probably want to pass a file path char* to this, and have it try to load from there, with fallback to sys default.
	
	if ( (the_theme = (Theme*)f_calloc(1, sizeof(Theme), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new Theme", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_theme	%p	size	%i", __func__ , __LINE__, the_theme, sizeof(Theme)));
	
	return the_theme;
	
error:
	if (the_theme)					Theme_Destroy(&the_theme);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Theme_Destroy(Theme** the_theme)
{
	if (*the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	if ((*the_theme)->control_t_close_)
	{
// 		Control_DestroyTemplate((*the_theme)->control_t_close_);
		(*the_theme)->control_t_close_ = NULL;
	}
	
	LOG_ALLOC(("%s %d:	__FREE__	*the_theme	%p	size	%i", __func__ , __LINE__, *the_theme, sizeof(Theme)));
	f_free(*the_theme, MEM_STANDARD);
	*the_theme = NULL;
	
	return true;
}


//! create default Theme.
//! used in cases where a custom theme is not specified or is not available
Theme* Theme_CreateDefaultTheme(void)
{
	Theme*		the_theme;
	
	if ( (the_theme = Theme_New()) == NULL)
	{
		LOG_ERR(("%s %d: could not create a new Theme", __func__ , __LINE__));
		goto error;
	}
	
	// LOGIC:
	//   this is to create a default style involving resources guaranteed to be in "ROM" (ie, no dependency on disk)
	//   fonts will come from the system, CLUT will be the system default, etc. 
	
	the_theme->icon_font_ = Sys_GetAppFont(global_system);
	the_theme->control_font_ = Sys_GetSystemFont(global_system);
	
	if (the_theme->icon_font_ == NULL || the_theme->control_font_ == NULL)
	{
		LOG_ERR(("%s %d: Could not acquire the system and/or app font", __func__ , __LINE__));
		goto error;
	}
	
	the_theme->outline_size_ = WIN_DEFAULT_OUTLINE_SIZE;
	the_theme->outline_color_ = WIN_DEFAULT_OUTLINE_COLOR;
	
	the_theme->titlebar_height_ = WIN_DEFAULT_TITLEBAR_HEIGHT;
	the_theme->titlebar_y_ = WIN_DEFAULT_TITLEBAR_Y;
	the_theme->titlebar_color_ = WIN_DEFAULT_TITLEBAR_COLOR;
	
	the_theme->iconbar_height_ = WIN_DEFAULT_ICONBAR_HEIGHT;
	the_theme->iconbar_y_ = WIN_DEFAULT_ICONBAR_Y;
	the_theme->iconbar_color_ = WIN_DEFAULT_ICONBAR_COLOR;
	
	the_theme->contentarea_y_ = WIN_DEFAULT_CONTENTAREA_Y;
	the_theme->contentarea_color_ = WIN_DEFAULT_CONTENTAREA_COLOR;
	
	the_theme->desktop_color_ = WIN_DEFAULT_DESKTOP_COLOR;
	the_theme->desktop_pattern_ = Theme_CreateDefaultDesktopPattern();
	
	// get control templates
	the_theme->control_t_close_ = Theme_CreateDefaultControlTemplateClose();
	the_theme->control_t_minimize_ = Theme_CreateDefaultControlTemplateMinimize();
	the_theme->control_t_norm_size_ = Theme_CreateDefaultControlTemplateNormSize();
	the_theme->control_t_maximize_ = Theme_CreateDefaultControlTemplateMaximize();
	
	// debug
	Theme_Print(the_theme);
	
	return the_theme;
	
error:
	if (the_theme)		f_free(the_theme, MEM_STANDARD);
	return NULL;
}





// **** Set xxx functions *****





// **** Get xxx functions *****

ControlTemplate* Theme_GetCloseControlTemplate(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}

	return the_theme->control_t_close_;
}


ControlTemplate* Theme_GetMinimizeControlTemplate(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}

	return the_theme->control_t_minimize_;
}


ControlTemplate* Theme_GetNormSizeControlTemplate(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}

	return the_theme->control_t_norm_size_;
}


ControlTemplate* Theme_GetMaximizeControlTemplate(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}

	return the_theme->control_t_maximize_;
}


Bitmap* Theme_GetDesktopPattern(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return NULL;
	}

	return the_theme->desktop_pattern_;
}


ColorIdx Theme_GetTitlebarColor(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return 0;
	}

	return the_theme->titlebar_color_;
}


ColorIdx Theme_GetIconbarColor(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return 0;
	}

	return the_theme->iconbar_color_;
}


ColorIdx Theme_GetOutlineColor(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return 0;
	}

	return the_theme->outline_color_;
}


ColorIdx Theme_GetContentAreaColor(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return 0;
	}

	return the_theme->contentarea_color_;
}


ColorIdx Theme_GetDesktopColor(Theme* the_theme)
{
	if (the_theme == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return 0;
	}

	return the_theme->desktop_color_;
}





// **** xxx functions *****




