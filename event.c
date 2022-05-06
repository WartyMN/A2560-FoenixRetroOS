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
#include "a2560_platform.h"
#include "general.h"
#include "bitmap.h"
#include "text.h"
#include "font.h"
#include "window.h"


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
	
	if ( (the_event = (EventRecord*)calloc(1, sizeof(EventRecord)) ) == NULL)
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
	free(*the_event);
	*the_event = NULL;
	
	return true;
}


// constructor
//! Allocate an EventManager object
EventManager* EventManager_New(void)
{
	EventManager*	the_event_manager;
	
	if ( (the_event_manager = (EventManager*)calloc(1, sizeof(EventManager)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new EventManager", __func__ , __LINE__));
		goto error;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_event_manager	%p	size	%i", __func__ , __LINE__, the_event_manager, sizeof(EventManager)));

	the_event_manager->write_idx_ = 0;
	the_event_manager->read_idx_ = 0;
	the_event_manager->mouse_tracker_->mode_ = mouseFree;

	// get a mouse tracker
	if ( (the_event_manager->mouse_tracker_ = Mouse_New()) == NULL)
	{
		LOG_ERR(("%s %d: Out of memory when creating mouse tracker", __func__ , __LINE__));
		goto error;
	}
	//LOG_ALLOC(("%s %d:	__ALLOC__	the_event_manager->mouse_tracker_	%p	", __func__ , __LINE__, the_event_manager->mouse_tracker_));

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
	free(*the_event_manager);
	*the_event_manager = NULL;
	
	return true;
}



// **** Queue Management functions *****

//! Nulls out any events associated with the window pointer passed
//! Call this when a window has been closed, to ensure that there are not future events that will try to recall the window after it is destroyed
void EventManager_RemoveEventsForWindow(Window* the_window)
{
	EventManager*	the_event_manager;
	EventRecord*	the_event;
	uint16_t		start;
	uint16_t		stop;
	uint16_t		i;
	
	// LOGIC:
	//   the event buffer is circular. nullEvents are allowed and present.
	//   so the way to know if there is a waiting event is to compare the read and write indices
	//   if read=write, then there are no pending events
	
	the_event_manager = Sys_GetEventManager(global_system);
	
// 	if (the_event_manager->read_idx_ == the_event_manager->write_idx_)
// 	{
// 		DEBUG_OUT(("%s %d: read_idx_=%i SAME AS write_idx_=%i", __func__, __LINE__, the_event_manager->read_idx_, the_event_manager->write_idx_));
// 		return;
// 	}
	
	start = the_event_manager->read_idx_;
	stop = the_event_manager->write_idx_;
	DEBUG_OUT(("%s %d: read_idx_=%i, write_idx_=%i, window=%p", __func__, __LINE__, the_event_manager->read_idx_, the_event_manager->write_idx_, the_window));
	
	for (i = start; i <= stop; i++)
	{
		the_event = the_event_manager->queue_[i];

		DEBUG_OUT(("%s %d: this event: window=%p, readidx=%i", __func__, __LINE__, the_event->window_, i));

		if (the_event->window_ != NULL)
		{
			if (the_event->window_ == the_window)
			{
				Event_SetNull(the_event);
			}
		}
	}
	
	return;
}


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
		DEBUG_OUT(("%s %d: read_idx_=%i SAME AS write_idx_=%i", __func__, __LINE__, the_event_manager->read_idx_, the_event_manager->write_idx_));
		return NULL;
	}
	
	the_event = the_event_manager->queue_[the_event_manager->read_idx_];

	DEBUG_OUT(("%s %d: Next Event: type=%i", __func__, __LINE__, the_event->what_));
	//EventManager_Print(the_event_manager);
	Event_Print(the_event);
	
	the_event_manager->read_idx_++;
	the_event_manager->read_idx_ %= EVENT_QUEUE_SIZE;
	
	if (the_event->what_ == nullEvent)
	{
		DEBUG_OUT(("%s %d: event was null. read_idx_=%i, write_idx_=%i", __func__, __LINE__, the_event_manager->read_idx_, the_event_manager->write_idx_));
		return NULL;
	}

	DEBUG_OUT(("%s %d: read_idx_=%i, read_idx_ mod EVENT_QUEUE_SIZE=%i", __func__, __LINE__, the_event_manager->read_idx_, the_event_manager->read_idx_ % EVENT_QUEUE_SIZE));
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
		}
	}
	else if (the_event->window_ == NULL)
	{
		the_event->window_ = Sys_GetActiveWindow(global_system);
	}	
}


