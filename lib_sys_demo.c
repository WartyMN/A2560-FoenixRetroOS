/*
 * lib_sys_demo.c
 *
 *  Created on: Mar 22, 2022
 *      Author: micahbly
 *
 *  Demonstrates some of the features of the system library
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/


// project includes
#include "lib_sys.h"


// C includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>	// just for initiating rand()


// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
#include <mb/bitmap.h>
#include <mb/font.h>
#include <mb/window.h>


/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern System*			global_system;


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

// pre-configure screen data. TODO: use sys info to populate based on what's in hardware.
void InitScreen(void);

// have user hit a key, then clear screens
void WaitForUser(void);

// Draw fancy box on the B screen and display demo description
void ShowDescription(char* the_message);

// run all the demos
void RunDemo(void);

// various demos
void Demo_Font_ShowChars(Bitmap* the_bitmap, unsigned int x1, unsigned int y);
void Demo_Font_DrawString(Bitmap* the_bitmap, unsigned int y);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


// have user hit a key, then clear screens
void WaitForUser(void)
{
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"Press any key to continue", FG_COLOR_YELLOW, 0);
	
	getchar();
	
// 	Bitmap_FillMemory(global_system->screen_[ID_CHANNEL_B], 0xbb);
// 	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], ' ');
// 	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 0);
}

// Draw fancy box on the B screen and display demo description
void ShowDescription(char* the_message)
{
	signed int	x1 = 0;
	signed int	x2 = global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1;
	signed int	y1 = 0;
	signed int	y2 = 5;

	// draw box and fill contents in prep for next demo description
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, FG_COLOR_BLUE, 0);
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, ' ', FG_COLOR_WHITE, 0);
	
	// wrap text into the message box, leaving one row at the bottom for "press any key"
	Text_DrawStringInBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, the_message, FG_COLOR_WHITE, 0, NULL);
}


void Demo_Font_ShowChars(Bitmap* the_bitmap, unsigned int x1, unsigned int y)
{
	Font*			the_font;
	unsigned int	x2;
	uint8_t			i;
	unsigned int	pix_written;
	
	
	ShowDescription("Font_DrawChar -> draw a single character at the current pen position");	

	the_font = the_bitmap->font_;
	
	x2 = 600;
	i = the_font->firstChar;
	pix_written = x1;
	
	Bitmap_SetXY(the_bitmap, x1, y);
	Bitmap_SetColor(the_bitmap, 0xaa);
	
	for (; i < the_font->lastChar; i++)
	{
		pix_written += Font_DrawChar(the_bitmap, i, the_font);
		
		if (pix_written >= x2)
		{
			pix_written = x1;
			y = y + the_font->fRectHeight + the_font->leading;
			Bitmap_SetXY(the_bitmap, x1, y);
		}
	}
	
 	WaitForUser();
}


void Demo_Font_DrawString(Bitmap* the_bitmap, unsigned int y)
{
	Font*			the_font;
	unsigned int	x;
	uint8_t			row_height;
	char*			string1 = (char*)"March 17, 2022: hiya world!";
	char*			string2 = (char*)"This string is too long to fit";
	
	ShowDescription("Font_DrawString -> draw as much of a string as will fit at the current pen position");	

	the_font = the_bitmap->font_;
	row_height = the_font->leading + the_font->fRectHeight;
	
	// draw whereever the pen happens to be, in white
	Bitmap_SetColor(the_bitmap, 0xff);
	
	if (Font_DrawString(the_bitmap, string1, GEN_NO_STRLEN_CAP) == false)
	{
	}

	// draw at upper right, in too narrow a space, in light blue
	x = 450;
	Bitmap_SetColor(the_bitmap, 0x88);
	
	// draw copy of string down right edge of screen to test fit
	for (; y < the_bitmap->height_; y += row_height)
	{
		Bitmap_SetXY(the_bitmap, x, y);

		if (Font_DrawString(the_bitmap, string2, GEN_NO_STRLEN_CAP) == false)
		{
		}
		
		x += 20;
	}
	
 	WaitForUser();
}





void RunDemo(void)
{
	Window*				the_window[5];
	NewWinTemplate*		the_win_template;
	int					max_width = 640;
	int					max_height = 460;
	static char*		the_win_title = "A New Window";
	int					win_num;
	int					random_num;
	
	srand(sys_time_jiffies());
	//srand(time(NULL));   // Initialization, should only be called once.
	
	DEBUG_OUT(("%s %d: Setting graphics mode...", __func__, __LINE__));

	Sys_SetModeGraphics(global_system);
	
	if ( (the_win_template = Window_GetNewWinTemplate(the_win_title)) == NULL)
	{
		LOG_ERR(("%s %d: Could not get a new window template", __func__ , __LINE__));
		return;
	}	
	// note: all the default values are fine for us in this case.
	
	for (win_num = 0; win_num < 4; win_num++)
	{
		the_win_template->x_ = (rand() * (max_width / 2)) / RAND_MAX;
		the_win_template->y_ = (rand() * (max_height / 2)) / RAND_MAX;
		the_win_template->width_ = (rand() * (max_width)) / RAND_MAX;
		the_win_template->height_ = (rand() * (max_height)) / RAND_MAX;

		if ( (the_window[win_num] = Window_New(the_win_template)) == NULL)
		{
			DEBUG_OUT(("%s %d: Couldn't instantiate a window", __func__, __LINE__));
			return;
		}

		// declare the window to be visible
		Window_SetVisible(the_window[win_num], true);
	}
	
	// one more guaranteed smallest window for testing
	the_win_template->x_ = 10;
	the_win_template->y_ = 10;
	the_win_template->width_ = 1;
	the_win_template->height_ = 1;

	if ( (the_window[4] = Window_New(the_win_template)) == NULL)
	{
		DEBUG_OUT(("%s %d: Couldn't instantiate a window", __func__, __LINE__));
		return;
	}

	// declare the window to be visible
	Window_SetVisible(the_window[4], true);
	
	
	
	// temporary until event handler is written: tell system to render the screen and all windows
	Sys_Render(global_system);

	// delay a bit before switching
	General_DelaySeconds(3);
	
	// switch to green theme!
	Theme*	the_theme;
	
	if ( (the_theme = Theme_CreateGreenTheme() ) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create custom green theme", __func__, __LINE__));
		return;
	}
	
	Theme_Activate(the_theme);

	for (win_num = 0; win_num < 5; win_num++)
	{
		Window_ClearContent(the_window[win_num]);
	}

	Sys_Render(global_system);


// 	Window_Destroy(&the_window);

	// call something to shut the system down and free memory	
	
// 	Font_Destroy(&global_system_font);
// 	Font_Destroy(&the_icon_font);
	
	
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/




int main(int argc, char* argv[])
{
	if (Sys_InitSystem() == false)
	{
		DEBUG_OUT(("%s %d: Couldn't initialize the system", __func__, __LINE__));
		exit(0);
	}
	
	// NOTE: at this point, the_system should equal global_system, as that is set by Sys_InitSystem().
	
// 	Sys_SetModeGraphics(global_system);;
//  Sys_SetModeText(global_system, true);
	
 	RunDemo();

// 	Sys_SetModeText(global_system, false);
	
	return 0;
}