/*
 * window_demo.c
 *
 *  Created on: Mar 19, 2022
 *      Author: micahbly
 *
 *  Demonstrates many of the features of the graphics library
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/


// project includes


// C includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
#include <mb/bitmap.h>
#include <mb/font.h>
#include <mb/window.h>
#include <mb/lib_sys.h>


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



void RunDemo(void)
{
	Window*			the_window;
	NewWindowData	the_win_setup;
	Bitmap*			the_bitmap;
	unsigned int	x1;
	unsigned int	y;
	char			temp_buff[100];
	char*			wintitle = temp_buff;
	unsigned int	width = 500;
	unsigned int	height = 300;

// 	ShowDescription("Welcome to the A2560 Window Demo!");	
// 	WaitForUser();
	
	DEBUG_OUT(("%s %d: Setting graphics mode...", __func__, __LINE__));

	Sys_SetModeGraphics(global_system);
	
	// try to allocate a bitmap in VRAM for the window
	if ( (the_bitmap = Bitmap_New(width, height, Sys_GetAppFont(global_system))) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create bitmap", __func__, __LINE__));
		return;
	}

	sprintf(wintitle, "My Window");
	
	the_win_setup.x_ = 101;
	the_win_setup.y_ = 101;
	the_win_setup.width_ = width;
	the_win_setup.height_ = height;
	the_win_setup.min_width_ = 100;
	the_win_setup.min_height_ = 100;
	the_win_setup.max_width_ = 1024;
	the_win_setup.max_height_ = 768;

	the_win_setup.user_data_ = 0L;
	the_win_setup.title_ = wintitle;
	the_win_setup.type_ = WIN_STANDARD;
	the_win_setup.bitmap_ = the_bitmap;
	the_win_setup.buffer_bitmap_ = NULL;
	the_win_setup.is_backdrop_ = false;
	the_win_setup.can_resize_ = true;

	DEBUG_OUT(("%s %d: x=%i, y=%i, width=%i, title='%s'", __func__, __LINE__, the_win_setup.x_, the_win_setup.y_, the_win_setup.width_, the_win_setup.title_));
	
	if ( (the_window = Window_New(&the_win_setup)) == NULL)
	{
		DEBUG_OUT(("%s %d: Couldn't instantiate a window", __func__, __LINE__));
		return;
	}

	Window_Print(the_window);

	Bitmap_DrawCircle(global_system->screen_[ID_CHANNEL_B]->bitmap_, 25, 25, 12, 0x88);
	Bitmap_DrawCircle(global_system->screen_[ID_CHANNEL_B]->bitmap_, 25, 25, 15, 0xcc);
	Bitmap_DrawCircle(global_system->screen_[ID_CHANNEL_B]->bitmap_, 25, 25, 20, 0xff);
	Bitmap_DrawCircle(global_system->screen_[ID_CHANNEL_B]->bitmap_, 50, 200, 20, 0xff);
	Bitmap_DrawBoxCoords(global_system->screen_[ID_CHANNEL_B]->bitmap_, 25, 25, 100, 100, 0xff);
	//Demo_Font_DrawString(global_system->screen_[ID_CHANNEL_B]->bitmap_, 20);
	x1 = 25;
	y = 10;
// 		Demo_Font_ShowChars(the_bitmap, x1, y);
// 		Demo_Font_DrawString(the_bitmap, y);
// 		Demo_Font_DrawString(the_bitmap, y + 50);
	//Demo_Font_DrawStringInBox1(the_bitmap);
	
	// pretend render the window
	Window_Render(the_window);

	
	// blit to screen
	Bitmap_Blit(the_window->bitmap_, 0, 0, global_system->screen_[ID_CHANNEL_B]->bitmap_, the_window->x_, the_window->y_, width, height);


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
	
 	RunDemo();
	
	exit(0);
}