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
#include <mb/theme.h>

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
void SwitchThemes(Window* the_window);
void ToggleButtonStates(Window* the_window);
void AddControls(Window* the_window);

// handler for the hello world window
void HelloWindowEventHandler(EventRecord* the_event);



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
	int16_t		x1 = 0;
	int16_t		x2 = global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1;
	int16_t		y1 = 0;
	int16_t		y2 = 5;

	// draw box and fill contents in prep for next demo description
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, FG_COLOR_BLUE, 0);
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, ' ', FG_COLOR_WHITE, 0);
	
	// wrap text into the message box, leaving one row at the bottom for "press any key"
	Text_DrawStringInBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, the_message, FG_COLOR_WHITE, 0, NULL);
}


// switch to green theme!
void SwitchThemes(Window* the_window)
{
	Theme*	the_theme;

// 	Window_SetPenXY(the_window, 2, 25);
// 
// 	if (Window_DrawString(the_window, (char*)"This is the default theme. Let's try a custom one...", GEN_NO_STRLEN_CAP) == false)
// 	{
// 		// oh, no! you should handle this.
// 	}
	
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
	
	if (Window_DrawStringInBox(the_window, the_window->inner_width_-10, the_window->inner_height_-10,(char*)"A theme designer can change the background, color palette, fonts, graphics for controls, position of controls and content, and more!\nThe simplest form of theme is nothing more than a different color palette. This theme has a different palette, different control graphics (size is also different); the position of the controls in the title is different, and of course the font is different. \n You could place the title bar at the bottom of the window if you want.", GEN_NO_STRLEN_CAP, &wrapBuff, NULL) != NULL)
	{
		// oh, no! you should handle this.
	}

	// draw some stuff in the window we created
	{
		// draw some color blocks
		int16_t i;
		int16_t x = 1;
		int16_t y = the_window->content_rect_.MinY + 50;
		int16_t height = 16;
	
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

// 	if (Window_DrawString(the_window, (char*)"In a second, the window will redraw in a slow and ugly fashion. Testing the control visuals...", GEN_NO_STRLEN_CAP) == false)
// 	{
// 		// oh, no! you should handle this.
// 	}

	Sys_Render(global_system);
}


void ToggleButtonStates(Window* the_window)
{	
	Control*	root_control;
	Control*	this_control;
	int8_t		is_active;
	int8_t		is_pushed;
	
	// toggle all controls through their states to test things out
	General_DelaySeconds(1);
	
	for (is_active = 0; is_active < 2; is_active++)
	{
		for (is_pushed = 0; is_pushed < 2; is_pushed++)
		{

			root_control = Window_GetRootControl(the_window);
			this_control = root_control;
	
			while (this_control)
			{
				Control_SetActive(this_control, is_active);
				Control_SetPressed(this_control, is_pushed);
		
				this_control = Control_GetNextControl(this_control);
			}
	
			Sys_Render(global_system);
			
			General_DelaySeconds(2);
		}
	}
}


void AddControls(Window* the_window)
{
	Theme*				the_theme;
	Control*			button_1;
	Control*			button_2;
	//Control*			button_3;
	Control*			button_4;
	int16_t				x_offset;
	int16_t				y_offset;
	int16_t				width;
	int16_t				height;
	static char*		caption_1 = "Active";
	static char*		caption_2 = "Active - pushed";
	static char*		caption_3 = "Inactive";
	static char*		caption_4 = "4th state - selected button?";
	uint16_t			the_id;
	uint16_t			group_id;
	
	if ( (the_theme = Sys_GetTheme(global_system)) == NULL)
	{
		LOG_ERR(("%s %d: failed to get the current system theme!", __func__ , __LINE__));
		return;
	}
	
	// create 4 buttons, with varying width, and set each to a different state to check visuals

	// all buttons need to use same height, which is height-of-button-in-the-Theme
	height = the_theme->flex_width_backdrops_[TEXT_BUTTON].height_; // make a function for getting this from theme. 
	group_id = 0; // will not be grouping this button with any other buttons
	
	width = 80;
	the_id = 1000; // arbitrary, for use of programmer. manage them though, and keep them unique within any one window
	x_offset = 100;
	y_offset = 100;
	
	// for user docs:
	// first way of adding a control is to call Window_AddNewControl(), and supply all the information at once. 
	button_1 = Window_AddNewControl(the_window, TEXT_BUTTON, width, height, x_offset, y_offset, H_ALIGN_LEFT, V_ALIGN_TOP, caption_1, the_id++, group_id);
	Control_SetPressed(button_1, CONTROL_UP);
	
	// for user docs:
	// 2nd way of adding a control is to call get a control template first, then call Window_AddNewControlFromTemplate()
	// You might do this if you had a set of similar buttons, etc, and wanted to re-use the same template for all the buttons. Don't forget to dispose of the template when done.
	ControlTemplate*	the_template;
	
	width = 100;
	y_offset += 30;

	if ( (the_template = Theme_CreateControlTemplateFlexWidth(the_theme, TEXT_BUTTON, width, height, x_offset, y_offset, H_ALIGN_LEFT, V_ALIGN_TOP, caption_2)) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create the control template", __func__, __LINE__));
		return;
	}

	button_2 = Window_AddNewControlFromTemplate(the_window, the_template, the_id++, group_id);
	Control_SetActive(button_2, CONTROL_ACTIVE);
	Control_SetPressed(button_2, CONTROL_DOWN);
	ControlTemplate_Destroy(&the_template);
	
	// for user docs:
	// 3rd way of adding a control is to call get a control template first, then turn that into a control, then pass that control to Window_AddControl()

	Control*	the_control;
	
	width = 120;
	y_offset += 30;

	if ( (the_template = Theme_CreateControlTemplateFlexWidth(the_theme, TEXT_BUTTON, width, height, x_offset, y_offset, H_ALIGN_LEFT, V_ALIGN_TOP, caption_2)) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create the control template", __func__, __LINE__));
		return;
	}

	the_control = Control_New(the_template, the_window, &the_window->content_rect_, the_id++, group_id);

	if ( Window_AddControl(the_window, the_control) == false)
	{
		LOG_ERR(("%s %d: Failed to add control to window", __func__, __LINE__));
		return;
	}
	
	Control_SetActive(the_control, CONTROL_INACTIVE);
	Control_SetPressed(the_control, CONTROL_UP);

	width = 140;
	y_offset += 30;
	button_4 = Window_AddNewControl(the_window, TEXT_BUTTON, width, height, x_offset, y_offset, H_ALIGN_LEFT, V_ALIGN_TOP, caption_4, the_id++, group_id);
	Control_SetActive(button_4, CONTROL_INACTIVE);
	Control_SetPressed(button_4, CONTROL_DOWN);
	
	
	// temporary until event handler is written: tell system to render the screen and all windows
	Sys_Render(global_system);	
}


void RunDemo(void)
{
	Window*				the_window;
	NewWinTemplate*		the_win_template;
	static char*		the_win_title = "My New Window";
	
	DEBUG_OUT(("%s %d: Setting graphics mode...", __func__, __LINE__));

	Sys_SetModeGraphics(global_system);
	
	if ( (the_win_template = Window_GetNewWinTemplate(the_win_title)) == NULL)
	{
		LOG_ERR(("%s %d: Could not get a new window template", __func__ , __LINE__));
		return;
	}	
	// note: all the default values are fine for us in this case.
	
	DEBUG_OUT(("%s %d: x=%i, y=%i, width=%i, title='%s'", __func__, __LINE__, the_win_template->x_, the_win_template->y_, the_win_template->width_, the_win_template->title_));
	
	if ( (the_window = Window_New(the_win_template, &HelloWindowEventHandler)) == NULL)
	{
		DEBUG_OUT(("%s %d: Couldn't instantiate a window", __func__, __LINE__));
		return;
	}

	// say hi
	Window_SetPenXY(the_window, 5, 5);

	if (Window_DrawString(the_window, (char*)"Hello, World", GEN_NO_STRLEN_CAP) == false)
	{
		// oh, no! you should handle this.
	}
	

	// draw some stuff in the window we created
	{
		// draw some color blocks
		int16_t i;
		int16_t x = 1;
		int16_t y = the_window->content_rect_.MinY + 50;
		int16_t height = 16;
	
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
	
	// temporary until event handler is written: tell system to render the screen and all windows
	//Sys_Render(global_system);

	// test adding controls?
	AddControls(the_window);

	// get ready to switch theme
	General_DelaySeconds(2);
	
	// switch themes?
	SwitchThemes(the_window);

	// test out buttona states?
	ToggleButtonStates(the_window);
	
// 	Window_Destroy(&the_window);

	// call something to shut the system down and free memory	
	
// 	Font_Destroy(&global_system_font);
// 	Font_Destroy(&the_icon_font);
	
	
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// handler for the hello world window
void HelloWindowEventHandler(EventRecord* the_event)
{
	DEBUG_OUT(("%s %d: event received!", __func__, __LINE__));
	
	return;
}


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