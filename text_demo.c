/*
 * text_demo.c
 *
 *  Created on: Mar 5, 2022
 *      Author: micahbly
 *
 *  Demonstrates many of the features of the text library
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

// simple function for testing passing a function hook for "do something to see another page of text" for the drawstringinbox stuff. 
//! @return	returns true if the user wants to continue, or false if the user wants to stop the current action.
bool Test_MyGetUserResponseFunc(void);

// various demos
void Demo_Text_FillCharMem1(void);
void Demo_Text_FillCharMem2(void);
void Demo_Text_FillAttrMem1(void);
void Demo_Text_FillAttrMem2(void);
void Demo_Text_FillBoxSlow1(void);
void Demo_Text_FillBoxSlow2(void);
void Demo_Text_FillBox1(void);
void Demo_Text_FillBox2(void);
void Demo_Text_FillBox3(void);
void Demo_Text_InvertBox(void);
void Demo_Text_ShowFontChars(void);
void Demo_Text_SetCharAndColorAtXY(void);
void Demo_Text_DrawHLine1(void);
void Demo_Text_DrawHLine2(void);
void Demo_Text_DrawHLine3(void);
void Demo_Text_NamedColors(void);
void Demo_Text_DrawBox(void);
void Demo_Text_DrawBoxCoords(void);
void Demo_Text_DrawBoxCoordsFancy(void);
void Demo_Text_DrawStringAtXY(void);
void Demo_Text_DrawStringInBox1(void);
void Demo_Text_DrawStringInBox2(void);
void Demo_Text_ScreenResolution1(void);
void Demo_Text_ScreenResolution2(void);
void Demo_Text_ScreenResolution3(void);
void Demo_Text_ScreenResolution4(void);
void Demo_Text_UpdateFontData(void);
void Demo_Text_CopyMemBox1(void);
void Demo_Text_CopyMemBox2(void);

/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


// simple function for testing passing a function hook for "do something to see another page of text" for the drawstringinbox stuff. 
//! @return	returns true if the user wants to continue, or false if the user wants to stop the current action.
bool Test_MyGetUserResponseFunc(void)
{
	unsigned char	c;
	
	c = General_GetChar();
	
	if (c == 'q')
	{
		return false;
	}
	
	return true;
}



// have user hit a key, then clear screens
void WaitForUser(void)
{
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"Press any key to continue", FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLUE);
	
	General_GetChar();
	
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_A], ' ');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_A], 159);
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], ' ');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 159);
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


void Demo_Text_CopyMemBox1(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			h_line_len;
	int16_t			v_line_len;
	char			b1[80*60];
	char			b2[80*60];
	char*			buffer1 = b1;
	char*			buffer2 = b2;
	char*			the_message;
	
	x = 0;
	y = 6;
	h_line_len = 20;
	v_line_len = 4;

	ShowDescription("Text_CopyMemBox -> copy a rectangular chunk of text to the screen from a buffer. The screen and off-screen buffer must have the same dimensions. You can copy from one part of the screen to another by specifying src and dst buffers that match.");	
	
	// draw some text - 80x4
	the_message = (char*)"12345678901234567890                                                            *This is some text *                                                            *from the offscreen*                                                            *buffer.           *                                                            ";
	//2022-09-03: memset freezes machine and debugger. TEMP comment out until Calypsi fix, etc.
	memset(buffer1, '.', 80*60); // fill with a dot first
	memcpy(buffer1 + 6*80, the_message, 80*4+1);
	
	// copy text to channel B, from off-screen buffer 1	
	Text_CopyMemBox(global_system->screen_[ID_CHANNEL_B], buffer1, x + 0, y, x+h_line_len, y+v_line_len, SCREEN_COPY_TO_SCREEN, SCREEN_FOR_TEXT_CHAR);
	
	WaitForUser();
}


void Demo_Text_CopyMemBox2(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			h_line_len;
	int16_t			v_line_len;
	char			b1[80*60];
	char*			buffer1 = b1;
	char			b2[80*60];
	char*			buffer2 = b2;
	
	h_line_len = 20;
	v_line_len = 4;

	int16_t			line_len;
	unsigned char	the_char;
	int16_t			i;
	int16_t			num_colors = 16;


	// draw some rows of color across the screen
	x = 1;
	y = 10;
	line_len = 60;
	the_char = CH_DIAMOND;	

	// fill background with a char so we can see what's copied and not
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], 'X');

	ShowDescription("Text_CopyMemBox -> copy a rectangular chunk of color (attributes) to the screen from a buffer and vice versa.");	

	for (i = 0; i < num_colors; i++)
	{
		Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + i, line_len, the_char, i, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	}
	
	// fill buffer 1 and 2 with a different colors
	//2022-09-03: memset freezes machine and debugger. TEMP comment out until Calypsi fix, etc.
// 	memset(buffer1, 176, 80*60); // 176=bright yellow on black
// 	memset(buffer2, 160, 80*60); // 160=bright green on black

	// copy colors on channel B, to off-screen buffer 1	
	Text_CopyMemBox(global_system->screen_[ID_CHANNEL_B], buffer1, x, y, x+line_len/2, y+v_line_len, SCREEN_COPY_FROM_SCREEN, SCREEN_FOR_TEXT_ATTR);
	
	General_GetChar();
	
	// copy colors in buffer 2 to screen (replacing what was there)
	Text_CopyMemBox(global_system->screen_[ID_CHANNEL_B], buffer2, x, y, x+line_len/2, y+v_line_len, SCREEN_COPY_TO_SCREEN, SCREEN_FOR_TEXT_ATTR);
	
	General_GetChar();
	
	// copy colors in buffer 1 to screen (restoring what had been there)
	Text_CopyMemBox(global_system->screen_[ID_CHANNEL_B], buffer1, x, y, x+line_len/2, y+v_line_len, SCREEN_COPY_TO_SCREEN, SCREEN_FOR_TEXT_ATTR);
	
	WaitForUser();
}


void Demo_Text_FillCharMem1(void)
{
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], 'Y');
	ShowDescription("Text_FillCharMem -> fill screen with the letter Y");	
	WaitForUser();
}


void Demo_Text_FillCharMem2(void)
{
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], CH_DIAMOND);
	ShowDescription("Text_FillCharMem -> fill screen with a diamond character");	
	WaitForUser();
}


void Demo_Text_FillAttrMem1(void)
{
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], CH_DIAMOND);
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 127);
	ShowDescription("Text_FillAttrMem -> fill screen with gray-on-black colors without changing characters");	
	WaitForUser();
}


void Demo_Text_FillAttrMem2(void)
{
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], CH_DIAMOND);
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 147);
	ShowDescription("Text_FillAttrMem -> fill screen with red-on-olive colors without changing characters");	
	WaitForUser();
}


void Demo_Text_FillBoxSlow1(void)
{
	ShowDescription("Text_FillBoxSlow -> fill a square on screen with a checkered pattern, black on white (slow routine)");	
	Text_FillBoxSlow(global_system->screen_[ID_CHANNEL_B], 0, 6, global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1, 35, CH_CHECKERED1, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE, CHAR_AND_ATTR);
	WaitForUser();
}


void Demo_Text_FillBoxSlow2(void)
{
	ShowDescription("Text_FillBoxSlow -> fill a square on screen with a different checkered pattern, blue on dark blue (slow routine)");	
	Text_FillBoxSlow(global_system->screen_[ID_CHANNEL_B], 2, 9, global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1, 35, CH_CHECKERED3, COLOR_BRIGHT_BLUE, COLOR_BLUE, CHAR_AND_ATTR);
	WaitForUser();
}


void Demo_Text_FillBox1(void)
{
	int16_t		x1;
	int16_t		y1;
	int16_t		x2;
	int16_t		y2;
	
	ShowDescription("Text_FillBox -> fill a square on screen with a checkered pattern, black on white (fast routine)");	

	x1 = 0;
	y1 = 8;
	x2 = 0;
	y2 = 8;

	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);

	y1 += 2;
	x2++;
	y2 += 3;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);

	y1 += 3;
	x2++;
	y2 += 4;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);

	y1 += 4;
	x2++;
	y2 += 5;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);

	y1 += 5;
	x2++;
	y2 += 6;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);

	WaitForUser();
}


void Demo_Text_FillBox2(void)
{
	ShowDescription("Text_FillBox -> fill a square on screen with a different checkered pattern, blue on dark blue (fast routine)");	
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], 6, 13, global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1, 39, CH_CHECKERED3, COLOR_BRIGHT_BLUE, COLOR_BLUE);
	WaitForUser();
}


void Demo_Text_FillBox3(void)
{
	ShowDescription("Text_FillBox -> fill various squares with colors and characters (fast routine)");	
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], 0, 7, 3, 9, CH_RIGHT, COLOR_BRIGHT_GREEN, COLOR_BLUE);
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], 0, 10, 2, 12, CH_LEFT, COLOR_BRIGHT_RED, COLOR_BLUE);
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], 0, 13, 4, 16, CH_UP, COLOR_BRIGHT_MAGENTA, COLOR_BLUE);
	WaitForUser();
}


void Demo_Text_InvertBox(void)
{
	ShowDescription("Text_InvertBox -> invert a rectangle of colors");	
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], 6, 10, 60, 35, CH_DIAMOND, FG_COLOR_RED, BG_COLOR_BRIGHT_WHITE);
	Text_InvertBox(global_system->screen_[ID_CHANNEL_B], 40, 0, global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1, 33);
	WaitForUser();
}


void Demo_Text_ShowFontChars(void)
{
	ShowDescription("Text_ShowFontChars -> show font characters on screen");	
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_B], 10);
	WaitForUser();
}


void Demo_Text_SetCharAndColorAtXY(void)
{
	ShowDescription("Text_SetCharAndColorAtXY -> Draw various characters and colors at various locations on screen");	
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 0, 14, 33, FG_COLOR_BLACK, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 1, 14, 34, FG_COLOR_RED, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 2, 14, 35, FG_COLOR_BRIGHT_GREEN, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 3, 14, 36, FG_COLOR_BRIGHT_BLUE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 4, 14, 37, FG_COLOR_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 5, 14, 38, FG_COLOR_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 6, 14, 39, FG_COLOR_BRIGHT_MAGENTA, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 7, 14, 40, FG_COLOR_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 8, 14, 41, COLOR_BLACK, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 9, 14, 42, FG_COLOR_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 10,14, 43, FG_COLOR_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 11,14, 44, FG_COLOR_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 12,14, 45, FG_COLOR_GRAY, BG_COLOR_GRAY);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 13,14, 46, FG_COLOR_WHITE, BG_COLOR_GRAY);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 14,14, 47, FG_COLOR_BRIGHT_WHITE, BG_COLOR_GRAY);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 15,14, 48, FG_COLOR_WHITE, BG_COLOR_GRAY);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 0, 15, 33, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BLACK);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 1, 15, 34, FG_COLOR_BRIGHT_WHITE, BG_COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 2, 15, 35, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BRIGHT_GREEN);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 3, 15, 36, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BRIGHT_BLUE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 4, 15, 37, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 5, 15, 38, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 6, 15, 39, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 7, 15, 40, FG_COLOR_BRIGHT_WHITE, BG_COLOR_GRAY);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 8, 15, 41, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 9, 15, 42, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 10, 15, 43, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 11, 15, 44, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 12, 15, 45, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 13, 15, 46, FG_COLOR_BRIGHT_WHITE, COLOR_RED);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 14, 15, 47, FG_COLOR_BRIGHT_WHITE, BG_COLOR_WHITE);
 	Text_SetCharAndColorAtXY(global_system->screen_[ID_CHANNEL_B], 15, 15, 48, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BRIGHT_WHITE);
	WaitForUser();
}


void Demo_Text_DrawHLine1(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			line_len;

	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], '.');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 31);

	ShowDescription("Text_DrawHLine / Text_DrawVLine -> Draw straight lines using a specified character (CHAR_ONLY)");	
	
	x = 20;
	y = 10;
	line_len = 20;
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y, line_len, CH_HEART, FG_COLOR_BRIGHT_GREEN, BG_COLOR_BLACK, CHAR_ONLY);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + 2, line_len, CH_HEART, FG_COLOR_BRIGHT_GREEN, BG_COLOR_BLACK, CHAR_ONLY);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + 4, line_len, CH_HEART, FG_COLOR_BRIGHT_GREEN, BG_COLOR_BLACK, CHAR_ONLY);
	Text_DrawVLine(global_system->screen_[ID_CHANNEL_B], x + 10, y - 1, line_len, CH_CLUB, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK, CHAR_ONLY);
	Text_DrawVLine(global_system->screen_[ID_CHANNEL_B], x + 15, y - 1, line_len + 5, CH_CLUB, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK, CHAR_ONLY);

	WaitForUser();
}



void Demo_Text_DrawHLine2(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			line_len;

	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], '.');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 31);

	ShowDescription("Text_DrawHLine / Text_DrawVLine -> Paint straight lines using a specified color combo (ATTR_ONLY)");	
	
	x = 20;
	y = 10;
	line_len = 20;
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y, line_len, CH_HEART, FG_COLOR_BRIGHT_GREEN, BG_COLOR_BLACK, ATTR_ONLY);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + 2, line_len, CH_HEART, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BRIGHT_GREEN, ATTR_ONLY);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + 4, line_len, CH_HEART, FG_COLOR_BRIGHT_BLUE, BG_COLOR_BLACK, ATTR_ONLY);
	Text_DrawVLine(global_system->screen_[ID_CHANNEL_B], x + 10, y - 1, line_len, CH_CLUB, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BRIGHT_BLUE, ATTR_ONLY);
	Text_DrawVLine(global_system->screen_[ID_CHANNEL_B], x + 15, y - 1, line_len + 5, CH_CLUB, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK, ATTR_ONLY);

	WaitForUser();
}


void Demo_Text_DrawHLine3(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			line_len;

	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], '.');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 31);

	ShowDescription("Text_DrawHLine / Text_DrawVLine -> Draw straight lines using a specified character and color combo (CHAR_AND_ATTR)");	
	
	x = 20;
	y = 10;
	line_len = 20;
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y, line_len, CH_HEART, FG_COLOR_BRIGHT_GREEN, BG_COLOR_BLACK, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + 2, line_len, CH_DIAMOND, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BRIGHT_GREEN, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + 4, line_len, CH_CLUB, FG_COLOR_BRIGHT_BLUE, BG_COLOR_BLACK, CHAR_AND_ATTR);
	Text_DrawVLine(global_system->screen_[ID_CHANNEL_B], x + 10, y - 1, line_len, CH_CLUB, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawVLine(global_system->screen_[ID_CHANNEL_B], x + 15, y - 1, line_len + 5, CH_SMILEY1, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK, CHAR_AND_ATTR);

	WaitForUser();
}


void Demo_Text_NamedColors(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			line_len;
	unsigned char	the_char;
	int16_t			i;
	int16_t			num_colors = 16;

	ShowDescription("(using named default colors) -> Set foreground and background colors with FG_COLOR_BLACK, BG_COLOR_BRIGHT_BLUE, etc.");	

	// draw 16 rows, 1 for each foreground color. 
	// draw foreground as solid / inverse spaces at left, background with middot at right
	x = 1;
	y = 10;
	line_len = 18;
	the_char = CH_SOLID;	

	for (i = 0; i < num_colors; i++)
	{
		Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x, y + i, line_len, the_char, i, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
		Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + 26, y + i, line_len, CH_MIDDOT, FG_COLOR_BLACK, i, CHAR_AND_ATTR);
	}

	// manually line up named colors
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 0, 6, the_char, FG_COLOR_BLACK, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 1, 6, the_char, FG_COLOR_RED, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 2, 6, the_char, FG_COLOR_GREEN, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 3, 6, the_char, FG_COLOR_YELLOW, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 4, 6, the_char, FG_COLOR_BLUE, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 5, 6, the_char, FG_COLOR_MAGENTA, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 6, 6, the_char, FG_COLOR_CYAN, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 7, 6, the_char, FG_COLOR_WHITE, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 8, 6, the_char, FG_COLOR_GRAY, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 9, 6, the_char, FG_COLOR_BRIGHT_RED, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 10, 6, the_char, FG_COLOR_BRIGHT_GREEN, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 11, 6, the_char, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 12, 6, the_char, FG_COLOR_BRIGHT_BLUE, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 13, 6, the_char, FG_COLOR_BRIGHT_MAGENTA, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 14, 6, the_char, FG_COLOR_BRIGHT_CYAN, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);
	Text_DrawHLine(global_system->screen_[ID_CHANNEL_B], x + line_len + 1, y + 15, 6, the_char, FG_COLOR_BRIGHT_WHITE, BG_COLOR_BRIGHT_BLUE, CHAR_AND_ATTR);

	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 0, (char*)"FG_COLOR_BLACK", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 1, (char*)"FG_COLOR_RED", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 2, (char*)"FG_COLOR_GREEN", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 3, (char*)"FG_COLOR_YELLOW", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 4, (char*)"FG_COLOR_BLUE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 5, (char*)"FG_COLOR_MAGENTA", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 6, (char*)"FG_COLOR_CYAN", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 7, (char*)"FG_COLOR_WHITE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 8, (char*)"FG_COLOR_GRAY", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 9, (char*)"FG_COLOR_BRIGHT_RED", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 10, (char*)"FG_COLOR_BRIGHT_GREEN", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 11, (char*)"FG_COLOR_BRIGHT_YELLOW", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 12, (char*)"FG_COLOR_BRIGHT_BLUE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 13, (char*)"FG_COLOR_BRIGHT_MAGENTA", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 14, (char*)"FG_COLOR_BRIGHT_CYAN", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], x + 26 + line_len + 1, y + 15, (char*)"FG_COLOR_BRIGHT_WHITE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);

	WaitForUser();
}


void Demo_Text_DrawBox(void)
{
	int16_t			x;
	int16_t			y;
	int16_t			h_line_len;
	int16_t			v_line_len;
	unsigned char	the_char;

	ShowDescription("Text_DrawBox -> Draw a basic box using start coordinates + width and height. All cells of the box will use the same character.");	

	x = 0;
	y = 8;
	h_line_len = 1;
	v_line_len = 1;
	the_char = CH_CHECKERED3;

	Text_DrawBox(global_system->screen_[ID_CHANNEL_B], x, y, h_line_len, v_line_len, the_char, FG_COLOR_BRIGHT_CYAN, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y += 2;
	h_line_len++;
	v_line_len++;
	Text_DrawBox(global_system->screen_[ID_CHANNEL_B], x, y, h_line_len, v_line_len, the_char, FG_COLOR_BRIGHT_CYAN, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y += 3;
	h_line_len++;
	v_line_len++;
	Text_DrawBox(global_system->screen_[ID_CHANNEL_B], x, y, h_line_len, v_line_len, the_char, FG_COLOR_BRIGHT_CYAN, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y += 4;
	h_line_len++;
	v_line_len++;
	Text_DrawBox(global_system->screen_[ID_CHANNEL_B], x, y, h_line_len, v_line_len, the_char, FG_COLOR_BRIGHT_CYAN, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y += 5;
	h_line_len++;
	v_line_len++;
	Text_DrawBox(global_system->screen_[ID_CHANNEL_B], x, y, h_line_len, v_line_len, the_char, FG_COLOR_BRIGHT_CYAN, BG_COLOR_CYAN, CHAR_AND_ATTR);

	WaitForUser();
}


void Demo_Text_DrawBoxCoords(void)
{
	int16_t			x1;
	int16_t			y1;
	int16_t			x2;
	int16_t			y2;
	unsigned char	the_char;

	ShowDescription("Text_DrawBoxCoords -> Draw a basic box using 4 coordinates. All cells of the box will use the same character.");	

	x1 = 0;
	y1 = 8;
	x2 = 0;
	y2 = 8;
	the_char = CH_CHECKERED1;

	Text_DrawBoxCoords(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, the_char, FG_COLOR_WHITE, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y1 += 2;
	x2++;
	y2 += 3;
	Text_DrawBoxCoords(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, the_char, FG_COLOR_WHITE, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y1 += 3;
	x2++;
	y2 += 4;
	Text_DrawBoxCoords(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, the_char, FG_COLOR_WHITE, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y1 += 4;
	x2++;
	y2 += 5;
	Text_DrawBoxCoords(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, the_char, FG_COLOR_WHITE, BG_COLOR_CYAN, CHAR_AND_ATTR);

	y1 += 5;
	x2++;
	y2 += 6;
	Text_DrawBoxCoords(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, the_char, FG_COLOR_WHITE, BG_COLOR_CYAN, CHAR_AND_ATTR);

	WaitForUser();
}


void Demo_Text_DrawBoxCoordsFancy(void)
{
	int16_t			x1;
	int16_t			y1;
	int16_t			x2;
	int16_t			y2;

	ShowDescription("Text_DrawBoxCoordsFancy -> Draw a box using 4 coordinates. The pre-defined 'wall' characters are used to build the box's outline. Text_FillBox() is used to add a fill.");	

	x1 = 1;
	y1 = 9;
	x2 = 1;
	y2 = 9;

	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1-1, y1-1, x2+1, y2+1, FG_COLOR_WHITE, BG_COLOR_GRAY);

	y1 += 3;
	x2++;
	y2 += 4;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1-1, y1-1, x2+1, y2+1, FG_COLOR_WHITE, BG_COLOR_GRAY);

	y1 += 4;
	x2++;
	y2 += 5;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1-1, y1-1, x2+1, y2+1, FG_COLOR_WHITE, BG_COLOR_GRAY);

	y1 += 6;
	x2++;
	y2 += 7;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1-1, y1-1, x2+1, y2+1, FG_COLOR_WHITE, BG_COLOR_GRAY);

	y1 += 8;
	x2++;
	y2 += 9;
	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, CH_CHECKERED3, COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1-1, y1-1, x2+1, y2+1, FG_COLOR_WHITE, BG_COLOR_GRAY);

	WaitForUser();
}


void Demo_Text_DrawStringAtXY(void)
{
	char*	the_message;
	int16_t	i;

	ShowDescription("Text_DrawStringAtXY -> Draw a string at the specified coordinates. No wrapping is performed. Will truncate at right edge of screen.");	

	the_message = General_StrlcpyWithAlloc((char*)"this is a string", 250);
	
	for (i = 6; i < global_system->screen_[ID_CHANNEL_B]->text_rows_vis_ - 1 && i*2 < global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1; i = i + 8)
	{	
		Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], i*2, i, the_message, FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLUE);
	}

	WaitForUser();
}


void Demo_Text_DrawStringInBox1(void)
{
	int16_t			x1;
	int16_t			y1;
	int16_t			x2;
	int16_t			y2;
	char*			the_message;

	ShowDescription("Text_DrawStringInBox -> Draw a string into the specified box coordinates. Wrap is performed and string is truncated after the specified space used up.");	

	the_message = General_StrlcpyWithAlloc((char*)"Allaire's machines pair a classic CPU like the 8/16-bit 65C816 or the 16/32-bit Motorola 68000 with an FPGA that provides colorful 2D graphics and emulates classic sound chips. Her latest product, the A2560K, also features an integrated keyboard like the home computers of the 1980s. They appeal to the kind of person who is not afraid of programming in assembly, enjoys electronic music, and likes old-school games. 'You need to be worried about your customer. You need to take care of your customer,' says Allaire. It's an approach that appears to have paid off in loyalty: '90 percent of my customers are repeat customers,' she says.", 1024);
	
	// small box
	x1 = 39;
	y1 = 19;
	x2 = global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 1;
	y2 = 41;

	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, ' ', BG_COLOR_BRIGHT_CYAN, BG_COLOR_BLACK);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, FG_COLOR_WHITE, BG_COLOR_BLACK);
	Text_DrawStringInBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, the_message, BG_COLOR_BRIGHT_CYAN, BG_COLOR_BLACK, NULL);

	free(the_message);

	WaitForUser();
}


void Demo_Text_DrawStringInBox2(void)
{
	int16_t			x1;
	int16_t			y1;
	int16_t			x2;
	int16_t			y2;
	char*			the_message;

	ShowDescription("Text_DrawStringInBox -> Optionally pass a pointer to a function that checks if another 'page' of content should be displayed. Demo: press any key to show more, or 'q' to stop.");	

	the_message = General_StrlcpyWithAlloc((char*)"\nThe Anecdote\n\nBill Atkinson worked mostly at home, but whenever he made significant progress he rushed in to Apple to show it off to anyone who would appreciate it. This time, he visited the Macintosh offices at Texaco Towers to show off his brand new oval routines in Quickdraw, which were implemented using a really clever algorithm.\n\nBill had added new code to QuickDraw (which was still called LisaGraf at this point) to draw circles and ovals very quickly. That was a bit hard to do on the Macintosh, since the math for circles usually involved taking square roots, and the 68000 processor in the Lisa and Macintosh didn't support floating point operations. But Bill had come up with a clever way to do the circle calculation that only used addition and subtraction, not even multiplication or division, which the 68000 could do, but was kind of slow at.\n\nBill's technique used the fact the sum of a sequence of odd numbers is always the next perfect square (For example, 1 + 3 = 4, 1 + 3 + 5 = 9, 1 + 3 + 5 + 7 = 16, etc). So he could figure out when to bump the dependent coordinate value by iterating in a loop until a threshold was exceeded. This allowed QuickDraw to draw ovals very quickly.\n\nBill fired up his demo and it quickly filled the Lisa screen with randomly-sized ovals, faster than you thought was possible. But something was bothering Steve Jobs. 'Well, circles and ovals are good, but how about drawing rectangles with rounded corners? Can we do that now, too?'\n\n'No, there's no way to do that. In fact it would be really hard to do, and I don't think we really need it'. I think Bill was a little miffed that Steve wasn't raving over the fast ovals and still wanted more.\n\nSteve suddenly got more intense. 'Rectangles with rounded corners are everywhere! Just look around this room!'. And sure enough, there were lots of them, like the whiteboard and some of the desks and tables. Then he pointed out the window. 'And look outside, there's even more, practically everywhere you look!'. He even persuaded Bill to take a quick walk around the block with him, pointing out every rectangle with rounded corners that he could find.\n\n\nWhen Steve and Bill passed a no-parking sign with rounded corners, it did the trick. 'OK, I give up', Bill pleaded. 'I'll see if it's as hard as I thought.' He went back home to work on it.\n\nBill returned to Texaco Towers the following afternoon, with a big smile on his face. His demo was now drawing rectangles with beautifully rounded corners blisteringly fast, almost at the speed of plain rectangles.", 80*60+1);	

	// medium box
	x1 = 12;
	y1 = 6;
	x2 = global_system->screen_[ID_CHANNEL_B]->text_cols_vis_ - 3;
	y2 = 51;

	Text_FillBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, ' ', FG_COLOR_WHITE, BG_COLOR_BRIGHT_WHITE);
	Text_DrawBoxCoordsFancy(global_system->screen_[ID_CHANNEL_B], x1, y1, x2, y2, FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE);
	Text_DrawStringInBox(global_system->screen_[ID_CHANNEL_B], x1+1, y1+1, x2-1, y2-1, the_message, FG_COLOR_BLACK, BG_COLOR_BRIGHT_WHITE, &Test_MyGetUserResponseFunc);

	free(the_message);

	WaitForUser();
}


void Demo_Text_ScreenResolution1(void)
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


void Demo_Text_ScreenResolution2(void)
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


void Demo_Text_ScreenResolution3(void)
{
	char			msg_buffer[80*3];
	char*			the_message = msg_buffer;
	int16_t			y = 7;
	
	Sys_SetVideoMode(global_system->screen_[ID_CHANNEL_A], RES_800X600);
	ShowDescription("Sys_SetVideoMode -> (RES_800X600) Changes resolution to 800x600 if available for this screen/channel.");	

	sprintf(the_message, "Requested 800x600. Actual: %i x %i, %i x %i text, %i x %i visible text", 
		global_system->screen_[ID_CHANNEL_A]->width_, 
		global_system->screen_[ID_CHANNEL_A]->height_, 
		global_system->screen_[ID_CHANNEL_A]->text_mem_cols_, 
		global_system->screen_[ID_CHANNEL_A]->text_mem_rows_, 
		global_system->screen_[ID_CHANNEL_A]->text_cols_vis_, 
		global_system->screen_[ID_CHANNEL_A]->text_rows_vis_
		);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, y, the_message, FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, y + 1, (char*)"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", FG_COLOR_BLUE, BG_COLOR_BRIGHT_YELLOW);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, y + 2, (char*)"<-START OF LINE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y + 3, (char*)"800x600 should now be showing on Channel A", FG_COLOR_BRIGHT_RED, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, 70, (char*)"ROW70", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_A], y + 4);

	WaitForUser();
}


void Demo_Text_ScreenResolution4(void)
{
	char			msg_buffer[80*3];
	char*			the_message = msg_buffer;
	int16_t			y = 7;
	
	Sys_SetVideoMode(global_system->screen_[ID_CHANNEL_A], RES_1024X768);
	ShowDescription("Sys_SetVideoMode -> (RES_1024X768) Changes resolution to 1024x768 if available for this screen/channel.");	

	sprintf(the_message, "Requested 1024x768. Actual: %i x %i, %i x %i text, %i x %i visible text", 
		global_system->screen_[ID_CHANNEL_A]->width_, 
		global_system->screen_[ID_CHANNEL_A]->height_, 
		global_system->screen_[ID_CHANNEL_A]->text_mem_cols_, 
		global_system->screen_[ID_CHANNEL_A]->text_mem_rows_, 
		global_system->screen_[ID_CHANNEL_A]->text_cols_vis_, 
		global_system->screen_[ID_CHANNEL_A]->text_rows_vis_
		);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, y, the_message, FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, y + 1, (char*)"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567", FG_COLOR_BLUE, BG_COLOR_BRIGHT_YELLOW);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, y + 2, (char*)"<-START OF LINE", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 0, y + 3, (char*)"1024x768 should now be showing on Channel A", FG_COLOR_BRIGHT_RED, BG_COLOR_BRIGHT_GREEN);
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_A], 0, 91, (char*)"ROW91", FG_COLOR_BLACK, BG_COLOR_BRIGHT_GREEN);
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_A], y + 4);

	WaitForUser();
}


void Demo_Text_UpdateFontData(void)
{
	// until file objects available in emulator, need to embed data to test font replacement.
	// this is a remapped C64 font for code page 437

	// Exported using VChar64 v0.2.4
	// Total bytes: 2048
	unsigned char testfont[] = {  
		0xf0,0xf0,0xf0,0xf0,0x0f,0x0f,0x0f,0x0f,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xff,0x36,0x7f,0x7f,0x7f,0x3e,0x1c,0x08,0x00,
		0x08,0x1c,0x3e,0x7f,0x3e,0x1c,0x08,0x00,0x18,0x18,0x66,0x66,0x18,0x18,0x3c,0x00,
		0x08,0x1c,0x3e,0x7f,0x7f,0x1c,0x3e,0x00,0x00,0x3c,0x7e,0x7e,0x7e,0x7e,0x3c,0x00,
		0xff,0xc3,0x81,0x81,0x81,0x81,0xc3,0xff,0x00,0x3c,0x7e,0x66,0x66,0x7e,0x3c,0x00,
		0xff,0xc3,0x81,0x99,0x99,0x81,0xc3,0xff,0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,
		0xff,0xfe,0xfc,0xf8,0xf0,0xe0,0xc0,0x80,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0x00,0x00,
		0xff,0xff,0xfc,0xc1,0x89,0xc9,0xc9,0xff,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,
		0xf7,0xe3,0xc1,0x80,0x80,0xe3,0xc1,0xff,0xff,0xff,0xff,0x00,0x00,0xff,0xff,0xff,
		0xff,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,
		0xff,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xff,0xff,
		0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xcf,0xcf,0xcf,0xcf,0xcf,0xcf,0xcf,0xcf,
		0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xff,0xff,0xff,0x1f,0x0f,0xc7,0xe7,0xe7,
		0xe7,0xe7,0xe3,0xf0,0xf8,0xff,0xff,0xff,0x00,0x10,0x30,0x7f,0x7f,0x30,0x10,0x00,
		0x00,0x00,0x00,0x00,0x0f,0x0f,0x0f,0x0f,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
		0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x03,0x07,0x0e,0x1c,0x38,0x70,0xe0,0xc0,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x00,
		0x66,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x66,0x66,0xff,0x66,0xff,0x66,0x66,0x00,
		0x18,0x3e,0x60,0x3c,0x06,0x7c,0x18,0x00,0x62,0x66,0x0c,0x18,0x30,0x66,0x46,0x00,
		0x3c,0x66,0x3c,0x38,0x67,0x66,0x3f,0x00,0x06,0x0c,0x18,0x00,0x00,0x00,0x00,0x00,
		0x0c,0x18,0x30,0x30,0x30,0x18,0x0c,0x00,0x30,0x18,0x0c,0x0c,0x0c,0x18,0x30,0x00,
		0x00,0x66,0x3c,0xff,0x3c,0x66,0x00,0x00,0x00,0x18,0x18,0x7e,0x18,0x18,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x7e,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x03,0x06,0x0c,0x18,0x30,0x60,0x00,
		0x3c,0x66,0x6e,0x76,0x66,0x66,0x3c,0x00,0x18,0x18,0x38,0x18,0x18,0x18,0x7e,0x00,
		0x3c,0x66,0x06,0x0c,0x30,0x60,0x7e,0x00,0x3c,0x66,0x06,0x1c,0x06,0x66,0x3c,0x00,
		0x06,0x0e,0x1e,0x66,0x7f,0x06,0x06,0x00,0x7e,0x60,0x7c,0x06,0x06,0x66,0x3c,0x00,
		0x3c,0x66,0x60,0x7c,0x66,0x66,0x3c,0x00,0x7e,0x66,0x0c,0x18,0x18,0x18,0x18,0x00,
		0x3c,0x66,0x66,0x3c,0x66,0x66,0x3c,0x00,0x3c,0x66,0x66,0x3e,0x06,0x66,0x3c,0x00,
		0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x30,
		0x0e,0x18,0x30,0x60,0x30,0x18,0x0e,0x00,0x00,0x00,0x7e,0x00,0x7e,0x00,0x00,0x00,
		0x70,0x18,0x0c,0x06,0x0c,0x18,0x70,0x00,0x3c,0x66,0x06,0x0c,0x18,0x00,0x18,0x00,
		0x3c,0x66,0x6e,0x6e,0x60,0x62,0x3c,0x00,0x18,0x3c,0x66,0x7e,0x66,0x66,0x66,0x00,
		0x7c,0x66,0x66,0x7c,0x66,0x66,0x7c,0x00,0x3c,0x66,0x60,0x60,0x60,0x66,0x3c,0x00,
		0x78,0x6c,0x66,0x66,0x66,0x6c,0x78,0x00,0x7e,0x60,0x60,0x78,0x60,0x60,0x7e,0x00,
		0x7e,0x60,0x60,0x78,0x60,0x60,0x60,0x00,0x3c,0x66,0x60,0x6e,0x66,0x66,0x3c,0x00,
		0x66,0x66,0x66,0x7e,0x66,0x66,0x66,0x00,0x3c,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,
		0x1e,0x0c,0x0c,0x0c,0x0c,0x6c,0x38,0x00,0x66,0x6c,0x78,0x70,0x78,0x6c,0x66,0x00,
		0x60,0x60,0x60,0x60,0x60,0x60,0x7e,0x00,0x63,0x77,0x7f,0x6b,0x63,0x63,0x63,0x00,
		0x66,0x76,0x7e,0x7e,0x6e,0x66,0x66,0x00,0x3c,0x66,0x66,0x66,0x66,0x66,0x3c,0x00,
		0x7c,0x66,0x66,0x7c,0x60,0x60,0x60,0x00,0x3c,0x66,0x66,0x66,0x66,0x3c,0x0e,0x00,
		0x7c,0x66,0x66,0x7c,0x78,0x6c,0x66,0x00,0x3c,0x66,0x60,0x3c,0x06,0x66,0x3c,0x00,
		0x7e,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x3c,0x00,
		0x66,0x66,0x66,0x66,0x66,0x3c,0x18,0x00,0x63,0x63,0x63,0x6b,0x7f,0x77,0x63,0x00,
		0x66,0x66,0x3c,0x18,0x3c,0x66,0x66,0x00,0x66,0x66,0x66,0x3c,0x18,0x18,0x18,0x00,
		0x7e,0x06,0x0c,0x18,0x30,0x60,0x7e,0x00,0x3c,0x30,0x30,0x30,0x30,0x30,0x3c,0x00,
		0xc0,0xe0,0x70,0x38,0x1c,0x0e,0x07,0x03,0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3c,0x00,
		0x00,0x18,0x3c,0x7e,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
		0x00,0x00,0x00,0xe0,0xf0,0x38,0x18,0x18,0x00,0x00,0x3c,0x06,0x3e,0x66,0x3e,0x00,
		0x00,0x60,0x60,0x7c,0x66,0x66,0x7c,0x00,0x00,0x00,0x3c,0x60,0x60,0x60,0x3c,0x00,
		0x00,0x06,0x06,0x3e,0x66,0x66,0x3e,0x00,0x00,0x00,0x3c,0x66,0x7e,0x60,0x3c,0x00,
		0x00,0x0e,0x18,0x3e,0x18,0x18,0x18,0x00,0x00,0x00,0x3e,0x66,0x66,0x3e,0x06,0x7c,
		0x00,0x60,0x60,0x7c,0x66,0x66,0x66,0x00,0x00,0x18,0x00,0x38,0x18,0x18,0x3c,0x00,
		0x00,0x06,0x00,0x06,0x06,0x06,0x06,0x3c,0x00,0x60,0x60,0x6c,0x78,0x6c,0x66,0x00,
		0x00,0x38,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,0x00,0x66,0x7f,0x7f,0x6b,0x63,0x00,
		0x00,0x00,0x7c,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x3c,0x66,0x66,0x66,0x3c,0x00,
		0x00,0x00,0x7c,0x66,0x66,0x7c,0x60,0x60,0x00,0x00,0x3e,0x66,0x66,0x3e,0x06,0x06,
		0x00,0x00,0x7c,0x66,0x60,0x60,0x60,0x00,0x00,0x00,0x3e,0x60,0x3c,0x06,0x7c,0x00,
		0x00,0x18,0x7e,0x18,0x18,0x18,0x0e,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x3e,0x00,
		0x00,0x00,0x66,0x66,0x66,0x3c,0x18,0x00,0x00,0x00,0x63,0x6b,0x7f,0x3e,0x36,0x00,
		0x00,0x00,0x66,0x3c,0x18,0x3c,0x66,0x00,0x00,0x00,0x66,0x66,0x66,0x3e,0x0c,0x78,
		0x00,0x00,0x7e,0x0c,0x18,0x30,0x7e,0x00,0x18,0x18,0x18,0xff,0xff,0x18,0x18,0x18,
		0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x18,0x18,0x18,0xff,0xff,0x00,0x00,0x00,
		0x0c,0x12,0x30,0x7c,0x30,0x62,0xfc,0x00,0x01,0x03,0x06,0x6c,0x78,0x70,0x60,0x00,
		0xff,0xff,0xff,0xe0,0xe0,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0x00,0x00,0xff,0xff,0xff,
		0xff,0xff,0xff,0x00,0x00,0xe7,0xe7,0xe7,0x3f,0x3f,0xcf,0xcf,0x3f,0x3f,0xcf,0xcf,
		0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
		0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
		0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe7,0xe7,0xc7,0x0f,0x1f,0xff,0xff,0xff,
		0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x3f,0x1f,0x8f,0xc7,0xe3,0xf1,0xf8,0xfc,
		0xfc,0xf8,0xf1,0xe3,0xc7,0x8f,0x1f,0x3f,0x00,0x00,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,
		0x00,0x00,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0x0f,0x0f,0x0f,0x0f,0xf0,0xf0,0xf0,0xf0,
		0x00,0x00,0x00,0x00,0xf0,0xf0,0xf0,0xf0,0xc9,0x80,0x80,0x80,0xc1,0xe3,0xf7,0xff,
		0x9f,0x9f,0x9f,0x9f,0x9f,0x9f,0x9f,0x9f,0xff,0xff,0xff,0xf8,0xf0,0xe3,0xe7,0xe7,
		0x3c,0x18,0x81,0xc3,0xc3,0x81,0x18,0x3c,0x0f,0x0f,0x0f,0x0f,0x00,0x00,0x00,0x00,
		0xe7,0xe7,0x99,0x99,0xe7,0xe7,0xc3,0xff,0xf9,0xf9,0xf9,0xf9,0xf9,0xf9,0xf9,0xf9,
		0xf7,0xe3,0xc1,0x80,0xc1,0xe3,0xf7,0xff,0xe7,0xe7,0xe7,0x00,0x00,0xe7,0xe7,0xe7,
		0xc0,0xc0,0x30,0x30,0xc0,0xc0,0x30,0x30,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
		0x18,0x18,0x18,0x1f,0x1f,0x18,0x18,0x18,0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,
		0x18,0x18,0x18,0xff,0xff,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
		0xff,0x7f,0x3f,0x1f,0x0f,0x07,0x03,0x01,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,
		0x33,0x33,0xcc,0xcc,0x33,0x33,0xcc,0xcc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,
		0xff,0xff,0xff,0xff,0x33,0x33,0xcc,0xcc,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,
		0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xe7,0xe7,0xe7,0xe0,0xe0,0xe7,0xe7,0xe7,
		0xff,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xe7,0xe7,0xe7,0xe0,0xe0,0xff,0xff,0xff,
		0xff,0xff,0xff,0x07,0x07,0xe7,0xe7,0xe7,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,
		0x33,0x33,0xcc,0xcc,0x33,0x33,0xcc,0xcc,0x33,0x99,0xcc,0x66,0x33,0x99,0xcc,0x66,
		0xcc,0xcc,0x33,0x33,0xcc,0xcc,0x33,0x33,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
		0x18,0x18,0x18,0xf8,0xf8,0x18,0x18,0x18,0x00,0x00,0x00,0xff,0xff,0x18,0x18,0x18,
		0xe7,0xe7,0xe7,0x07,0x07,0xe7,0xe7,0xe7,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
		0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,
		0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xff,0xff,0xff,0xff,0x0f,0x0f,0x0f,0x0f,
		0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xff,0xe7,0xe7,0xe7,0x07,0x07,0xff,0xff,0xff,
		0x0f,0x0f,0x0f,0x0f,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0xf8,0xf8,0x18,0x18,0x18,
		0x18,0x18,0x18,0x1f,0x1f,0x00,0x00,0x00,0xc0,0xc0,0x30,0x30,0xc0,0xc0,0x30,0x30,
		0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,
		0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x34,
		0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
		0x18,0x18,0x1c,0x0f,0x07,0x00,0x00,0x00,0x18,0x18,0x38,0xf0,0xe0,0x00,0x00,0x00,
		0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
		0xff,0xff,0x03,0x03,0x03,0x03,0x03,0x03,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
		0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x07,0x0f,0x1c,0x18,0x18,
		0xc3,0xe7,0x7e,0x3c,0x3c,0x7e,0xe7,0xc3,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
		0x00,0x00,0x00,0x00,0xcc,0xcc,0x33,0x33,0x18,0x18,0x18,0xf8,0xf8,0x00,0x00,0x00,
		0x00,0x00,0x00,0x1f,0x1f,0x18,0x18,0x18,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
		0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
		0x00,0x00,0x03,0x3e,0x76,0x36,0x36,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
		0xcc,0xcc,0x33,0x33,0xcc,0xcc,0x33,0x33,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
		0x00,0x00,0x00,0x00,0xcc,0xcc,0x33,0x33,0xcc,0x99,0x33,0x66,0xcc,0x99,0x33,0x66,
		0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x18,0x18,0x18,0x1f,0x1f,0x18,0x18,0x18,
		0x00,0x00,0x00,0x00,0x0f,0x0f,0x0f,0x0f,0x18,0x18,0x18,0x1f,0x1f,0x00,0x00,0x00,
		0x00,0x00,0x00,0xf8,0xf8,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
		0x00,0x00,0x00,0x1f,0x1f,0x18,0x18,0x18,0x18,0x18,0x18,0xff,0xff,0x00,0x00,0x00,
		0x00,0x00,0x00,0xff,0xff,0x18,0x18,0x18,0x18,0x18,0x18,0xf8,0xf8,0x18,0x18,0x18,
		0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
		0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
		0x03,0x03,0x03,0x03,0x03,0x03,0xff,0xff,0x00,0x00,0x00,0x00,0xf0,0xf0,0xf0,0xf0,
		0x0f,0x0f,0x0f,0x0f,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0xf8,0xf8,0x00,0x00,0x00,
		0xf0,0xf0,0xf0,0xf0,0x00,0x00,0x00,0x00,0xf0,0xf0,0xf0,0xf0,0x0f,0x0f,0x0f,0x0f,
	};

	unsigned char*	the_new_font_data = testfont;

	ShowDescription("Text_UpdateFontData -> Change the font characters with 2K of data from the specified buffer. See screen A as comparison.");	
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_A], 10);
	Text_ShowFontChars(global_system->screen_[ID_CHANNEL_B], 10);
	Text_UpdateFontData(global_system->screen_[ID_CHANNEL_B], (char*)the_new_font_data);

	WaitForUser();
}




void RunDemo(void)
{
	Text_FillCharMem(global_system->screen_[ID_CHANNEL_B], ' ');
	Text_FillAttrMem(global_system->screen_[ID_CHANNEL_B], 160);

	ShowDescription("Welcome to the Text Library Demo!");	
	WaitForUser();
	
	Demo_Text_CopyMemBox1();
	Demo_Text_CopyMemBox2();
	
	Demo_Text_FillCharMem1();
	Demo_Text_FillCharMem2();
	
	Demo_Text_FillAttrMem1();
	Demo_Text_FillAttrMem2();
	
	Demo_Text_FillBoxSlow1();
	Demo_Text_FillBoxSlow2();
	
	Demo_Text_FillBox1();
	Demo_Text_FillBox2();
	Demo_Text_FillBox3();

	Demo_Text_InvertBox();
	
	Demo_Text_ShowFontChars();

	Demo_Text_SetCharAndColorAtXY();

	Demo_Text_DrawHLine1();
	Demo_Text_DrawHLine2();
	Demo_Text_DrawHLine3();
	
	Demo_Text_NamedColors();

	Demo_Text_DrawBox();
	Demo_Text_DrawBoxCoords();
	Demo_Text_DrawBoxCoordsFancy();

	Demo_Text_DrawStringAtXY();
	Demo_Text_DrawStringInBox1();
	Demo_Text_DrawStringInBox2();

	Demo_Text_ScreenResolution1();
	Demo_Text_ScreenResolution2();
	
	// 2 other resolutions, but only if this is a GenX or A2560 machine
	#ifndef _C256_FMX_
		Demo_Text_ScreenResolution3();
		Demo_Text_ScreenResolution4();
	#endif

	Demo_Text_UpdateFontData();
	
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/




int main(int argc, char* argv[])
{
	DEBUG_OUT(("%s %d: starting text demo...", __func__, __LINE__));
	
	if (Sys_InitSystem() == false)
	{
		DEBUG_OUT(("%s %d: Couldn't initialize the system", __func__, __LINE__));
		//exit(0); // maybe this isn't doing anything on FMX?
		return 0;
	}

	RunDemo();

	return 0;
}