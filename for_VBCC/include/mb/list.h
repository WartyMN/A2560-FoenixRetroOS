/*
 * list.h
 *
 *  Created on: Oct 28, 2020
 *      Author: micahbly
 *
 * 		Adaptation for inclusion in A2560 OS/f: Apr 3, 2022
 */

#ifndef LIST_H_
#define LIST_H_


/* about this class
 *
 * a doubly linked list of pointers to various objects
 * void* pointers are used for the payload. they may point to:
 *  files
 *  file types
 *  whatever
 *
 * Will be used for:
 *
 *
 *** things a list needs to be able to do
 * create itself
 * insert a new item at the head
 * remove an item
 * sort itself? (probably not needed)
 * delete an item
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
#include "a2560_platform.h"
#include "general.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


struct List
{
	List*	next_item_;
	List*	prev_item_;
    void*		payload_;
};



/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// generates a new list item. Does not add the list item to a list. Use List_AddItem()
List* List_NewItem(void* the_payload);

// destructor
void List_Destroy(List** head_item);

// adds a new list item as the head of the list
void List_AddItem(List** head_item, List* the_item);

// adds a new list item after the list_item passed
void List_AddItemAfter(List** list_head, List* the_new_item, List* the_existing_item);

// adds a new list item before the list_item passed (making itself the head item)
void List_Insert(List** head_item, List* the_item, List* previous_item);

// removes the specified item from the list (without destroying the list item)
void List_RemoveItem(List** head_item, List* the_item);

// iterates through the list looking for the list item that matches the address of the payload object passed
List* List_FindThisObject(List** head_item, void* the_payload);

// prints out every item in the list, using the helper function passed
List* List_Print(List** list_head, void (* print_function)(void*));

// frees the specified item and the data it points to
//void List_DeleteItem(List* the_item);

// Merge Sort. pass a pointer to a function that compares the payload of 2 list items, and returns true if thing 1 > thing 2
void List_InitMergeSort(List** list_head, bool (* compare_function)(void*, void*));

// returns a list item for the first item in the list; returns null if list is empty
List* List_GetFirst(List** head_item);

// returns a list item for the last item in the list; returns null if list is empty
List* List_GetLast(List** list_head);

// for the passed list, return the mid-point list item, given the starting point and ending point desired
// use this to do binary searches, etc. if max_item is NULL, will continue until end of list
List* List_GetMidpoint(List** list_head, List* starting_item, List* max_item);

//*** declarations for test function(s) ***



#endif /* LIST_H_ */
