/*
 * bitmap_demo.c
 *
 *  Created on: Mar 10, 2022
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
void Demo_Bitmap_FillMemory1(void);
void Demo_Bitmap_FillMemory2(void);
void Demo_Bitmap_FillBox1(void);
void Demo_Bitmap_FillBox2(void);
void Demo_Bitmap_FillBox3(void);
void Demo_Bitmap_SetPixelAtXY(void);
void Demo_Bitmap_GetPixelAtXY(void);
void Demo_Bitmap_DrawHLine1(void);
void Demo_Bitmap_DrawLine(void);
void Demo_Bitmap_DrawBox(void);
void Demo_Bitmap_DrawBoxCoords(void);
void Demo_Bitmap_DrawRoundBox(void);
void Demo_Bitmap_DrawCircle(void);
void Demo_Bitmap_Blit1(void);
void Demo_Bitmap_BlitRect(void);
void Demo_Bitmap_ScreenResolution1(void);
void Demo_Bitmap_ScreenResolution2(void);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


// have user hit a key, then clear screens
void WaitForUser(void)
{
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"Press any key to continue", FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLUE);
	
	General_GetChar();
	
	Bitmap_FillMemory(Sys_GetScreenBitmap(global_system, back_layer), 0xbb);
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], ' ');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 0);
}

// Draw fancy box on the B screen and display demo description
void ShowDescription(char* the_message)
{
	int16_t		x1 = 0;
	int16_t		x2 = global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1;
	int16_t		y1 = 0;
	int16_t		y2 = 5;

	// draw box and fill contents in prep for next demo description
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, FG_COLOR_BRIGHT_BLUE, BG_COLOR_BLUE);
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, ' ', FG_COLOR_BRIGHT_WHITE, BG_COLOR_BLUE);
	
	// wrap text into the message box, leaving one row at the bottom for "press any key"
	Text_DrawStringInBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, the_message, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BLUE, NULL);
}


void Demo_Bitmap_FillMemory1(void)
{
	Bitmap_FillMemory(Sys_GetScreenBitmap(global_system, back_layer), 0x05);
	ShowDescription("Bitmap_FillMemory -> fill bitmap screen with value 0x05");	
	WaitForUser();
}


void Demo_Bitmap_FillMemory2(void)
{
	Bitmap_FillMemory(Sys_GetScreenBitmap(global_system, back_layer), 0xff);
	ShowDescription("Bitmap_FillMemory -> fill bitmap screen with value 0xff");	
	WaitForUser();
}


void Demo_Bitmap_FillBox1(void)
{
	int16_t x = 5;
	int	y = 8*6;
	int	width = global_system->screen_[ID_CHANNEL_B]->width_ - x - 30;
	int16_t height = global_system->screen_[ID_CHANNEL_B]->height_ - y - 100;
	
	ShowDescription("Bitmap_FillBox -> fill a square on screen with 0xff");	
	Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, 0xff);
	WaitForUser();
}


void Demo_Bitmap_FillBox2(void)
{
	int16_t x = 500;
	int	y = 8*6+100;
	int	width = global_system->screen_[ID_CHANNEL_B]->width_ - x - 30;
	int16_t height = global_system->screen_[ID_CHANNEL_B]->height_ - y - 100;
	
	ShowDescription("Bitmap_FillBox -> fill a square on screen with 0x05");	
	Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, 0x05);
	WaitForUser();
}


void Demo_Bitmap_FillBox3(void)
{
	int16_t x = 5;
	int	y = 8*6;
	
	ShowDescription("Bitmap_FillBox -> fill various squares with different color values");	
	Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x + 30, y, 250, 100, 0x55);
	Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, 25, 25, 0xf5);
	Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x, y + 50, 350, 200, 0x3f);
	WaitForUser();
}


void Demo_Bitmap_SetPixelAtXY(void)
{
	unsigned char	i;
	unsigned char*	junk_value = (unsigned char*)EA_USER;	// we'll just use whatever bytes we find in our code as x and y
	
	ShowDescription("Bitmap_SetPixelAtXY -> Draw pixels in various colors at various locations on screen");	
 	
 	for (i = 0; i < 254; i++)
 	{
	 	Bitmap_SetPixelAtXY(Sys_GetScreenBitmap(global_system, back_layer), *(junk_value++) + i, *(junk_value++) + i, i);
 	}
	WaitForUser();
}


void Demo_Bitmap_GetPixelAtXY(void)
{
	int16_t			x = 1;
	int16_t			y = 8*8;
	int16_t			width = 60;
	int16_t			height = 200;
	int16_t			color = 0x20;
	unsigned char	i;
	char			temp_buff[25];
	int16_t			text_y = (y+height)/8 + 1; // put it under the colored squares
	
	ShowDescription("Bitmap_GetPixelAtXY -> Get the color value of a specified pixel");	
 	
 	for (i = 0; i < 10; i++)
 	{
		int16_t	detected_color;

		Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, color);
	 	detected_color = Bitmap_GetPixelAtXY(Sys_GetScreenBitmap(global_system, back_layer), x, y);
	 	sprintf(temp_buff, "Set:%x", color);
	 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], (x+1)/8, text_y, (char*)temp_buff, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK);
	 	sprintf(temp_buff, "Got:%x", detected_color);
	 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], (x+1)/8, text_y+1, (char*)temp_buff, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK);
		color += 25;
		x += width;
 	}
	WaitForUser();
}


void Demo_Bitmap_DrawHLine1(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			line_len;

	ShowDescription("Text_DrawHLine / Text_DrawVLine -> Draw straight lines using a specified color");	
	
	x = 20;
	y = 8*(10);
	line_len = 200;
	Bitmap_DrawHLine(Sys_GetScreenBitmap(global_system, back_layer), x, y, line_len, 0xff);
	Bitmap_DrawHLine(Sys_GetScreenBitmap(global_system, back_layer), x, y + 20, line_len, 0xff);
	Bitmap_DrawHLine(Sys_GetScreenBitmap(global_system, back_layer), x, y + 40, line_len, 0xff);
	Bitmap_DrawVLine(Sys_GetScreenBitmap(global_system, back_layer), x + 100, y - 10, line_len, 0xff);
	Bitmap_DrawVLine(Sys_GetScreenBitmap(global_system, back_layer), x + 150, y - 10, line_len + 50, 0xff);

	WaitForUser();
}


void Demo_Bitmap_DrawLine(void)
{
	int16_t			x1 = 45;
	int16_t			y1 = 100;
	int16_t			x2 = 630;
	int16_t			y2 = 350;
	int16_t			i;

	ShowDescription("Bitmap_DrawLine -> Draw a line from any coordinate to any coordinate");	

	for (i = 0; i < 256; i++)
	{
		Bitmap_DrawLine(Sys_GetScreenBitmap(global_system, back_layer), x1, y1, x2, y2, i);
		
		x1 += 2;
		x2 -= 2;
	}

// 	Bitmap_DrawLine(Sys_GetScreenBitmap(global_system, back_layer), x1, y1, x2, y2, 0xee);
// 	Bitmap_DrawLine(Sys_GetScreenBitmap(global_system, back_layer), x2, y1, x1, y2, 0xce);
// 	Bitmap_DrawLine(Sys_GetScreenBitmap(global_system, back_layer), x2, y1+20, x1, y2-20, 0x88);
// 	Bitmap_DrawLine(Sys_GetScreenBitmap(global_system, back_layer), x2, y1+40, x1, y2-40, 0x55);
	
	WaitForUser();
}


void Demo_Bitmap_DrawBox(void)
{
	int16_t x = 60;
	int	y = 8*8;
	int	width = global_system->screen_[ID_CHANNEL_B]->width_ - x - 60;
	int16_t height = 300;

	ShowDescription("Bitmap_DrawBox -> Draw a filled or unfilled box. Supply start coordinates, width, height, color, and fill choice.");	

	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, 0xff, PARAM_DO_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, 0x55, PARAM_DO_NOT_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x + 25, y + 25, width, height, 0x33, PARAM_DO_NOT_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x + 50, y + 50, width, height, 0x77, PARAM_DO_NOT_FILL);

	x = 0;
	width = 10;
	height = 10;
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, 0xff, PARAM_DO_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, 0x55, PARAM_DO_NOT_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), x+11, y, width, height, 0xff, PARAM_DO_FILL);
	
	WaitForUser();
}


void Demo_Bitmap_DrawBoxCoords(void)
{
	int16_t			x1 = 45;
	int16_t			y1 = 100;
	int16_t			x2 = 630;
	int16_t			y2 = 350;

	ShowDescription("Bitmap_DrawBoxCoords -> Draw a box using 4 coordinates.");	

	Bitmap_DrawBoxCoords(Sys_GetScreenBitmap(global_system, back_layer), x1, y1, x2, y2, 0xff);
	WaitForUser();
}


void Demo_Bitmap_DrawRoundBox(void)
{
	int16_t 	x = 60;
	int16_t	y = 8*7;
	int16_t	width = 40;
	int16_t	height = 20;
	int16_t	radius = 8;
	int16_t	i;
	int16_t	color = 0x20;
	int16_t	line_color = 0xff;
	int16_t	xleft = 560;
	
	ShowDescription("Demo_Bitmap_DrawRoundBox -> Draw an unfilled rect with rounded corners. Specify start coords, width, height, corner radius, color, and fill/no-fill.");	

	for (i = 0; i <= 20; i++)
	{
		Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), x + color, y + color, width + i*2, height + i*2, i, color, PARAM_DO_NOT_FILL);
		Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), xleft - color, y + color, width*2 + i*2, height*2 + i*2, 20 - i, color, PARAM_DO_FILL);
		Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), xleft - color, y + color, width*2 + i*2, height*2 + i*2, 20 - i, line_color, PARAM_DO_NOT_FILL);
		color += 7;
	}

	// one filled one - SLOW - dangerous with small stack
	x = 80;
// 	y = 50;
	width = 80;
	height = 16;
	radius = 5;
	color = 0xFF;
// 	Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, radius, color);
// 	Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), x + radius, y + radius, width - radius*2, height-radius*2, color);
// 	Bitmap_Fill(Sys_GetScreenBitmap(global_system, back_layer), x + radius, y + 1, color);
// 	Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, radius, 0x01);
// 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], (x)/8-2, y/8-1, (char*)"Cancel", FG_COLOR_BLACK, BG_COLOR_BLACK);
// 	getchar();
	
	// faster fill by making rect fills and then just flood filling the corners
	y = 250;
	Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, radius, color, PARAM_DO_FILL);
	Bitmap_DrawRoundBox(Sys_GetScreenBitmap(global_system, back_layer), x, y, width, height, radius, 0x01, PARAM_DO_NOT_FILL);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], (x)/8-2, y/8-1, (char*)"Cancel", FG_COLOR_BLACK, BG_COLOR_BLACK);


	
	WaitForUser();
}


void Demo_Bitmap_DrawCircle(void)
{
	int16_t			x1 = 320;
	int16_t			y1 = 200;
	int16_t			radius = 6;
	int16_t			i;

	ShowDescription("Bitmap_DrawCircle -> Draw a circle");	

	for (i = 0; i < 256 && radius < 238; i += 3)
	{
		Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), x1, y1, radius, i);
		
		radius += 3;
	}

	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 12, 0xff);
	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 15, 0xff);

	WaitForUser();
}


void Demo_Bitmap_Blit1(void)
{
	int16_t			box_height = 16;
	int16_t			box_width = global_system->screen_[ID_CHANNEL_B]->width_;
	int16_t			color = 0x20;
	int16_t			i;
	Bitmap			src_bm;
	Bitmap			dst_bm;

	ShowDescription("Bitmap_Blit -> Copy a rectange of pixels from one bitmap to another");	

	// draw 30 horizontal bands of color to help judge blit effects
	
	for (i = 0; i < 30; i++)
	{
		Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), 0, box_height * i, box_width, box_height, color);		
		color += 7;
	}
	
 	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 12, 0x88);
	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 15, 0xcc);
	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 20, 0xff);

	// copy bits of this screen to other parts of the Screen
	src_bm.addr_ = (unsigned char*)VRAM_START;
	src_bm.width_ = global_system->screen_[ID_CHANNEL_B]->width_;
	src_bm.height_ = global_system->screen_[ID_CHANNEL_B]->height_;
	dst_bm.addr_ = (unsigned char*)VRAM_START;
	dst_bm.width_ = global_system->screen_[ID_CHANNEL_B]->width_;
	dst_bm.height_ = global_system->screen_[ID_CHANNEL_B]->height_;

	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 100, 0, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 100, 100, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 100, 200, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 100, 300, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 200, 100, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 200, 200, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 200, 300, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 300, 100, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 300, 200, 50, 50);
	Bitmap_Blit(&src_bm, 0, 0, &dst_bm, 300, 300, 50, 50);
// 	Bitmap_Blit(&src_bm, x1 - 100, y1 - 100, &dst_bm, 0, 0, 100, 100);
// 	Bitmap_Blit(&src_bm, x1 - 100, y1 - 100, &dst_bm, 400, 300, 100, 100);
	
	// do a 'dragon' effect
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), 550, 350, 20, 20, 0xff, PARAM_DO_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), 550, 350, 30, 30, 0xcc, PARAM_DO_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), 550, 350, 40, 40, 0xbb, PARAM_DO_FILL);
	Bitmap_DrawBox(Sys_GetScreenBitmap(global_system, back_layer), 550, 350, 50, 50, 0x99, PARAM_DO_FILL);
	
	for (i = 0; i < 25; i++)
	{
		Bitmap_Blit(&src_bm, 550, 350, &dst_bm, i*10, 350 - i*10, 50, 50);
	}
	
	WaitForUser();
}


void Demo_Bitmap_BlitRect(void)
{
	int16_t			box_height = 2;
	int16_t			box_width = 2;
	int16_t			color = 1;
	int16_t			i;
	Bitmap*			src_bm;
	Bitmap*			dst_bm;
	Rectangle		src_rect;

	ShowDescription("Bitmap_BlitRect -> Copy a rectange of pixels from one bitmap to another");	

	// draw 30 horizontal bands of color to help judge blit effects
	
	for (i = 0; i < 30; i++)
	{
		Bitmap_FillBox(Sys_GetScreenBitmap(global_system, back_layer), 0, box_height * i, box_width * i, box_height, color);		
		color += 4;
	}
	
 	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 12, SYS_COLOR_RED1);
	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 15, SYS_COLOR_GREEN1);
	Bitmap_DrawCircle(Sys_GetScreenBitmap(global_system, back_layer), 25, 25, 20, SYS_COLOR_BLUE1);

	// copy bits of this screen to other parts of the Screen
	src_bm = Sys_GetScreenBitmap(global_system, back_layer);
	dst_bm = Sys_GetScreenBitmap(global_system, back_layer);
	
	src_rect.MinX = 0;

	for (i = 0; i < 30; i++)
	{
		src_rect.MaxX = box_width * i;
		src_rect.MinY = box_height * i;
		src_rect.MaxY = src_rect.MinY + box_height;
		
		Bitmap_BlitRect(src_bm, src_rect, dst_bm, 0, 200 + box_height * i);
	}
	
	WaitForUser();
}


// bool Bitmap_Blit(Screen* the_screen, Bitmap* src_bm, int16_t src_x, int16_t src_y, Bitmap* dst_bm, int16_t dst_x, int16_t dst_y, int16_t width, int16_t height);



void Demo_Bitmap_ScreenResolution1(void)
{
	char			msg_buffer[80*3];
	char*			the_message = msg_buffer;
	int16_t			y = 7;
	
	Sys_SetVideoMode(global_system->screen_[ID_CHANNEL_B], RES_800X600);
	ShowDescription("Sys_SetVideoMode -> (RES_800X600) Changes resolution to 800x600 if available for this screen/channel.");	

	sprintf(the_message, "Requested 800x600. Actual: %i x %i, %i x %i text, %i x %i visible text", 
		global_system->screen_[ID_CHANNEL_B]->width_, 
		global_system->screen_[ID_CHANNEL_B]->height_, 
		global_system->screen_[ID_CHANNEL_B]->text_mem_cols_, 
		global_system->screen_[ID_CHANNEL_B]->text_mem_rows_, 
		global_system->screen_[ID_CHANNEL_B]->text_cols_vis_, 
		global_system->screen_[ID_CHANNEL_B]->text_rows_vis_
		);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y, the_message, FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y + 1, (char*)"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", FG_COLOR_BLUE, BG_COLOR_BRIGHT_YELLOW);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y + 2, (char*)"<-START OF LINE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, 70, (char*)"ROW70", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_B], y + 3);

	WaitForUser();
}


void Demo_Bitmap_ScreenResolution2(void)
{
	char			msg_buffer[80*3];
	char*			the_message = msg_buffer;
	int16_t			y = 7;
	
	Sys_SetVideoMode(global_system->screen_[ID_CHANNEL_B], RES_640X480);
	ShowDescription("Sys_SetVideoMode -> (RES_640X480) Changes resolution to 640x480 if available for this screen/channel.");	

	sprintf(the_message, "Requested 640x480. Actual: %i x %i, %i x %i text, %i x %i visible text", 
		global_system->screen_[ID_CHANNEL_B]->width_, 
		global_system->screen_[ID_CHANNEL_B]->height_, 
		global_system->screen_[ID_CHANNEL_B]->text_mem_cols_, 
		global_system->screen_[ID_CHANNEL_B]->text_mem_rows_, 
		global_system->screen_[ID_CHANNEL_B]->text_cols_vis_, 
		global_system->screen_[ID_CHANNEL_B]->text_rows_vis_
		);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y, the_message, FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y + 1, (char*)"01234567890123456789012345678901234567890123456789012345678901234567890123456789", FG_COLOR_BLUE, BG_COLOR_BRIGHT_YELLOW);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y + 2, (char*)"<-START OF LINE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, 55, (char*)"ROW55", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_B], y + 3);

	WaitForUser();
}




void RunDemo(void)
{
// 	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], ' ');
// 	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 160);

	ShowDescription("Welcome to the Graphics Library Demo!");	
	WaitForUser();
	
	Demo_Bitmap_FillMemory1();
	Demo_Bitmap_FillMemory2();
	
	Demo_Bitmap_FillBox1();
	Demo_Bitmap_FillBox2();
	Demo_Bitmap_FillBox3();

	Demo_Bitmap_SetPixelAtXY();

 	Demo_Bitmap_GetPixelAtXY();

	Demo_Bitmap_DrawHLine1();
	
	Demo_Bitmap_DrawLine();
	
	Demo_Bitmap_DrawBox();
	Demo_Bitmap_DrawBoxCoords();

	Demo_Bitmap_DrawRoundBox();
	
	Demo_Bitmap_DrawCircle();
	
	Demo_Bitmap_Blit1();
	Demo_Bitmap_BlitRect();
	
	Demo_Bitmap_ScreenResolution1();
	Demo_Bitmap_ScreenResolution2();
	
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/




int main(int argc, char* argv[])
{
	if ( Sys_InitSystem() == false)
	{
		DEBUG_OUT(("%s %d: Couldn't initialize the system", __func__, __LINE__));
		exit(0);
	}
	
	//Sys_SetModeGraphics(global_system);
	Sys_SetModeText(global_system, true);
	
	RunDemo();

	return 0;
}