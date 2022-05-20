/*
 * menu.c
 *
 *  Created on: May 12, 2022
 *      Author: micahbly
 */



/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/


// project includes
#include "menu.h"

// A2560 includes
#include <mcp/syscalls.h>
#include "lib_sys.h"
#include "font.h"
#include "general.h"
#include "window.h"

// C includes
#include <stdio.h>
#include <stdlib.h>



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

//! Analyzes the current menu_group, calculates longest width, total height, changes menu overall and global rects, and draws contents in non-selected style
void Menu_LayoutMenu(Menu* the_menu);

//! Convert the passed x, y global coordinates to local (to menu) coordinates
void Menu_GlobalToLocal(Menu* the_menu, int16_t* x, int16_t* y);

//! Find the menu item at the passed location
//! @param x:	Local horizontal coordinate already confirmed to be within the bounds of the menu
//! @param y:	Local vertical coordinate already confirmed to be within the bounds of the menu
//! @return:	Returns MENU_NOTHING_HIGHLIGHTED if no menu item was at the location specified, or if the location is over a divider. Otherwise, returns the index to the selected menu item
int16_t Menu_FindSelectionFromXY(Menu* the_menu, int16_t x, int16_t y);

//! Draw the background and text of the passed menu item, using either unselected or selected style
void Menu_DrawOneMenuItem(Menu* the_menu, int16_t selection_index, bool as_selected);

//! Copy the passed rectangle to the menu's clip rect collection
//! NOTE: the incoming rect must be using menu-local coordinates, not global. No translation will be performed.
bool Menu_AddClipRect(Menu* the_menu, Rectangle* new_rect);

//! Blit each clip rect to the screen, and clear all clip rects when done
//! This is the actual mechanics of rendering the menu to the screen
bool Menu_BlitClipRects(Menu* the_menu);



/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

