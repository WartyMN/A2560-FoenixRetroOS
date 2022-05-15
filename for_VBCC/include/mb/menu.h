//! @file menu.h

/*
 * menu.h
 *
 *  Created on: May 12, 2022
 *      Author: micahbly
 */

#ifndef MENU_H_
#define MENU_H_


/* about this class
 *
 * code to manage all aspects of menus
 *
 * Will be used for:
 *
 *
 *** things a list needs to be able to do
 * create/build menus at app startup
 * determine which menu item has been selected by the user
 * execute or route actions for each menu item
 * 
 *
*/


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

#include "a2560_platform.h"
#include "event.h"



/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define MENU_MAX_ITEMS				30	//! Maximum number of items that can be part of any one given menu
#define MENU_MAX_WIDTH				300	//! Maximum horizontal pixels allowed for a Menu display
#define MENU_MAX_HEIGHT				350	//! Maximum vertical pixels allowed for a Menu display
#define MENU_MARGIN					5	//! Margin between any outer edge of a Menu, and contents. Applied left, right, top, and bottom.
#define MENU_TEXT_PADDING			2	//! Leading and trailing space between menu margin and text. Prevents selection highlighting from starting at first pixel of text.
#define MENU_SHORTCUT_SPACE			40	//! Fixed width space reserved for use with keyboard shortcuts on right side of menu. Will also be used by ">" symbol for submenus.

#define MENU_ID_NO_PARENT			-1	//! For the parent_id_ field, a value indicating the menu item does not have a parent menu item set (yet)
#define MENU_ID_DIVIDER				-2	//! For the id_ field, a value indicating the menu item is a divider. Not 100% required, but may improve readability of code, and reinforces fact that dividers are never acted upon, so their ID value is pointless. Do not rely on this to determine if a menu item is a divider: use the menu_item_type enum value of menuDivider!
#define MENU_ID_NO_SELECTION		-3	//! On a mouse click when menu is open, but user clicks on a menu header or other non-clickable item, this value is returned
#define MENU_NOTHING_HIGHLIGHTED	-1	//! On a mouse move when menu is open, if no highlightable object is under the mouse, this value is set for menu->current_selection_

#define MENU_PARAM_SHOW_HIGHLIGHTED	true	//! for menu functions that render menu items, the value that will cause the item to be rendered as selected/highlighted
#define MENU_PARAM_SHOW_NORMAL		false	//! for menu functions that render menu items, the value that will cause the item to be rendered with a normal/unselected background and foreground

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum menu_item_type
{
	menuItem 				= 0,
	menuSubmenu 			= 1,
	menuDivider 			= 2,
} menu_item_type;

// typedef enum menu_level
// {
// 	menu_no_menu			= -1,
// 	menu_level_0			= 0,
// 	menu_level_1 			= 1,
// 	menu_level_2 			= 2,
// 	menu_level_3 			= 3,
// } menu_level;

// typedef enum menu_modifiers
// {
// 	foenixKeyBit			= 1,	// foenix key down?
// 	shiftKeyBit				= 2,	// shift key down?
// 	optionKeyBit			= 3,	// option key down?
// 	controlKeyBit			= 4,	// control key down?
// } menu_modifiers;
// 
// typedef enum menu_modifier_flags
// {
// 	foenixKey				= 1 << foenixKeyBit,
// 	shiftKey				= 1 << shiftKeyBit,
// 	optionKey				= 1 << optionKeyBit,
// 	controlKey				= 1 << controlKeyBit,
// } menu_modifier_flags;

/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


struct MenuItem
{
	int16_t					id_;
	char*					text_;					//! the label/text of the menu item
	event_modifier_flags	modifiers_;				//! none, or some/all of (foenixKey|shiftKey|optionKey|controlKey)
	unsigned char			shortcut_;				//! the shortcut key. Must be typeable or user won't be able to use it.
	menu_item_type			type_;					//! the type of menu object: a submenu, a menu item, or a divider
	Rectangle				selection_rect_;		//! the local (to menu) rect describing the area the user would click in to select the menu item. Populated by the system as menus are built.
};

