//! @file event.h

/*
 * event.h
 *
*  Created on: Apr 16, 2022
 *      Author: micahbly
 */

#ifndef EVENT_H_
#define EVENT_H_


/* about this class: Event Manager
 *
 * Provides structures and functions for queueing events

 * NOTE: Event structures and style are largely based on Apple's old (pre-OS X) events.h
 *   I have adapted for Foenix realities, and for my style, and added a couple of conveniences
 *     A couple of conveniences added in style of Amiga Intuition: the window and control are available directly from the event record
 *   There is no expectation that somebody's old mac code would work
 *   however, it should be familiar in feel to anyone who programmed macs before OS X

 *
 *** things this class needs to be able to do
 * Provide interrupt handlers that turn mouse and keyboard actions into events
 * Provide a global event queue that apps can access
 * 
 *
 * STRETCH GOALS
 * 
 *
 * SUPER STRETCH GOALS
 * 
 * 
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes


// C includes
#include <stdbool.h>


// A2560 includes
#include <mcp/syscalls.h>
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
#include <mb/bitmap.h>
#include <mb/window.h>
#include <mb/lib_sys.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define EVENT_QUEUE_SIZE	256		//! number of event records in the circular buffer


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


typedef enum event_kind 
{
	nullEvent				= 0,
	mouseDown				= 1,
	mouseUp					= 2,
	keyDown					= 3,
	keyUp					= 4,
	autoKey					= 5,
	updateEvt				= 6,
	diskEvt					= 7,
	activateEvt				= 8,
	inactivateEvt			= 9,
	rMouseDown				= 10,
	rMouseUp				= 11,
	menuOpened				= 12,
	menuSelected			= 13,
	menuCanceled			= 14,
	controlClicked			= 15,
} event_kind;


typedef enum event_mask 
{
	mouseDownMask			= 1 << mouseDown,		// mouse button pressed
	mouseUpMask				= 1 << mouseUp,			// mouse button released
	keyDownMask				= 1 << keyDown,			// key pressed
	keyUpMask				= 1 << keyUp,			// key released
	autoKeyMask				= 1 << autoKey,			// key repeatedly held down
	updateMask				= 1 << updateEvt,		// window needs updating
	diskEvtMask				= 1 << diskEvt,			// disk inserted
	activateEvtMask			= 1 << activateEvt,		// activate window
	inactivateEvtMask		= 1 << inactivateEvt,	// deactivate window
	rMouseDownMask			= 1 << rMouseDown,		// right mouse button pressed
	rMouseUpMask			= 1 << rMouseUp,		// right mouse button released
	menuOpenedMask			= 1 << menuOpened,		// contextual menu opened
	menuSelectedMask		= 1 << menuSelected,	// item from the contextual menu selected
	menuCanceledMask		= 1 << menuCanceled,	// contextual menu closed without an item being selected
	controlClickedMask		= 1 << controlClicked,	// a clickable (2 state) control has been clicked
	everyEvent				= 0xFFFF				// all of the above
} event_mask;


typedef enum event_modifiers
{
	activeFlagBit			= 0,    // activate? (activateEvt and mouseDown)
	btnStateBit				= 7,    // state of button?
	foenixKeyBit			= 8,    // foenix key down?
	shiftKeyBit				= 9,    // shift key down?
	alphaLockBit			= 10,   // alpha lock down?
	optionKeyBit			= 11,   // option key down?
	controlKeyBit			= 12,   // control key down?
	rightShiftKeyBit		= 13,   // right shift key down?
	rightOptionKeyBit		= 14,   // right Option key down?
	rightControlKeyBit		= 15    // right Control key down?
} event_modifiers;

typedef enum event_modifier_flags
{
	activeFlag				= 1 << activeFlagBit,
	btnState				= 1 << btnStateBit,
	foenixKey				= 1 << foenixKeyBit,
	shiftKey				= 1 << shiftKeyBit,
	alphaLock				= 1 << alphaLockBit,
	optionKey				= 1 << optionKeyBit,
	controlKey				= 1 << controlKeyBit,
	rightShiftKey			= 1 << rightShiftKeyBit,
	rightOptionKey			= 1 << rightOptionKeyBit,
	rightControlKey			= 1 << rightControlKeyBit
} event_modifier_flags;


// TODO: localize this for A2560
enum
{
	kNullCharCode                 = 0,
	kHomeCharCode                 = 1,
	kEnterCharCode                = 3,
	kEndCharCode                  = 4,
	kHelpCharCode                 = 5,
	kBellCharCode                 = 7,
	kBackspaceCharCode            = 8,
	kTabCharCode                  = 9,
	kLineFeedCharCode             = 10,
	kVerticalTabCharCode          = 11,
	kPageUpCharCode               = 11,
	kFormFeedCharCode             = 12,
	kPageDownCharCode             = 12,
	kReturnCharCode               = 13,
	kFunctionKeyCharCode          = 16,
	kEscapeCharCode               = 27,
	kClearCharCode                = 27,
	kLeftArrowCharCode            = 28,
	kRightArrowCharCode           = 29,
	kUpArrowCharCode              = 30,
	kDownArrowCharCode            = 31,
	kDeleteCharCode               = 127,
	kNonBreakingSpaceCharCode     = 202
};

struct EventRecord
{
	event_kind			what_;
	uint32_t			code_;		//! set for keydown, keyup: the key code
	uint32_t			when_;		//! ticks
	Window*				window_;	//! not set for a diskEvt
	Control*			control_;	//! not set for every event type. if not set on mouseDown/Up, pointer was not over a control
	int16_t				x_;			//! global X coordinate. Only set for mouse events.
	int16_t				y_;			//! global Y coordinate. Only set for mouse events.
	event_modifiers		modifiers_;	//! set for keyboard and mouse events
};

struct EventManager
{
	EventRecord*		queue_[EVENT_QUEUE_SIZE];	//! circular buffer for the event queue
	uint16_t			write_idx_;					//! index to queue_: where the next event record will be slotted
	uint16_t			read_idx_;					//! index to queue_: where the next event record will be read from
};




/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// **** events are pre-created in a fixed size array on system startup (circular buffer)
// **** as interrupts need to add more events, they take the next slot available in the array

// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate an EventManager object
EventRecord* Event_New(void);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Event_Destroy(EventRecord** the_event);



// constructor
//! Allocate an EventManager object
EventManager* EventManager_New(void);

// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool EventManager_Destroy(EventManager** the_event_manager);




// **** Queue Management functions *****

//! Checks to see if there is an event in the queue
//! returns NULL if no event (not the same as returning an event of type nullEvent)
EventRecord* EventManager_NextEvent(void);

//! Add a new event to the event queue
//! NOTE: this does not actually insert a new record, as the event queue is a circular buffer
//! It overwrites whatever slot is next in line
//! @param	the_window: this may be set for non-mouse up/down events. For mouse up/down events, it will not be set, and X/Y will be used to find the window.
void EventManager_AddEvent(event_kind the_what, uint32_t the_code, int16_t x, int16_t y, event_modifiers the_modifiers, Window* the_window, Control* the_control);

//! Wait for an event to happen, do system-processing of it, then give the event to the call function
EventRecord* EventManager_WaitForEvent(event_mask the_mask);





// **** Debug functions *****

void Event_Print(EventRecord* the_event);
void EventManager_Print(EventManager* the_event_manager);



#endif /* EVENT_H_ */