//! Analyzes the current menu_group, calculates longest width, total height, changes menu overall and global rects, and draws contents in non-selected style
void Menu_LayoutMenu(Menu* the_menu)
{
	MenuGroup*	the_menu_group;
	Theme*		the_theme;
	int16_t		max_width = 0;
	int16_t		i;
	uint8_t		back_color;
	uint8_t		fore_color;
	uint8_t		outline_color;
	int16_t		available_width;
	int16_t		available_height;
	int16_t		row_height;
	int16_t		back_row_height;
	Font*		the_font;
	int16_t		chars_that_fit;
	int16_t		pixels_used;
	MenuItem*	back_menu;

	// LOGIC:
	//   this is called when a menu is first generated for a given menu group
	//   in order to render the menu, we need to know how long the longest menu item text is, in pixels
	//   we also need to know how tall all of the menu items are, in total, as pixels
	//   there is a margin of a few pixels around the text, as well. on all sides. 
	//   we could use the font measure function to first measure each menu item title, then size the menu, then draw, 
	//     but this is not necessary: the menu's bitmap is already sized to the max menu size...
	//     we can just draw the menu text, and keep track of whichever was longest.
	//   when a menu displays a submenu, it needs to allow extra space at the top for a < back item
	//     back_row_height will cover this. if used, it will allow also a little extra whitespace under the back.
	
	the_menu_group = the_menu->menu_group_;
	the_font = Bitmap_GetFont(the_menu->bitmap_);
	row_height = the_font->fRectHeight;
	
	// clear bitmap in prep for drawing new titles
	the_theme = Sys_GetTheme(global_system);
	fore_color = Theme_GetMenuForeColor(the_theme);
	back_color = Theme_GetMenuBackColor(the_theme);
	outline_color = Theme_GetOutlineColor(the_theme);
	Bitmap_FillMemory(the_menu->bitmap_, back_color);
	Bitmap_SetColor(the_menu->bitmap_, fore_color);
	
	available_width = MENU_MAX_WIDTH - MENU_MARGIN - MENU_MARGIN;
	available_height = MENU_MAX_HEIGHT - MENU_MARGIN - MENU_MARGIN;

	if (the_menu_group->is_submenu_)
	{
		// LOGIC:
		//   Submenus need a way to get back to the previous menu group
		//   This is done by creating a "< back" type menu item at the top of the list
		//   The ID of this menu item is set to match the ID of the menu_group's parent_id. 
		//   The menu item is then inserted into the list of menu_items, at the end

		back_menu = the_menu_group->item_[the_menu_group->num_menu_items_]; // one past the current end... don't increase count until all the normal items are rendered below
		back_menu->id_ = the_menu_group->parent_id_;
		back_menu->text_ = the_menu_group->title_;
		
		back_row_height = the_font->fRectHeight + 5;

		chars_that_fit = Font_MeasureStringWidth(the_font, back_menu->text_, GEN_NO_STRLEN_CAP, available_width, 0, &pixels_used);
		DEBUG_OUT(("%s %d: available_width=%i, chars_that_fit=%i, text='%s', pixels_used=%i", __func__, __LINE__, available_width, chars_that_fit, back_menu->text_, pixels_used));

		Bitmap_SetXY(the_menu->bitmap_, MENU_MARGIN + MENU_TEXT_PADDING, MENU_MARGIN);

		if (Font_DrawString(the_menu->bitmap_, back_menu->text_, chars_that_fit) == false)
		{
		}
	}
	else
	{
		back_row_height = 0;
	}
	
	for (i = 0; i < the_menu_group->num_menu_items_; i ++)
	{
		MenuItem*	this_menu_item;
		int16_t		this_width;
		
		this_menu_item = the_menu_group->item_[i];
		
		if (this_menu_item->type_ != menuDivider)
		{

			chars_that_fit = Font_MeasureStringWidth(the_font, this_menu_item->text_, GEN_NO_STRLEN_CAP, available_width, 0, &pixels_used);
			DEBUG_OUT(("%s %d: available_width=%i, chars_that_fit=%i, text='%s', pixels_used=%i", __func__, __LINE__, available_width, chars_that_fit, this_menu_item->text_, pixels_used));
	
			Bitmap_SetXY(the_menu->bitmap_, MENU_MARGIN + MENU_TEXT_PADDING, MENU_MARGIN + back_row_height + (row_height * i));

			if (Font_DrawString(the_menu->bitmap_, this_menu_item->text_, chars_that_fit) == false)
			{
			}

			max_width = (pixels_used > max_width) ? pixels_used : max_width;
		}
		else
		{
			// skip drawing divider on this pass, because we don't know width of menu yet
		}
	}
	
	// we need to allow for space for shortcut key(s), ">", and also not have highlighting start/stop right at the edge of the text pixels
	max_width += MENU_SHORTCUT_SPACE + MENU_TEXT_PADDING + MENU_TEXT_PADDING;
	
	// we now have max width, can set up final size of menu box
	the_menu->inner_width_ = max_width;
	the_menu->inner_height_ = back_row_height + row_height * i;
	the_menu->width_ = MENU_MARGIN + the_menu->inner_width_ + MENU_MARGIN;
	the_menu->height_ = MENU_MARGIN + the_menu->inner_height_ + MENU_MARGIN;
	the_menu->overall_rect_.MinX = 0;
	the_menu->overall_rect_.MaxX = the_menu->width_ - 1;
	the_menu->overall_rect_.MinY = 0;
	the_menu->overall_rect_.MaxY = the_menu->height_ - 1;

	// only draw the border rect after width/height known
	Bitmap_DrawBoxRect(the_menu->bitmap_, &the_menu->overall_rect_, outline_color);	
	
	// now that we know actual width of menu, we can draw dividers and capture selection rects for all items
	for (i = 0; i < the_menu_group->num_menu_items_; i ++)
	{
		MenuItem*	this_menu_item;

		this_menu_item = the_menu_group->item_[i];
		
		if (this_menu_item->type_ == menuDivider)
		{
			// draw divider by using a simple h-line
			// TODO: investigate drawing menu divider in a different color, or maybe dotted?
			if (Bitmap_DrawHLine(the_menu->bitmap_, MENU_MARGIN + MENU_TEXT_PADDING, MENU_MARGIN + back_row_height + row_height * i + row_height/2, max_width - MENU_TEXT_PADDING - MENU_TEXT_PADDING, fore_color) == false)
			{
			}
		}
		else if (this_menu_item->type_ == menuSubmenu)
		{
			// draw the ">" char at far right			
			Bitmap_SetXY(the_menu->bitmap_, MENU_MARGIN + the_menu->inner_width_ - 1 - FONT_CHAR_MENU_RIGHT_WIDTH, MENU_MARGIN + back_row_height + (row_height * i));
			pixels_used = Font_DrawChar(the_menu->bitmap_, FONT_CHAR_MENU_RIGHT, the_font);
			
			DEBUG_OUT(("%s %d: menu id %i was a submenu, > drawn at %i, %i; %i pixels used", __func__, __LINE__, this_menu_item->id_, MENU_MARGIN + the_menu->inner_width_ - 1 - FONT_CHAR_MENU_RIGHT_WIDTH, MENU_MARGIN + (row_height * i)));
		}
		
		this_menu_item->selection_rect_.MinX = MENU_MARGIN;
		this_menu_item->selection_rect_.MinY = MENU_MARGIN + back_row_height + row_height * i;
		this_menu_item->selection_rect_.MaxX = MENU_MARGIN + the_menu->inner_width_ -1; // mouse selection will stop at the margins
		this_menu_item->selection_rect_.MaxY = this_menu_item->selection_rect_.MinY + row_height - 1;
		DEBUG_OUT(("%s %d: r1 (%i, %i : %i, %i)", __func__, __LINE__, this_menu_item->selection_rect_.MinX, this_menu_item->selection_rect_.MinY, this_menu_item->selection_rect_.MaxX, this_menu_item->selection_rect_.MaxY));
	}

	// now that the normal items are all set up, we can increment the count of items, and set up the selection rect for the "back" item (if any)
	if (the_menu_group->is_submenu_)
	{
		back_menu->selection_rect_.MinX = MENU_MARGIN;
		back_menu->selection_rect_.MinY = MENU_MARGIN;
		back_menu->selection_rect_.MaxX = MENU_MARGIN + the_menu->inner_width_ -1; // mouse selection will stop at the margins
		back_menu->selection_rect_.MaxY = back_menu->selection_rect_.MinY + row_height - 1;
		
		the_menu_group->num_menu_items_++;
	}
	
	the_menu->invalidated_ = true;
}


