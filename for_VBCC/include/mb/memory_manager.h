//! @file memory_manager.h

/*
 * memory_manager.h
 *
*  Created on: Mar 21, 2022
 *      Author: micahbly
 */

#ifndef MEMORY_MGR_H_
#define MEMORY_MGR_H_


/* about this class: Memory Manager
 *
 * Provides wrapper functions for the BGET memory manager, very slightly modified to have 2 separate pools, one for VRAM, one for normal RAM.
 * All of the WORK of this code IS the BGET code.
 * I made 2 very minor hacks:
 *  I removed the select best fit and auto memory manager functions to make it easier for (me) to read the code.
 *  I made it so that the "freelist" global property is not global, and is passed to the functions by my wrappers. This lets me have it manage separate pools for VRAM and normal RAM
 *
 *** things this class needs to be able to do
 * Allocate and free memory in VRAM space
 * Allocate and free memory in system RAM space
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

// A2560 includes
#include <mb/a2560_platform.h>

// C includes
#include <stdbool.h>
#include <stdio.h>

/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

// start and end of standard RAM is a bit tricky to know. 
// if someone configures vlink.cmd (if VBCC) differently, these aren't goint to work
// also need to account for BSSSIZE of the system
// will estimate for now
#define STD_RAM_START	0x00060000
#define STD_RAM_LEN		0x00300000
//#define STD_RAM_LEN		0x0000000F

// VRAM is currently only available in VRAM Buffer A on A2560K (2022/03/21)
#define VRAM_START		0x00800000
#define VRAM_LEN		0x00BFFFFF - VRAM_START


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum mem_type
{
	MEM_STANDARD	= 0,
	MEM_VRAM	 	= 1,
} mem_type;


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

typedef struct bfhead MemoryPool;



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/



//! Initialize the VRAM and standard RAM memory pools'
//! return Returns NULL if it fails to allocate enough memory to start the BGET system up
bool Memory_Initialize(void);



// **** BGET wrapper functions *****

void* f_calloc(size_t num, size_t size, mem_type the_mem_type);
void* f_malloc(size_t size, mem_type the_mem_type);
void f_free(void* the_ptr, mem_type the_mem_type);



// **** xxx functions *****




// **** xxx functions *****


/*****************************************************************************/
/*                       BGET Function Prototypes                            */
/*****************************************************************************/

/*

    Interface definitions for bget.c, the memory management package.

*/

// NOTE: I moved all of the BGET function prototypes out of the header, and into memory_manager.c, because I don't want them to be the interface





#endif /* MEMORY_MGR_H_ */


