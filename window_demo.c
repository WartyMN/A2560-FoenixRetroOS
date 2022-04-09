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
#include <stdbool.h>
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
	NewWinTemplate*		the_win_template;
	unsigned int	width = 500;
	unsigned int	height = 300;
	static char*	the_win_title = "My New Window";
	
	DEBUG_OUT(("%s %d: Setting graphics mode...", __func__, __LINE__));

	Sys_SetModeGraphics(global_system);
	
	if ( (the_win_template = Window_GetNewWinTemplate(the_win_title)) == NULL)
	{
		LOG_ERR(("%s %d: Could not get a new window template", __func__ , __LINE__));
		return;
	}	
	// note: all the default values are fine for us in this case.
	
	DEBUG_OUT(("%s %d: x=%i, y=%i, width=%i, title='%s'", __func__, __LINE__, the_win_template->x_, the_win_template->y_, the_win_template->width_, the_win_template->title_));
	
	if ( (the_window = Window_New(the_win_template)) == NULL)
	{
		DEBUG_OUT(("%s %d: Couldn't instantiate a window", __func__, __LINE__));
		return;
	}

	// say hi
	Window_SetPenXY(the_window, 5, 5);

	if (Window_DrawString(the_window, (char*)"Hello, World", FONT_NO_STRLEN_CAP) == false)
	{
		// oh, no! you should handle this.
	}
	

	// draw some stuff in the window we created
	{
		// draw some color blocks
		int i;
		int x = 1;
		int y = the_window->content_rect_.MinY + 50;
		int height = 16;
	
		// chromatic
		for (i=1; i<(256-41); i++)
		{
			Bitmap_FillBox(the_window->bitmap_, x, y, 2, height, i);
			x += 2;
		
			if (i % 36 == 0)
			{
				x = 1;
				y += height;
			}
		}

		// reds > greens > blues > grays
		x = 1;
		y += height;

		for (; i<256; i++)
		{
			Bitmap_FillBox(the_window->bitmap_, x, y, 2, height, i);
			x += 2;
		}
	}

	// declare the window to be visible
	Window_SetVisible(the_window, true);

	Window_SetPenXY(the_window, 2, 25);

	if (Window_DrawString(the_window, (char*)"This is the default theme. Let's try a custom one...", FONT_NO_STRLEN_CAP) == false)
	{
		// oh, no! you should handle this.
	}
	
	// temporary until event handler is written: tell system to render the screen and all windows
	Sys_Render(global_system);
	
	// get ready to switch theme
	DelaySeconds(2);

	// switch to green theme!
	Theme*	the_theme;
	
	if ( (the_theme = Theme_CreateGreenTheme() ) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create custom green theme", __func__, __LINE__));
		return;
	}
	
	Theme_Activate(the_theme);

	Window_ClearContent(the_window);

	Window_SetPenXY(the_window, 5, 5);

	char	the_buff[2048];
	char*	wrapBuff = the_buff;
	
	if (Window_DrawStringInBox(the_window, the_window->inner_width_-10, the_window->inner_height_-10,(char*)"A theme designer can change the background, color palette, fonts, graphics for controls, position of controls and content, and more!\nThe simplest form of theme is nothing more than a different color palette. This theme has a different palette, different control graphics (size is also different); the position of the controls in the title is different, and of course the font is different. \n You could place the title bar at the bottom of the window if you want.", FONT_NO_STRLEN_CAP, &wrapBuff, NULL) != NULL)
	{
		// oh, no! you should handle this.
	}

	// draw some stuff in the window we created
	{
		// draw some color blocks
		int i;
		int x = 1;
		int y = the_window->content_rect_.MinY + 50;
		int height = 16;
	
		// chromatic
		for (i=1; i<(256-41); i++)
		{
			Bitmap_FillBox(the_window->bitmap_, x, y, 2, height, i);
			x += 2;
		
			if (i % 36 == 0)
			{
				x = 1;
				y += height;
			}
		}

		// reds > greens > blues > grays
		x = 1;
		y += height;

		for (; i<256; i++)
		{
			Bitmap_FillBox(the_window->bitmap_, x, y, 2, height, i);
			x += 2;
		}
	}

	Window_SetPenXY(the_window, 5, 200);

	if (Window_DrawString(the_window, (char*)"In a second, the window will redraw in a slow and ugly fashion. Testing the control visuals...", FONT_NO_STRLEN_CAP) == false)
	{
		// oh, no! you should handle this.
	}

	Sys_Render(global_system);


	// inactivate some buttons to test things out
	DelaySeconds(1);
	
	// deactivate some of the controls
	Control*	root_control;
	Control*	this_control;
	
	root_control = Window_GetRootControl(the_window);
	this_control = root_control;
	
	while (this_control)
	{
		Control_SetActive(this_control, CONTROL_INACTIVE);
		Control_SetPressed(this_control, CONTROL_UP);
		
		this_control = Control_GetNextControl(this_control);
		Sys_Render(global_system);
		
		Delay(30);
	}
	

	// reactive some buttons and pretend to push them
	DelaySeconds(2);
	
	this_control = root_control;
	
	while (this_control)
	{
		Control_SetActive(this_control, CONTROL_ACTIVE);
		Control_SetPressed(this_control, CONTROL_DOWN);
		
		this_control = Control_GetNextControl(this_control);
		Sys_Render(global_system);
		
		Delay(30);
	}


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

	return 0;
}