//! Convert the passed x, y global coordinates to local (to menu) coordinates
void Menu_GlobalToLocal(Menu* the_menu, int16_t* x, int16_t* y)
{
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

		
	DEBUG_OUT(("%s %d: menu x/y = %i, %i", __func__, __LINE__, the_menu->x_, the_menu->y_));
	
	*x = *x - the_menu->x_;
	*y = *y - the_menu->y_;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Find the menu item at the passed location
//! @param x:	Local horizontal coordinate already confirmed to be within the bounds of the menu
//! @param y:	Local vertical coordinate already confirmed to be within the bounds of the menu
//! @return:	Returns MENU_NOTHING_HIGHLIGHTED if no menu item was at the location specified, or if the location is over a divider. Otherwise, returns the index to the selected menu item
int16_t Menu_FindSelectionFromXY(Menu* the_menu, int16_t x, int16_t y)
{
	// LOGIC:
	//   iterate through menu_item_[] array and check rect intersect
	
	int16_t		i;
	int16_t		menu_selection;
	MenuGroup*	the_menu_group;
	int16_t		count;
	
	the_menu_group = the_menu->menu_group_;
	count = the_menu_group->num_menu_items_;
	
	for (i = 0; i < count; i++)
	{
		MenuItem*	this_menu_item;
		
		this_menu_item = the_menu_group->item_[i];
		
		if (this_menu_item->type_ != menuDivider)
		{
			if (General_PointInRect(x, y, this_menu_item->selection_rect_) == true)
			{
				return i;
			}
		}
	}
	
	return MENU_NOTHING_HIGHLIGHTED;
}


//! Draw the background and text of the passed menu item, using either unselected or selected style
void Menu_DrawOneMenuItem(Menu* the_menu, int16_t selection_index, bool as_selected)
{
	Theme*		the_theme;
	Font*		the_font;
	MenuGroup*	the_menu_group;
	MenuItem*	the_menu_item;
	int16_t		chars_that_fit;
	int16_t		pixels_used;
	uint8_t		back_color;
	uint8_t		fore_color;
	
	the_menu_group = the_menu->menu_group_;

	the_menu_item = the_menu_group->item_[selection_index];

	DEBUG_OUT(("%s %d: text='%s', selection_index=%i, as_selected=%i, count=%i", __func__, __LINE__, the_menu_item->text_, selection_index, as_selected, the_menu_group->num_menu_items_));
		
	// Need the theme to be able to get current highlight/standard back and fore colors	
	the_theme = Sys_GetTheme(global_system);
	
	if (as_selected)
	{
		fore_color = Theme_GetHighlightForeColor(the_theme);
		back_color = Theme_GetHighlightBackColor(the_theme);
	}
	else
	{
		fore_color = Theme_GetMenuForeColor(the_theme);
		back_color = Theme_GetMenuBackColor(the_theme);
	}

	Bitmap_FillBoxRect(the_menu->bitmap_, &the_menu_item->selection_rect_, back_color);
	Bitmap_SetXY(the_menu->bitmap_, the_menu_item->selection_rect_.MinX + MENU_TEXT_PADDING, the_menu_item->selection_rect_.MinY);
	Bitmap_SetColor(the_menu->bitmap_, fore_color);

	the_font = Bitmap_GetFont(the_menu->bitmap_);
	chars_that_fit = Font_MeasureStringWidth(the_font, the_menu_item->text_, GEN_NO_STRLEN_CAP, the_menu->inner_width_, 0, &pixels_used);
	DEBUG_OUT(("%s %d: available_width=%i, chars_that_fit=%i, text='%s', pixels_used=%i", __func__, __LINE__, the_menu->inner_width_, chars_that_fit, the_menu_item->text_, pixels_used));

	if (Font_DrawString(the_menu->bitmap_, the_menu_item->text_, chars_that_fit) == false)
	{
	}

	if (the_menu_item->type_ == menuSubmenu)
	{
		// draw the ">" char at far right
		Bitmap_SetXY(the_menu->bitmap_, MENU_MARGIN + the_menu->inner_width_ - 1 - FONT_CHAR_MENU_RIGHT_WIDTH, the_menu_item->selection_rect_.MinY);
		pixels_used = Font_DrawChar(the_menu->bitmap_, FONT_CHAR_MENU_RIGHT, the_font);
		
		DEBUG_OUT(("%s %d: menu id %i was a submenu, > drawn at %i, %i; %i pixels used", __func__, __LINE__, the_menu_item->id_, MENU_MARGIN + the_menu->inner_width_ - 1 - FONT_CHAR_MENU_RIGHT_WIDTH, the_menu_item->selection_rect_.MinY));
	}
	
	Menu_AddClipRect(the_menu, &the_menu_item->selection_rect_);
}


//! Copy the passed rectangle to the menu's clip rect collection
//! NOTE: the incoming rect must be using menu-local coordinates, not global. No translation will be performed.
bool Menu_AddClipRect(Menu* the_menu, Rectangle* new_rect)
{
	Rectangle*	the_clip;
	
	// LOGIC:
	//   A menu has a maximum number of clip rects it will track - 2
	//     it will only ever need to redraw a previously menu item and newly selected one
	
	if ( new_rect == NULL)
	{
		LOG_ERR(("%s %d: passed rect was null", __func__ , __LINE__));
		goto error;
	}
	
	if ( the_menu->clip_count_ >= 2)
	{
		return false;
	}
	
	the_clip = &the_menu->clip_rect_[the_menu->clip_count_];

	General_CopyRect(the_clip, new_rect);	
	
	the_menu->clip_count_++;

	//DEBUG_OUT(("%s %d: menu picked up a clip rect, now has %i cliprects; new clip rect is %i, %i : %i, %i", __func__, __LINE__, the_menu->clip_count_, the_clip->MinX, the_clip->MinY, the_clip->MaxX, the_clip->MaxY));
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


//! Blit each clip rect to the screen, and clear all clip rects when done
//! This is the actual mechanics of rendering the menu to the screen
bool Menu_BlitClipRects(Menu* the_menu)
{
	Rectangle*	the_clip;
	Bitmap*		the_screen_bitmap;
	int16_t		i;
	
	if ( the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_menu->clip_count_ == 0)
	{
		return true; // not an error condition
	}
	
	the_screen_bitmap = Sys_GetScreenBitmap(global_system, back_layer);
	
	for (i = 0; i < the_menu->clip_count_; i++)
	{
		the_clip = &the_menu->clip_rect_[i];

		DEBUG_OUT(("%s %d: menu blitting cliprect %p (%i, %i -- %i, %i)", __func__, __LINE__, the_clip, the_clip->MinX, the_clip->MinY, the_clip->MaxX, the_clip->MaxY));
	
		Bitmap_Blit(the_menu->bitmap_, 
					the_clip->MinX, 
					the_clip->MinY, 
					the_screen_bitmap, 
					the_clip->MinX + the_menu->x_, 
					the_clip->MinY + the_menu->y_, 
					the_clip->MaxX - the_clip->MinX + 1, 
					the_clip->MaxY - the_clip->MinY + 1
					);
	}
	
	// LOGIC: 
	//   clip rects are one-time usage: once we have blitted them, we never want to blit them again
	//   we want to clear the decks for the next set of updates
	
	the_menu->clip_count_ = 0;
	
	return true;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return false;
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/

// **** CONSTRUCTOR AND DESTRUCTOR *****


// constructor
// allocates space for the object, copies in the passed key string and sets a dummy string value
Menu* Menu_New(void)
{
	Menu*	the_menu;

	if ( (the_menu = (Menu*)calloc(1, sizeof(Menu)) ) == NULL)
	{
		LOG_ERR(("Menu_New: could not allocate memory to create new Menu object."));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_menu	%p	size	%i", __func__ , __LINE__, the_menu, sizeof(Menu)));

	if ( (the_menu->bitmap_ = Bitmap_New(MENU_MAX_WIDTH, MENU_MAX_HEIGHT, Sys_GetAppFont(global_system), PARAM_NOT_IN_VRAM)) == NULL)
	{
		LOG_ERR(("%s %d: Failed to create bitmap", __func__, __LINE__));
		goto error;
	}

	the_menu->x_ = 0;
	the_menu->y_ = 0;
	the_menu->width_ = MENU_MAX_WIDTH;
	the_menu->height_ = MENU_MAX_HEIGHT;
	the_menu->inner_width_ = the_menu->width_ - MENU_MARGIN - MENU_MARGIN;
	the_menu->inner_height_ = the_menu->height_ - MENU_MARGIN - MENU_MARGIN;
	
	the_menu->overall_rect_.MinX = 0;
	the_menu->overall_rect_.MinY = 0;
	the_menu->overall_rect_.MaxX = MENU_MAX_WIDTH - 1;
	the_menu->overall_rect_.MaxY = MENU_MAX_HEIGHT - 1;

	the_menu->global_rect_.MinX = the_menu->x_;
	the_menu->global_rect_.MaxX = (the_menu->x_ + the_menu->width_) - 1;
	the_menu->global_rect_.MinY = the_menu->y_;
	the_menu->global_rect_.MaxY = (the_menu->y_ + the_menu->height_) - 1;

	the_menu->pen_x_ = 0;
	the_menu->pen_y_ = 0;
	the_menu->pen_font_ = Sys_GetAppFont(global_system);
	Bitmap_SetFont(the_menu->bitmap_, Sys_GetAppFont(global_system));

	the_menu->clip_count_ = 0;

	the_menu->invalidated_ = true;
	the_menu->visible_ = false;
	the_menu->current_selection_ = MENU_NOTHING_HIGHLIGHTED;

	return the_menu;
	
error:
	if (the_menu) Menu_Destroy(&the_menu);
	Sys_Destroy(&global_system);	// crash early, crash often
	return NULL;
}


// destructor
void Menu_Destroy(Menu** the_menu)
{
	if (*the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	if ((*the_menu)->bitmap_)
	{
		Bitmap_Destroy(&(*the_menu)->bitmap_);
	}
	
	LOG_ALLOC(("%s %d:	__FREE__	*the_menu	%p	size	%i", __func__ , __LINE__, *the_menu, sizeof(Menu)));
	free(*the_menu);
	*the_menu = NULL;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Renders the menu and blits entire menu or required cliprects to the screen
void Menu_Render(Menu* the_menu)
{
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_menu->visible_ == false)
	{
		return;
	}
	
	// if the entire window has had to be redrawn, then don't bother with individual cliprects, just do one for entire window
	if (the_menu->invalidated_ == true)
	{
		the_menu->clip_count_ = 0;
		Bitmap_BlitRect(the_menu->bitmap_, the_menu->overall_rect_, Sys_GetScreenBitmap(global_system, back_layer), the_menu->x_, the_menu->y_);
		the_menu->invalidated_ = false;
	}
	else
	{
		DEBUG_OUT(("%s %d: menu has %i clip rects to render", __func__, __LINE__, the_menu->clip_count_));
		
		Menu_BlitClipRects(the_menu);
		// TODO: ^^^ above function is needed here, essentially unchanged. move that instead to bitmap or some other class, and take a pointer to an array of cliprects, and count of cliprects?
	}
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Replaces current menu_group_ reference with a new one, calculates widths, rects, and renders the menu at an appropriate place based on x, y passed and size of menu
//! NOTE: this sets mouse mode to mouseMenuOpen
void Menu_Open(Menu* the_menu, MenuGroup* the_menu_group, int16_t x, int16_t y)
{
	EventManager*	the_event_manager;
	
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	if (the_menu_group == NULL)
	{
		LOG_ERR(("%s %d: passed menu group was null", __func__ , __LINE__));
		goto error;
	}
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	the_event_manager = Sys_GetEventManager(global_system);
	Mouse_SetMode(the_event_manager->mouse_tracker_, mouseMenuOpen);
	
	the_menu->menu_group_ = the_menu_group;
	Menu_LayoutMenu(the_menu);
	
	// extract to private function
	{
		Screen*		the_screen;
		the_screen = Sys_GetScreen(global_system, ID_CHANNEL_B);
		
		if (x + the_menu->width_ > the_screen->width_)
		{
			x = the_screen->width_ - the_menu->width_;
		}

		DEBUG_OUT(("%s %d: menu x/y = %i, %i, incoming x,y=%i, %i, m hgt=%i, scr hgt=%i", __func__, __LINE__, the_menu->x_, the_menu->y_, x, y, the_menu->height_, the_screen->height_));
		
		if (y + the_menu->height_ > the_screen->height_)
		{
			y = the_screen->height_ - the_menu->height_;
		}
		
		the_menu->x_ = x;
		the_menu->y_ = y;
		the_menu->global_rect_.MinX = x;
		the_menu->global_rect_.MaxX = x + the_menu->width_ - 1;
		the_menu->global_rect_.MinY = y;
		the_menu->global_rect_.MaxY = y + the_menu->height_ - 1;
		
		DEBUG_OUT(("%s %d: menu x/y = %i, %i", __func__, __LINE__, the_menu->x_, the_menu->y_));
	}
	//
	
	the_menu->current_selection_ = MENU_NOTHING_HIGHLIGHTED;
	
	Menu_SetVisible(the_menu, true);
	Menu_Render(the_menu);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Cancels the opening of a menu before it is shown
//! NOTE: this sets mouse mode back to mouseFree
void Menu_CancelOpen(Menu* the_menu)
{
	EventManager*	the_event_manager;
	
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	the_event_manager = Sys_GetEventManager(global_system);	
	Mouse_SetMode(the_event_manager->mouse_tracker_, mouseFree);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Hides the menu by damaging and distributing damage rects to all other windows, and re-rendering screen
void Menu_Hide(Menu* the_menu)
{
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_menu->current_selection_ = MENU_NOTHING_HIGHLIGHTED;
	
	Menu_SetVisible(the_menu, false);

// 	* Needs to generate damage rects and distribute to windows under it. All windows are under it. 
// 	* This raises question: should the menu actually be a special kind of window, owned by the system?
// 		* If it was, the system could call Window_GenerateDamageRects(the_window, &the_new_rect) then Sys_IssueDamageRects().  just like other windows when they close. 
// 		* Could also use Window_draw text, window_DrawRect, etc. 
// 		* OTOH, there are other ways to do that: 
// 			* Have copy of Sys_IssueDamageRects slightly modified
// 			* When closing a window, get a copy of its rect, then destroy it, then call Sys_IssueDamageRects(), passing it the old window’s rect. That would work then for both regular windows and a rect from the menu. 

	Sys_IssueMenuDamageRects(global_system);

	// Re-render all windows
	Sys_Render(global_system);		
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Set the menu's visibility flag.
//! This does not immediately cause the menu to render. The menu will be rendered on the next rendering pass.
void Menu_SetVisible(Menu* the_menu, bool is_visible)
{
	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}
	
	the_menu->visible_ = is_visible;
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}


//! Accept a right or left mouse click while a menu is open, identify which, if any, menu item should be selected as a result
//! @param x:	Global horizontal coordinate when the mouse was clicked
//! @param y:	Global vertical coordinate when the mouse was clicked
//! @return:	Returns MENU_ID_NO_SELECTION if no menu item was under the mouse. Returns ID of menu item if one was selected.
int16_t	Menu_AcceptClick(Menu* the_menu, int16_t x, int16_t y)
{
	int16_t		local_x;
	int16_t		local_y;
	int16_t		selection_index;

	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	// * examine click loc >
	// 	* In any case: call Menu_Hide() 
	// 	* If not on menu, return MENU_ID_NO_SELECTION
	// 	* If on menu, iterate through menu_item_[] array and check rect intersect.
	// 		* If a match, return the id_ of the selected item
	// 		* If not match, return MENU_ID_NO_SELECTION

	local_x = x;
	local_y = y;
	Menu_GlobalToLocal(the_menu, &local_x, &local_y);

	// no matter what the result, we want to hide the menu
	Menu_Hide(the_menu);
	
	if (local_x < 0 || local_x > the_menu->global_rect_.MaxX || local_y < 0 || local_y > the_menu->global_rect_.MaxY)
	{
		return MENU_ID_NO_SELECTION;
	}
	
	selection_index = Menu_FindSelectionFromXY(the_menu, local_x, local_y);
	
	if (selection_index != MENU_NOTHING_HIGHLIGHTED)
	{
		return the_menu->menu_group_->item_[selection_index]->id_;
	}
	
	return MENU_ID_NO_SELECTION;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return MENU_ID_NO_SELECTION;
}


//! Accept a new mouse x/y while a menu is open, identify which, if any, menu item should be shown as selected
//! @param x:	Global horizontal coordinate of current mouse loc
//! @param y:	Global vertical coordinate of current mouse loc
void Menu_AcceptMouseMove(Menu* the_menu, int16_t x, int16_t y)
{
	int16_t		local_x;
	int16_t		local_y;
	int16_t		selection_index;

	if (the_menu == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		goto error;
	}

	// LOGIC:
	//   Match passed x/y against the internal rects, and determine which item should be highlighted, if any.
	//   If coords were over a highlightable menu item (dividers are not highlightable):
	//     Paint that rect in highlight color, redraw text in white/menu color, and adds a clip rect for that
	//     If a different item had been highlighted, draw that again in normal style, and add a cliprect for that. 

	local_x = x;
	local_y = y;
	Menu_GlobalToLocal(the_menu, &local_x, &local_y);

	DEBUG_OUT(("%s %d: passed x,y (%i, %i); local x,y (%i, %i)", __func__, __LINE__, x, y, local_x, local_y));

	// if mouse loc is outside bounds of menu, we need to unselect whatever was previously selected, if anything
	if (local_x < 0 || local_x > the_menu->global_rect_.MaxX || local_y < 0 || local_y > the_menu->global_rect_.MaxY)
	{
		if (the_menu->current_selection_ != MENU_NOTHING_HIGHLIGHTED)
		{
			Menu_DrawOneMenuItem(the_menu, the_menu->current_selection_, MENU_PARAM_SHOW_NORMAL);
			Menu_Render(the_menu);
		}
		
		return;
	}
	
	selection_index = Menu_FindSelectionFromXY(the_menu, local_x, local_y);
	DEBUG_OUT(("%s %d: selection_index=%i, the_menu->current_selection_=%i", __func__, __LINE__, selection_index, the_menu->current_selection_));
	
	if (selection_index == the_menu->current_selection_)
	{
		return;
	}
	
	if (the_menu->current_selection_ != MENU_NOTHING_HIGHLIGHTED)
	{
		Menu_DrawOneMenuItem(the_menu, the_menu->current_selection_, MENU_PARAM_SHOW_NORMAL);
	}
	
	Menu_DrawOneMenuItem(the_menu, selection_index, MENU_PARAM_SHOW_HIGHLIGHTED);
	the_menu->current_selection_ = selection_index;
	Menu_Render(the_menu);
	
	return;
	
error:
	Sys_Destroy(&global_system);	// crash early, crash often
	return;
}