struct MenuGroup
{
	int16_t					id_;
	char*					title_;					//! displayed at top of menu panel, and in the 'back' part of a child menu
	struct MenuItem*		item_[MENU_MAX_ITEMS];
	int16_t					num_menu_items_;		//! of the total possible menu items defined by MENU_MAX_ITEMS, for this menu, how many are currently used. -1 if none.
	bool					is_submenu_;			// if this menu group is a submenu, it will get a < back item at the top of the menu.
	int16_t					parent_id_;				//! the id_ of the parent menu group, if any. Will be assigned by the system to a pseudo menu item that provides a "back" functionality. 
};

struct Menu
{
	Bitmap* 				bitmap_;						// bitmap in standard memory, to hold the rendered menu. Will be blitted to the screen.
	int16_t					pen_x_;							// Local H position relative to the overall_rect_, of the "pen", for drawing functions
	int16_t					pen_y_;							// Local V position relative to the overall_rect_, of the "pen", for drawing functions
	uint8_t					pen_color_;						// Color index of the "pen", for drawing functions
	Font*					pen_font_;						// Font to be used by the "pen", for drawing functions
	Rectangle				overall_rect_;					// the local rect describing the total area of the menu
	Rectangle				global_rect_;					// the global rect describing the total area of the menu
	int16_t					x_;								// current global horizontal coordinate
	int16_t					y_;								// current global vertical coordinate
	int16_t					width_;							// current width of menu
	int16_t					height_;						// current height of menu
	int16_t					inner_width_;					// space available inside the menu, accounting for margin thicknesses
	int16_t					inner_height_;					// space available inside the menu, accounting for margin thicknesses
	Rectangle				clip_rect_[2];					// one or more clipping rects; determines which parts of menu need to be re-blitted to the screen
	int16_t					clip_count_;					// number of clip rects the menu is currently tracking
	MenuGroup*				menu_group_;					// pointer to the menu group currently being displayed
	bool					invalidated_;					// if true, the menu needs to be completely re-rendered on the next render pass
	bool					visible_;						// is the menu active/visible, or not?
	int16_t					current_selection_;				// index to menu_group_->item_[]. Updated during mouse move. Indicates which one of the rows is currently highlighted, if any. -1 if none.
};


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/

// constructor
// allocates space for the object, copies in the passed key string and sets a dummy string value
Menu* Menu_New(void);

// destructor
void Menu_Destroy(Menu** the_menu);


//! Renders the menu and blits entire menu or required cliprects to the screen
void Menu_Render(Menu* the_menu);

//! Replaces current menu_group_ reference with a new one, calculates widths, rects, and renders the menu at an appropriate place based on x, y passed and size of menu
void Menu_Open(Menu* the_menu, MenuGroup* the_menu_group, int16_t x, int16_t y);

//! Cancels the opening of a menu before it is shown
//! NOTE: this sets mouse mode back to mouseFree
void Menu_CancelOpen(Menu* the_menu);

//! Hides the menu by damaging and distributing damage rects to all other windows, and re-rendering screen
void Menu_Hide(Menu* the_menu);

//! Set the menu's visibility flag.
//! This does not immediately cause the menu to render. The menu will be rendered on the next rendering pass.
void Menu_SetVisible(Menu* the_menu, bool is_visible);


//! Accept a right or left mouse click while a menu is open, identify which, if any, menu item should be selected as a result
//! @param x:	Global horizontal coordinate when the mouse was clicked
//! @param y:	Global vertical coordinate when the mouse was clicked
//! @return:	Returns MENU_ID_NO_SELECTION if no menu item was under the mouse. Returns ID of menu item if one was selected.
int16_t	Menu_AcceptClick(Menu* the_menu, int16_t x, int16_t y);

//! Accept a new mouse x/y while a menu is open, identify which, if any, menu item should be shown as selected
//! @param x:	Global horizontal coordinate of current mouse loc
//! @param y:	Global vertical coordinate of current mouse loc
void Menu_AcceptMouseMove(Menu* the_menu, int16_t x, int16_t y);




#endif /* MENU_H_ */
