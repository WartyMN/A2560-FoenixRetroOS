/*
 * event.c
 *
 *  Created on: Apr 16, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "event.h"

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
#include <mb/memory_manager.h>


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


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

//! Make the passed event a nullEvent, blanking out all fields
static void Event_SetNull(EventRecord* the_event);

// **** DEBUG/TESTING Functions

// create one random event in simulation of an interrupt activity
void EventManager_GenerateRandomEvent(void);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


//! Make the passed event a nullEvent, blanking out all fields
static void Event_SetNull(EventRecord* the_event)
{
	the_event->what_ = nullEvent;
	the_event->code_ = 0L;
	the_event->when_ = 0L;
	the_event->window_ = NULL;
	the_event->control_ = NULL;
	the_event->x_ = -1;
	the_event->y_ = -1;
	the_event->modifiers_ = 0;
}

// **** Debug functions *****

void Event_Print(EventRecord* the_event)
{
	DEBUG_OUT(("EventRecord print out: (%p)", the_event));
	DEBUG_OUT(("  what_: %i", the_event->what_));
	DEBUG_OUT(("  code_: %i", the_event->code_));
	DEBUG_OUT(("  when_: %i", the_event->when_));
	if (the_event->window_ != NULL)
	{
		DEBUG_OUT(("  window_: %p ('%s')", the_event->window_, the_event->window_->title_));
	}
	else
	{
		DEBUG_OUT(("  window_: (NULL)"));
	}
	DEBUG_OUT(("  control_: %p", the_event->control_));
	DEBUG_OUT(("  x_: %i", the_event->x_));
	DEBUG_OUT(("  y_: %i", the_event->y_));
	DEBUG_OUT(("  modifiers_: %x", the_event->modifiers_));
}

void EventManager_Print(EventManager* the_event_manager)
{
	DEBUG_OUT(("EventManager print out:"));
	DEBUG_OUT(("  queue size: %u", EVENT_QUEUE_SIZE));
	DEBUG_OUT(("  queue_: %p", the_event_manager->queue_));
	DEBUG_OUT(("  write_idx_: %i", the_event_manager->write_idx_));
	DEBUG_OUT(("  read_idx_: %i", the_event_manager->read_idx_));
}




// **** DEBUG/TESTING Functions

// create one random event in simulation of an interrupt activity
void EventManager_GenerateRandomEvent(void)
{
	// LOGIC: 
	//   simulate having interrupts working, and doing an event loop
	//   because no interrupts (of mine) are working, will fake that. 
	
	event_kind	the_event_type;
	event_kind	max_event = mouseUp;
	int16_t		x;
	int16_t		y;
// 	uint8_t		x_var;
// 	uint8_t		y_var;
	uint8_t		char_code;
	
	the_event_type = (rand() * max_event) / RAND_MAX + 1;
	DEBUG_OUT(("%s %d: Generating event of type %i", __func__, __LINE__, the_event_type));
	
	switch (the_event_type)
	{
		case nullEvent:
			//EventManager_AddEvent(the_event_type, 0, -1, -1, 0L, NULL);
			break;
			
		case keyDown:
			char_code = (rand() * 256) / RAND_MAX;
			EventManager_AddEvent(the_event_type, char_code, -1, -1, 0L, NULL, NULL);
			EventManager_AddEvent(keyUp, char_code, -1, -1, 0L, NULL, NULL);
			break;
			
		case mouseDown:
		case mouseUp:
			// pick a random spot, do a mouse down, then pick a random range +/- 10 from there for mouse up
			x = (rand() * 640) / RAND_MAX;
			y = (rand() * 480) / RAND_MAX;
			EventManager_AddEvent(the_event_type, 0, x, y, 0L, NULL, NULL);
// 				x_var = (rand() * 20) / RAND_MAX;
// 				y_var = (rand() * 20) / RAND_MAX;
// 				EventManager_AddEvent(mouseUp, 0, x + x_var - 10, y + y_var - 10, 0L, NULL, NULL);
			break;
			
		default:
			break;
	}			
}

/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/


// **** CONSTRUCTOR AND DESTRUCTOR *****

// constructor
//! Allocate an EventManager object
EventRecord* Event_New(void)
{
	EventRecord*	the_event;
	
	if ( (the_event = (EventRecord*)f_calloc(1, sizeof(EventRecord), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new EventRecord", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_event	%p	size	%i", __func__ , __LINE__, the_event, sizeof(EventRecord)));

	Event_SetNull(the_event);
		
	return the_event;
	
error:
	if (the_event)					Event_Destroy(&the_event);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool Event_Destroy(EventRecord** the_event)
{
	if (*the_event == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	LOG_ALLOC(("%s %d:	__FREE__	*the_event	%p	size	%i", __func__ , __LINE__, *the_event, sizeof(EventRecord)));
	f_free(*the_event, MEM_STANDARD);
	*the_event = NULL;
	
	return true;
}


// constructor
//! Allocate an EventManager object
EventManager* EventManager_New(void)
{
	EventManager*	the_event_manager;
	
	if ( (the_event_manager = (EventManager*)f_calloc(1, sizeof(EventManager), MEM_STANDARD) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new EventManager", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_event_manager	%p	size	%i", __func__ , __LINE__, the_event_manager, sizeof(EventManager)));

	the_event_manager->write_idx_ = 0;
	the_event_manager->read_idx_ = 0;
	
	//DEBUG_OUT(("%s %d: EventManager (%p) created", __func__ , __LINE__, the_event_manager));
	
	int	i;
	
	for (i=0; i < EVENT_QUEUE_SIZE; i++)
	{
		if ( (the_event_manager->queue_[i] = Event_New()) == NULL)
		{
			LOG_ERR(("%s %d: could not create event record #%i", __func__ , __LINE__, i));
			goto error;
		}
		
		//DEBUG_OUT(("%s %d: Event %i (%p)(%p) created", __func__ , __LINE__, i, the_event, the_event_manager->queue_[i]));
	}
	
	return the_event_manager;
	
error:
	if (the_event_manager)					EventManager_Destroy(&the_event_manager);
	return NULL;
}


// destructor
// frees all allocated memory associated with the passed object, and the object itself
bool EventManager_Destroy(EventManager** the_event_manager)
{
	int16_t	i;
	
	if (*the_event_manager == NULL)
	{
		LOG_ERR(("%s %d: passed class object was null", __func__ , __LINE__));
		return false;
	}

	for (i=0; i < EVENT_QUEUE_SIZE; i++)
	{
		if ((*the_event_manager)->queue_[i] != NULL)
		{
			Event_Destroy(&(*the_event_manager)->queue_[i]);
		}
	}
	
	LOG_ALLOC(("%s %d:	__FREE__	*the_event_manager	%p	size	%i", __func__ , __LINE__, *the_event_manager, sizeof(EventManager)));
	f_free(*the_event_manager, MEM_STANDARD);
	*the_event_manager = NULL;
	
	return true;
}



// **** Queue Management functions *****

//! Checks to see if there is an event in the queue
//! returns NULL if no event (not the same as returning an event of type nullEvent)
EventRecord* EventManager_NextEvent(void)
{
	EventManager*	the_event_manager;
	EventRecord*	the_event;
	
	// LOGIC:
	//   the event buffer is circular. nullEvents are allowed and present.
	//   so the way to know if there is a waiting event is to compare the read and write indices
	//   if read=write, then there are no pending events
	
	the_event_manager = Sys_GetEventManager(global_system);
	
	if (the_event_manager->read_idx_ == the_event_manager->write_idx_)
	{
		return NULL;
	}
	
	the_event = the_event_manager->queue_[the_event_manager->read_idx_];

	//DEBUG_OUT(("%s %d: the_event_type=%i", __func__, __LINE__, the_event->what_));
	//EventManager_Print(the_event_manager);
	Event_Print(the_event);
	
// 	if (the_event->what_ == nullEvent)
// 	{
// 		return NULL;
// 	}
// 	
	the_event_manager->read_idx_++;
	the_event_manager->read_idx_ %= EVENT_QUEUE_SIZE;

	DEBUG_OUT(("%s %d: exiting; event what=%i (%p), read_idx_=%i, write_idx_=%i", __func__, __LINE__, the_event->what_, the_event, the_event_manager->read_idx_, the_event_manager->write_idx_));
	
	return the_event;
}


//! Add a new event to the event queue
//! NOTE: this does not actually insert a new record, as the event queue is a circular buffer
//! It overwrites whatever slot is next in line
//! @param	the_window: this may be set for non-mouse up/down events. For mouse up/down events, it will not be set, and X/Y will be used to find the window.
void EventManager_AddEvent(event_kind the_what, uint32_t the_code, int16_t x, int16_t y, event_modifiers the_modifiers, Window* the_window, Control* the_control)
{
	EventManager*	the_event_manager;
	EventRecord*	the_event;

	DEBUG_OUT(("%s %d: reached; the_what=%i, the_code=%i, x=%i, y=%i, the_window=%p", __func__, __LINE__, the_what, the_code, x, y, the_window));
	
	the_event_manager = Sys_GetEventManager(global_system);
	
	the_event = the_event_manager->queue_[the_event_manager->write_idx_++];
	the_event_manager->write_idx_ %= EVENT_QUEUE_SIZE;
	
	if (the_what == nullEvent)
	{
		DEBUG_OUT(("%s %d: null event added", __func__, __LINE__));
		Event_SetNull(the_event);
		return;
	}
	
	// LOGIC:
	//   Because an interrupt will (most likely) be the thing creating this event
	//   We accept only the minimum of info here and create the rest from that info
	the_event->what_ = the_what;
	the_event->code_ = the_code;
	the_event->when_ = sys_time_jiffies();
	the_event->x_ = x;
	the_event->y_ = y;
	the_event->modifiers_ = the_modifiers;
	the_event->window_ = the_window;
	the_event->control_ = the_control;
	
	// check for a window and a control, using x and y
	// LOGIC:
	//   do not try to find window for a disk event
	//   do not try to find window/control for a control clicked event, it will already have been set
	//   for mouseup/down find window based on cursor pos (if window not already set, which it might have been)
	//   for all other events, if window hadn't been set, set it to active window.
	
	if (the_what == diskEvt || the_what == controlClicked)
	{
		// twiddle thumbs
	}
	else if (the_what == mouseDown || the_what == mouseUp)
	{
		// might already have been set, if interrupt isn't what generated the event
		if (the_event->window_ == NULL)
		{
			the_event->window_ = Sys_GetWindowAtXY(global_system, x, y);
// 		
// 			if (the_event->window_ != NULL)
// 			{
// 				Window*		the_active_window;
		
				// tried adding the below here, but it doesn't work. have instead in some system-level event handler, before it passed control to app
			
	// 			//* Check if mouse is down in the active window, or in another window
	// 			//   * If not in active window, add 2 WindowActive events to the queue.
	// 			//     * Tell old front window it is now not active
	// 			//     * Make window the front most window
	// 			//    Cancel this add queue for hte mouse down, and add another AFTER the 2 activate/inactivate events.
	// 			
	// 			the_active_window = Sys_GetActiveWindow(global_system);
	// 			
	// 			if (the_event->window_ != the_active_window)
	// 			{
	// 				the_event->what_ = nullEvent; // cancels out this event, in effect
	// 				EventManager_AddEvent(inactivateEvt, -1, -1, -1, 0L, the_active_window);
	// 				EventManager_AddEvent(activateEvt, -1, -1, -1, 0L, the_event->window_);
	// 				EventManager_AddEvent(the_what, -1, x, y, 0L, NULL);
	// 				return;
	// 			}
			
				// skipp this in interrupt call, will do in WaitForEvent() instead.
	// 			the_event->control_ = Window_GetControlAtXY(the_event->window_, x, y);
// 			}
// 			else
// 			{
// 				the_event->control_ = NULL;
// 			}
		}
	}
	else if (the_event->window_ == NULL)
	{
		the_event->window_ = Sys_GetActiveWindow(global_system);
	}	
}


//! Wait for an event to happen, do system-processing of it, then if appropriate, give the window responsible for the event a chance to do something with it
void EventManager_WaitForEvent(event_mask the_mask)
{
	EventManager*	the_event_manager;
	EventRecord*	the_event;
	uint16_t		meets_mask = 0;
	
	the_event_manager = Sys_GetEventManager(global_system);
	
	DEBUG_OUT(("%s %d: write_idx_=%i, read_idx_=%i, the_mask=%x", __func__, __LINE__, the_event_manager->write_idx_, the_event_manager->read_idx_, the_mask));

	// TESTING: if no events in queue, add one to prime pump
	if (the_event_manager->write_idx_ == the_event_manager->read_idx_)
	{
		EventManager_AddEvent(keyDown, 65, -1, -1, 0L, NULL, NULL);
		EventManager_AddEvent(keyUp, 65, -1, -1, 0L, NULL, NULL);
		EventManager_GenerateRandomEvent();
	}
	
	// now process the queue as if it were happening in real time
// 	
// 	the_event = EventManager_NextEvent();
// 	DEBUG_OUT(("%s %d: first event: %i", __func__, __LINE__, the_event->what_));
// 	

	while ((the_event = EventManager_NextEvent()) != NULL && meets_mask == 0)
	{
		int16_t		local_x;
		int16_t		local_y;
		bool		skip_this_event = false;
		
		meets_mask = 0;
		
		Event_Print(the_event);
		
		Window*			the_window;
		Window*			the_active_window;
		
		// LOGIC:
		//   event could be for:
		//   1. a mouse event. Will sort out non-app window click vs main window click vs about window click in the specific handler
		//   2. an update event. need to detect and route for main window vs about window
		//   3. an activate event. need to detect and route for main window vs about window
		//   4. a keyboard event. need to check for menu shortcuts and activate menus. no other keyboard input needed.

		switch (the_event->what_)
		{
			case nullEvent:
				DEBUG_OUT(("%s %d: null event", __func__, __LINE__));
				break;
		
			case mouseDown:
				
				DEBUG_OUT(("%s %d: mouse down event (%i, %i)", __func__, __LINE__, the_event->x_, the_event->y_));

				//* Check if mouse is down in the active window, or in another window
				//   * If not in active window, add 2 WindowActive events to the queue.
				//     * Tell old front window it is now not active
				//     * Make window the front most window
				//    Cancel this add queue for hte mouse down, and add another AFTER the 2 activate/inactivate events.
			
				the_active_window = Sys_GetActiveWindow(global_system);
				the_window = Sys_GetWindowAtXY(global_system, the_event->x_, the_event->y_);

				if (the_window == NULL)
				{
					LOG_ERR(("%s %d: no window found at %i, %i!", __func__, __LINE__, the_event->x_, the_event->y_));
					Sys_Destroy(&global_system);
				}
				DEBUG_OUT(("%s %d: active window = '%s', clicked window = '%s'", __func__, __LINE__, the_active_window->title_, the_window->title_));
				
				if (the_window != the_active_window)
				{
					// LOGIC:
					//   Any mouse click on a window other than the active window causes change in active window. 
					//   The mouse click is consumed by the system in this case: the target window does not get an event (but it will get one later for activate)
					
					Sys_SetActiveWindow(global_system, the_window);
					DEBUG_OUT(("%s %d: **** changed active window to = '%s'; redrawing all windows", __func__, __LINE__, the_window->title_));

					EventManager_AddEvent(inactivateEvt, -1, -1, -1, 0L, the_active_window, NULL);
					EventManager_AddEvent(activateEvt, -1, -1, -1, 0L, the_window, NULL);

					//Sys_Render(global_system);
					//EventManager_AddEvent(mouseDown, -1, the_event->x_, the_event->y_, 0L, the_window, NULL); // add the mouse down event back in, AFTER the 2 window events.
					//skip_this_event = true;
// 				}
// 				else
// 				{
					// LOGIC:
					//   A mouse click on the active window could be start of a drag action (if in appropriate place), 
					//     or it could be the start of a click on a Control
					//     if the start of a drag action, no event will be sent to the window until mouse up (end of drag)

					the_event->window_ = the_window;
					
					// check for a control that needs activating
					local_x = the_event->x_;
					local_y = the_event->y_;
					Window_GlobalToLocal(the_event->window_, &local_x, &local_y);
				
					the_event->control_ = Window_GetControlAtXY(the_event->window_, local_x, local_y);
					
					// have control change it's visual state to pressed. that is system's responsibility, not app's. 
					// TODO: think about whether this should be limited to only controls that have 0/1 states?
					// TODO: think about whether the system should eat the mouseDown event if over a control
					if (the_event->control_)
					{
						DEBUG_OUT(("%s %d: ** control '%s' (id=%i) moused down!", __func__, __LINE__, the_event->control_->caption_, the_event->control_->id_));
						Window_SetSelectedControl(the_event->window_, the_event->control_);
						//Control_SetPressed(the_event->control_, true);
						// give window an event
						(*the_window->event_handler_)(the_event);
					}
				}
				
				break;

			case mouseUp:
				DEBUG_OUT(("%s %d: mouse up event (%i, %i)", __func__, __LINE__, the_event->x_, the_event->y_));

				//* Check if over a control. 
				//   * If yes, check if that control has state set to pressed
				//     * if yes, set the control's visual state to normal. add an event for control_clicked???
				//       * should we eat the mouse up event? YES
				//   * if no, give the window an event (why not?)
			
				the_active_window = Sys_GetActiveWindow(global_system);
				the_window = Sys_GetWindowAtXY(global_system, the_event->x_, the_event->y_);

				if (the_window == NULL)
				{
					LOG_ERR(("%s %d: no window found at %i, %i!", __func__, __LINE__, the_event->x_, the_event->y_));
					Sys_Destroy(&global_system);
				}
				DEBUG_OUT(("%s %d: active window = '%s', clicked window = '%s'", __func__, __LINE__, the_active_window->title_, the_window->title_));
				
				the_event->window_ = the_window;
				
				// check for a control that was pressed, and is now released = it got clicked
				local_x = the_event->x_;
				local_y = the_event->y_;
				Window_GlobalToLocal(the_event->window_, &local_x, &local_y);
				
				the_event->control_ = Window_GetControlAtXY(the_event->window_, local_x, local_y);
				
				if (the_event->control_)
				{
					if (Control_GetPressed(the_event->control_))
					{
						DEBUG_OUT(("%s %d: ** control '%s' (id=%i) was down, now up!", __func__, __LINE__, the_event->control_->caption_, the_event->control_->id_));
						//Control_SetPressed(the_event->control_, false);
						EventManager_AddEvent(controlClicked, -1, the_event->x_, the_event->y_, 0L, the_event->window_, the_event->control_);
						skip_this_event = true;					
					}
				}
				else
				{
					// give window an event
					(*the_window->event_handler_)(the_event);				
				}
				
				// either way, there should be no more selected control
				Window_SetSelectedControl(the_event->window_, NULL);
				
				break;
			
			case keyDown:
				DEBUG_OUT(("%s %d: key down event: %c", __func__, __LINE__, the_event->code_));

				// give active window an event
				the_active_window = Sys_GetActiveWindow(global_system);

				(*the_active_window->event_handler_)(the_event);				

				break;
			
			case keyUp:
				DEBUG_OUT(("%s %d: key up event: %c", __func__, __LINE__, the_event->code_));

				// give active window an event
				the_active_window = Sys_GetActiveWindow(global_system);

				(*the_active_window->event_handler_)(the_event);				

				break;

			case updateEvt:
				DEBUG_OUT(("%s %d: updateEvt event", __func__, __LINE__));

				// give window an event
				(*the_event->window_->event_handler_)(the_event);				

// 				// Get a RSS surface pointer from the message
// 				//  LOGIC: the_window will be 0 in some cases
// 				//    for activateEvt and updateEvt, message will be WindowPtr
// 				//    (see EventRecord)
// 			
// 				affected_app_window = (RSSWindow*)GetWRefCon((WindowPtr)the_event->message);
// 
// 				if (affected_app_window == global_app->main_window_)
// 				{
// 					the_surface = (RSSWindow*)affected_app_window;
// 					Window_HandleUpdateEvent(the_surface, the_event);
// 				}
// 				else
// 				{
// 					// maybe it was an About window that got the event? 	
// 					if (affected_app_window == global_app->about_window_)
// 					{
// 						the_about_window = (RSSAboutWindow*)affected_app_window;
// 						AboutWindow_HandleUpdateEvent(the_about_window, the_event);
// 					}
// 				}
// 			
// 				return kUIMsgTypeNonRelevant;
				break;
			
			case activateEvt:
				DEBUG_OUT(("%s %d: activateEvt event", __func__, __LINE__));

				// give window an event
				(*the_event->window_->event_handler_)(the_event);				

				
				// tell window to make its active_ state
				//Window_SetActive(the_event->window_, true); // now this is done as part of Sys_SetActiveWindow

				// do what, exactly? 
				//Sys_SetActiveWindow(global_system, the_event->window_); // don't set here, set in the mouse down thing that caused it
				//Sys_Render(global_system);

				//Sys_SetActiveWindow(global_system, the_event->window_);
				//DEBUG_OUT(("%s %d: **** changed active window to = '%s'; redrawing all windows", __func__, __LINE__, the_event->window_));
				
				
				// need to make system move the window to the front
				// but also need to give app a chance to get this activate event before system does that. 
				//Window_Activate(the_event->window_);
				
				break;
			
			case inactivateEvt:
				DEBUG_OUT(("%s %d: inactivateEvt event", __func__, __LINE__));
				// do what, exactly? 
				//Sys_SetActiveWindow(global_system, the_event->window_);
				
				// tell window to make its active_ state
				Window_SetActive(the_event->window_, false); // now this is done as part of Sys_SetActiveWindow
				//Sys_Render(global_system);

				// give window an event
				(*the_event->window_->event_handler_)(the_event);				

				
				// need to make system move the window to the front
				// but also need to give app a chance to get this activate event before system does that. 
				//Window_Inactivate(the_event->window_);
				
				break;
			
			default:
				DEBUG_OUT(("%s %d: other event: %i", __func__, __LINE__, the_event->what_));
				
				break;
		}
		
		DEBUG_OUT(("%s %d: r idx=%i, w idx=%i, meets_mask will be=%x", __func__, __LINE__, the_event_manager->write_idx_, the_event_manager->read_idx_, the_event->what_ & the_mask));
		
		if (skip_this_event == false)
		{
			meets_mask = the_event->what_ & the_mask;
			
		}
		DEBUG_OUT(("%s %d: the_mask=%x; skip=%i, meets_mask=%i", __func__, __LINE__, the_mask, skip_this_event, meets_mask));
		
		if (meets_mask > 0)
		{
			// TEST: generate another random event for next loop
			EventManager_GenerateRandomEvent();
			EventManager_GenerateRandomEvent();
			// TEST ****************
		}		
	}
	
	return;
}