//! Wait for an event to happen, do system-processing of it, then if appropriate, give the window responsible for the event a chance to do something with it
void EventManager_WaitForEvent(void)
{
	EventManager*	the_event_manager;
	EventRecord*	the_event;
	
	the_event_manager = Sys_GetEventManager(global_system);
	
	//DEBUG_OUT(("%s %d: write_idx_=%i, read_idx_=%i, the_mask=%x", __func__, __LINE__, the_event_manager->write_idx_, the_event_manager->read_idx_, the_mask));

	// TESTING: if no events in queue, add one to prime pump
// 	if (the_event_manager->write_idx_ == the_event_manager->read_idx_)
// 	{
// 		EventManager_AddEvent(keyDown, 65, -1, -1, 0L, NULL, NULL);
// 		EventManager_AddEvent(keyUp, 65, -1, -1, 0L, NULL, NULL);
// 		EventManager_GenerateRandomEvent();
// 	}
	
	// now process the queue as if it were happening in real time
// 	
// 	the_event = EventManager_NextEvent();
// 	DEBUG_OUT(("%s %d: first event: %i", __func__, __LINE__, the_event->what_));
// 	

	while ( (the_event = EventManager_NextEvent()) != NULL)
	{
		int16_t			local_x;
		int16_t			local_y;
		MouseMode		starting_mode;
		Window*			the_window;
		Window*			the_active_window;
		int16_t			x_delta;
		int16_t			y_delta;
		int16_t			prev_mouse_x;
		int16_t			prev_mouse_y;
		
		DEBUG_OUT(("%s %d: Received Event Event: type=%i", __func__, __LINE__, the_event->what_));
		Event_Print(the_event);
		
		starting_mode = Mouse_GetMode(the_event_manager->mouse_tracker_);
		
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
		
			case mouseMoved:
				
				DEBUG_OUT(("%s %d: mouse move event (%i, %i)", __func__, __LINE__, the_event->x_, the_event->y_));
	
				// LOGIC: 
				//   what happens here depends on what the current event manager mouse mode is
				//   if mouseFree: no real action.
				//   if mouseDownOnControl: no real action
				//   if mouseResizeXXX: draw window bounding box based on mouse x/y
				//   if mouseDragTitle: draw window bounding box based on mouse x/y
				//   if mouseLassoInProgress: draw lasso box
				//   if other, then no action

				the_window = the_event->window_;
				
				// update the mouse so it knows it's X/Y, but first capture what it was last time, so we can see what movement was made since last move (not since last click)
				prev_mouse_x = Mouse_GetX(the_event_manager->mouse_tracker_);
				prev_mouse_y = Mouse_GetY(the_event_manager->mouse_tracker_);
				Mouse_SetXY(the_event_manager->mouse_tracker_, the_event->x_, the_event->y_);
	
				// get the delta between current and last clicked position
				x_delta = Mouse_GetXDelta(the_event_manager->mouse_tracker_);
				y_delta = Mouse_GetYDelta(the_event_manager->mouse_tracker_);
			
				DEBUG_OUT(("%s %d: mouse clicked (%i, %i)", __func__, __LINE__, Mouse_GetClickedX(the_event_manager->mouse_tracker_), Mouse_GetClickedY(the_event_manager->mouse_tracker_)));
				DEBUG_OUT(("%s %d: mouse now (%i, %i)", __func__, __LINE__, Mouse_GetX(the_event_manager->mouse_tracker_), Mouse_GetY(the_event_manager->mouse_tracker_)));
				DEBUG_OUT(("%s %d: mouse delta (%i, %i)", __func__, __LINE__, Mouse_GetXDelta(the_event_manager->mouse_tracker_), Mouse_GetYDelta(the_event_manager->mouse_tracker_)));
			
				if (starting_mode == mouseDragTitle)
				{
					// for a drag rect, we want to draw an outline same shape as the Window, in the foreground bitmap
					// as mouse moves, we want to undraw the previous rect, and draw the new one.
					// we need to know what the previous mouse X/Y was for that to work. (unless we just clear the whole foreground bitmap, but that seems dumb expensive)
					// we will store every mouse-move X/Y as proposed X/Y in the window, and use that to draw/refresh rect as needed
					// will also need proposed width/height, for size drags.
					
					if (the_event->window_ != NULL)
					{
						if (x_delta != 0 && y_delta != 0)
						{
							int16_t	new_x;
							int16_t	new_y;
							int16_t	new_width;
							int16_t	new_height;
							bool	change_made = false;
							
							DEBUG_OUT(("%s %d: window x/y (%i, %i)", __func__, __LINE__,  Window_GetX(the_window), Window_GetY(the_window)));
							
							new_x = Window_GetX(the_window) + x_delta;
							new_y = Window_GetY(the_window) + y_delta;
							new_width = Window_GetWidth(the_window);
							new_height = Window_GetHeight(the_window);

							//if (change_made)
							{
								Bitmap*		the_bitmap = Sys_GetScreenBitmap(global_system, back_layer);
						
								// undraw the old box, draw the new one. temporary problem: A2560 emulators are not currently doing composition, so we can't draw to foreground layer. 
								//Bitmap_DrawBox(the_bitmap, prev_x, prev_y, prev_width, prev_height, 0, PARAM_DO_NOT_FILL)
								Bitmap_DrawBox(the_bitmap, new_x, new_y, new_width, new_height, SYS_COLOR_RED1, PARAM_DO_NOT_FILL);						
							}
						}
					}					
				}
				else if (starting_mode >= mouseResizeUp) // this gets all the resize items
				{
					int16_t	new_x;
					int16_t	new_y;
					int16_t	new_width;
					int16_t	new_height;
					bool	change_made = false;
					
					new_x = Window_GetX(the_window);
					new_y = Window_GetY(the_window);
					new_width = Window_GetWidth(the_window);
					new_height = Window_GetHeight(the_window);

					if (starting_mode == mouseResizeLeft || starting_mode == mouseResizeRight)
					{
						if (x_delta != 0)
						{
							new_width += x_delta;
							
							if (starting_mode == mouseResizeLeft)
							{
								new_x += x_delta;
							}

							change_made = true;						
						}
					}
					else if (starting_mode == mouseResizeUp || starting_mode == mouseResizeDown)
					{
						if (y_delta != 0)
						{
							new_height += y_delta;
							
							if (starting_mode == mouseResizeUp)
							{
								new_y += y_delta;
							}

							change_made = true;
						}
					}
					else if (starting_mode == mouseResizeDownRight)
					{
						if (y_delta != 0)
						{
							new_width += x_delta;
							new_height += y_delta;
							change_made = true;
						}
					}

					DEBUG_OUT(("%s %d: mouse move in RESIZE evt; change_made=%i, new width/height=%i, %i", __func__, __LINE__, change_made, new_width, new_height));
					
					if (change_made)
					{
						Bitmap*		the_bitmap = Sys_GetScreenBitmap(global_system, back_layer);
						
						// undraw the old box, draw the new one. temporary problem: A2560 emulators are not currently doing composition, so we can't draw to foreground layer. 
						//Bitmap_DrawBox(the_bitmap, prev_x, prev_y, prev_width, prev_height, 0, PARAM_DO_NOT_FILL)
						Bitmap_DrawBox(the_bitmap, new_x, new_y, new_width, new_height, SYS_COLOR_GREEN1, PARAM_DO_NOT_FILL);						
					}
				}
				else
				{
					// give window an event
					(*the_window->event_handler_)(the_event);				
				}
								
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

				// get the delta between current and last clicked position
				x_delta = Mouse_GetXDelta(the_event_manager->mouse_tracker_);
				y_delta = Mouse_GetYDelta(the_event_manager->mouse_tracker_);
				
				// update the mouse tracker so that if we end up dragging, we'll know where the original click was. (or if a future double click, what time the click was, etc.)
				Mouse_AcceptUpdate(the_event_manager->mouse_tracker_, the_event->x_, the_event->y_, true);

				// get local coords so we can check for drag and lasso
				local_x = the_event->x_;
				local_y = the_event->y_;
				Window_GlobalToLocal(the_window, &local_x, &local_y);
				
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
				}
				else
				{
					// LOGIC:
					//   A mouse click on the active window could be start of a drag action (if in appropriate place), 
					//     or it could be the start of a click on a Control
					//     if the start of a drag action, no event will be sent to the window until mouse up (end of drag)
					//     if the click is on a control, no event will be sent to the window until mouse up (and mouse is still on control). 
					//       the control itself, however, will get a <selected> call.

					the_event->window_ = the_window;
					
					// check for a control that needs activating
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
						//(*the_window->event_handler_)(the_event);
						
						Mouse_SetMode(the_event_manager->mouse_tracker_, mouseDownOnControl);
					}
				}
				
				// LOGIC:
				//   regardless of what window was clicked, we have to update mouse mode
				//   if mouse is in title bar, it could be start of a drag action to move the window.
				//   same for window resize widgets
				//   we do not want to activate controls yet though. (TODO: revisit this decision if it seems weird from HF perspective)
				//   first task is to determine if the point the mouse clicked on is in a draggable region.
				//   if this is active window, if not draggable region, it could be a lassoable region.
				
				// check for click in a draggable region
				MouseMode	possible_drag_mode;
				
				possible_drag_mode = Window_CheckForDragZone(the_event->window_, local_x, local_y);
				
				if (possible_drag_mode != mouseFree)
				{
					Mouse_SetMode(the_event_manager->mouse_tracker_, possible_drag_mode);
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
				
				// no matter what, reset the mouse history position flags
				Mouse_AcceptUpdate(the_event_manager->mouse_tracker_, the_event->x_, the_event->y_, false);

				// get the delta between current and last clicked position
				x_delta = Mouse_GetXDelta(the_event_manager->mouse_tracker_);
				y_delta = Mouse_GetYDelta(the_event_manager->mouse_tracker_);
				DEBUG_OUT(("%s %d: mouse delta at mouse up was %i, %i!", __func__, __LINE__, x_delta, y_delta));
				
				// LOGIC:
				//   Based on what the mode had been before mouse button up, we take different actions
				//   If mouseDragTitle > tell window to move to new coordinates
				//   if mouseResizeXXX > tell window to accept new size
				//   if mouseDownOnControl > create a control clicked event and send to user Window
				//   (fill in the rest)
				
				// If this is mouseDragTitle mode, tell window to move to new coordinates
				if (starting_mode == mouseDragTitle)
				{					
					DEBUG_OUT(("%s %d: mouse up from mouseDragTitle: move window '%s'!", __func__, __LINE__, the_window->title_));
					
					if (x_delta != 0 && y_delta != 0)
					{
						int16_t	new_x;
						int16_t	new_y;
						int16_t	new_width;
						int16_t	new_height;
						int32_t	the_code;
						
						new_x = Window_GetX(the_window) + x_delta;
						new_y = Window_GetY(the_window) + y_delta;
						new_width = Window_GetWidth(the_window);
						new_height = Window_GetHeight(the_window);
						the_code = (new_width << 16) + new_height;
						
						DEBUG_OUT(("%s %d: adding movewindow evt with %i, %i", __func__, __LINE__, new_x, new_y));
						
						EventManager_AddEvent(windowChanged, the_code, new_x, new_y, 0L, the_window, NULL);
					}
				}
				else if (starting_mode >= mouseResizeUp) // this gets all the resize items
				{
					int16_t	new_x;
					int16_t	new_y;
					int16_t	new_width;
					int16_t	new_height;
					int32_t	the_code;
					bool	change_made = false;
					
					new_x = Window_GetX(the_window);
					new_y = Window_GetY(the_window);
					new_width = Window_GetWidth(the_window);
					new_height = Window_GetHeight(the_window);

					DEBUG_OUT(("%s %d: mouse up from mouseResizeXXX: resize window '%s'!", __func__, __LINE__, the_window->title_));
					
					if (starting_mode == mouseResizeLeft || starting_mode == mouseResizeRight)
					{
						if (x_delta != 0)
						{
							new_width += x_delta;
							
							if (starting_mode == mouseResizeLeft)
							{
								new_x += x_delta;
							}

							change_made = true;						
						}
					}
					else if (starting_mode == mouseResizeUp || starting_mode == mouseResizeDown)
					{
						if (y_delta != 0)
						{
							new_height += y_delta;
							
							if (starting_mode == mouseResizeUp)
							{
								new_y += y_delta;
							}

							change_made = true;
						}
					}
					else if (starting_mode == mouseResizeDownRight)
					{
						if (y_delta != 0)
						{
							new_width += x_delta;
							new_height += y_delta;
							change_made = true;
						}
					}

					if (change_made)
					{
						the_code = (new_width << 16) + new_height;
						EventManager_AddEvent(windowChanged, the_code, new_x, new_y, 0L, the_window, NULL);
					}
				}
				else if (starting_mode == mouseDownOnControl)
				{
					// check for a control that was pressed, and is now released = it got clicked
					local_x = the_event->x_;
					local_y = the_event->y_;
					Window_GlobalToLocal(the_event->window_, &local_x, &local_y);
				
					DEBUG_OUT(("%s %d: mouse up from mouseDownOnControl: fire off a control click in window '%s'!", __func__, __LINE__, the_window->title_));
					
					the_event->control_ = Window_GetControlAtXY(the_event->window_, local_x, local_y);
				
					if (the_event->control_)
					{
						if (Control_GetPressed(the_event->control_))
						{
							DEBUG_OUT(("%s %d: ** control '%s' (id=%i) was down, now up!", __func__, __LINE__, the_event->control_->caption_, the_event->control_->id_));
							//Control_SetPressed(the_event->control_, false);
							EventManager_AddEvent(controlClicked, -1, the_event->x_, the_event->y_, 0L, the_event->window_, the_event->control_);
						}
						else
						{
							// a control was clicked on, button not let up until mouse left that control, and was placed over another
							// first control needs to be unselected.
							// second control does NOT need any action. should stay unselected/unpushed.
							
							// clear any selected control without setting another to selected.
							Window_SetSelectedControl(the_event->window_, NULL);
						}
					}
					else
					{
						// situation is SOME control was down. user let up mouse, but had moved mouse off of a control. 
						// should the window get a mouse up event in that case? What would it do with that? 
						// give window an event
						//(*the_window->event_handler_)(the_event);
						
						// either way, need to unselect whatever control had been clicked on mouse-down
						Window_SetSelectedControl(the_event->window_, NULL);
					}
				}
				
				Mouse_SetMode(the_event_manager->mouse_tracker_, mouseFree);
				
				break;

			case controlClicked:
				DEBUG_OUT(("%s %d: control clicked event: %c", __func__, __LINE__, the_event->code_));
				// give window an event
				(*the_window->event_handler_)(the_event);				
		
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

			case windowChanged:
				DEBUG_OUT(("%s %d: windowChanged event", __func__, __LINE__));

				// give window an event
				(*the_event->window_->event_handler_)(the_event);				

				break;
			
			default:
				DEBUG_OUT(("%s %d: other event: %i", __func__, __LINE__, the_event->what_));
				
				break;
		}
		
		//DEBUG_OUT(("%s %d: r idx=%i, w idx=%i, meets_mask will be=%x", __func__, __LINE__, the_event_manager->write_idx_, the_event_manager->read_idx_, the_event->what_ & the_mask));
		
		getchar();		
	}
	
	return;
}



