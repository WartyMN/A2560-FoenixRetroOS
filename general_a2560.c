/*
 * general_a2560.c
 *
 *  Created on: Feb 19, 2022
 *      Author: micahbly
 */





/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

#include "text.h"

// project includes

// C includes
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

// A2560 includes
#include "a2560_platform.h"
#include "general.h"



/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/




/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

static char				message_buffer[ALERT_MAX_MESSAGE_LEN];	// used for alert dialogs sprintfs




/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/




/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/




/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/




/*
// draw a line in an intuition window
void General_DrawLine(struct RastPort* the_rastport, signed long x1, signed long y1, signed long x2, signed long y2, uint8_t the_color_pen)
{
	// move the pen to the first coordinate, or it will draw a line from where it is, to there
	Move( the_rastport, x1, y1 );

	// draw the line
	SetAPen( the_rastport, the_color_pen );
	Draw( the_rastport, x2, y2 );
}
*/


/*
// draw a poly in an intuition window
void General_DrawPoly(struct RastPort* the_rastport, int16_t num_coords, int16_t* the_coordinates, uint8_t the_color_pen)
{
	int16_t	x = *(the_coordinates + 0);
	int16_t	y = *(the_coordinates + 1);

	// move the pen to the first coordinate, or it will draw a line from where it is, to there
	Move( the_rastport, x, y );

	// draw the poly
	SetAPen( the_rastport, the_color_pen );
	PolyDraw( the_rastport, num_coords, the_coordinates );
}
*/


/*
// draw a rectangle in the rastport passed. If do_undraw is TRUE, try to undraw it (unimplemented TODO)
void General_DrawBox(struct RastPort* the_rastport, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool do_undraw, uint8_t the_color_pen)
{
	int16_t				coordinates[10];

	// COMPLEMENT mode will simply inverse pixels. This makes it possible to draw once, then draw again, to restore what was there
	SetDrMd(the_rastport, COMPLEMENT);

	// need 10 coords to use Intuitions PolyDraw for a box
	coordinates[0] = x1;
	coordinates[1] = y1;

	coordinates[2] = x2;
	coordinates[3] = y1;

	coordinates[4] = x2;
	coordinates[5] = y2;

	coordinates[6] = x1;
	coordinates[7] = y2;

	coordinates[8] = x1;
	coordinates[9] = y1;
	
	//DEBUG_OUT(("%s %d: %i, %i to %i, %i", __func__ , __LINE__, x1, y1, x2, y2));

	// set a dotted line pattern
	//SetDrPt(the_rastport, 0xAAAA);
	SetDrPt(the_rastport, 0xFFFF);

	// draw the surrounding box
	General_DrawPoly(the_rastport, 5, &coordinates[0], the_color_pen);

	// turn off pattern
	//SetDrPt(the_rastport, 0xFFFF);

	// reset draw mode in case we need it for plotting/replotting/etc.
	SetDrMd(the_rastport, JAM1);
}
*/




/*
// checks a file exists without locking the file. tries to get a lock on the dir containing the file, then checks contents until it matches
// SLOW, and probably pointless, but struggling with issue of locks not unlocking when checking for existence of an icon file.
bool General_CheckFileExists(unsigned char* the_file_path)
{
	BPTR 					the_dir_lock;
	struct FileInfoBlock*	fileInfo;
	LONG					the_io_error;
	unsigned char*			the_file_name_to_check;
	int16_t					filename_len;
	unsigned char*			the_directory_path;

	// get a string for the directory portion of the filepath
	if ( (the_directory_path = General_ExtractPathToParentFolderWithAlloc(the_file_path)) == NULL)
	{
		return false;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_directory_path	%p	size	%i		'%s'", __func__ , __LINE__, the_directory_path, General_Strnlen(the_directory_path, FILE_MAX_PATHNAME_SIZE) + 1, the_directory_path));
	
	// get the name portion of the path, so we can compare it later to each file we find in the parent directory
	the_file_name_to_check = FilePart(the_file_path);
	filename_len = General_Strnlen(the_file_name_to_check, FILE_MAX_PATHNAME_SIZE);

	// FileInfoBlocks have to be byte aligned, so needs AllocDosObject
	fileInfo = AllocDosObject(DOS_FIB, NULL);
	LOG_ALLOC(("%s %d:	__ALLOC__	fileInfo	%p	size	%i		AllocDosObject", __func__ , __LINE__, fileInfo, sizeof(struct FileInfoBlock)));

	// try to get lock on the  directory
	if ( (the_dir_lock = Lock((STRPTR)the_directory_path, SHARED_LOCK)) == 0)
	{
		LOG_INFO(("%s %d: Failed to lock dir %s!", __func__ , __LINE__, the_directory_path));
		LOG_ALLOC(("%s %d:	__FREE__	the_directory_path	%p	size	%i		'%s'", __func__ , __LINE__, the_directory_path, General_Strnlen(the_directory_path, FILE_MAX_PATHNAME_SIZE) + 1, the_directory_path));
		free(the_directory_path);
		the_directory_path = NULL;
		return false;
	}
	//printf("General_CheckFileExists: debug - dir-lock: %li / %p\n", the_dir_lock, BADDR( the_dir_lock ));

	if (Examine(the_dir_lock, fileInfo) != 0)
	{
		while ( ExNext(the_dir_lock, fileInfo) ) // data = ExamineDir(context))
		{
			if (fileInfo->fib_DirEntryType > 0)
			{
				// it's a directory
			}
			else
			{
				// it's a file... is it the we are looking for?
				if (General_Strncmp(the_file_name_to_check, (unsigned char*)fileInfo->fib_FileName, filename_len) == 0)
				{
					// it's the file we were looking for
					//printf("General_CheckFileExists: Confirmed file %s!\n", the_file_path);
					LOG_ALLOC(("%s %d:	__FREE__	the_directory_path	%p	size	%i		'%s'", __func__ , __LINE__, the_directory_path, General_Strnlen(the_directory_path, FILE_MAX_PATHNAME_SIZE) + 1, the_directory_path));
					free(the_directory_path);
					the_directory_path = NULL;
					
					LOG_ALLOC(("%s %d:	__FREE__	fileInfo	%p	size	%i		FreeDosObject", __func__ , __LINE__, fileInfo, sizeof(struct FileInfoBlock)));
					FreeDosObject(DOS_FIB, fileInfo);
					
					UnLock(the_dir_lock);
					return true;
				}
				else
				{
	//				printf("General_CheckFileExists: found |%s|!\n", fileInfo->fib_FileName);
				}
			}
		}
	}


	the_io_error = IoErr();

	if (the_io_error == ERROR_NO_MORE_ENTRIES)
	{
		// last file read: fail condition
	}
	else
	{
		// some other io error happened: just another fail condition
	}

	// free objects allocated in this method
	LOG_ALLOC(("%s %d:	__FREE__	the_directory_path	%p	size	%i", __func__ , __LINE__, the_directory_path, General_Strnlen(the_directory_path, FILE_MAX_PATHNAME_SIZE) + 1));
	free(the_directory_path);
	the_directory_path = NULL;
	
	LOG_ALLOC(("%s %d:	__FREE__	fileInfo	%p	size	%i		FreeDosObject", __func__ , __LINE__, fileInfo, sizeof(struct FileInfoBlock)));
	FreeDosObject(DOS_FIB, fileInfo);
	
	UnLock(the_dir_lock);

	return false;
}
*/


/*
// return current date/time as a timestamp. 
struct DateStamp* General_GetCurrentDateStampWithAlloc(void)
{
	struct DateStamp*	the_datetime;

	if ( (the_datetime = (struct DateStamp*)calloc(1, sizeof(struct DateStamp)) ) == NULL)
	{
		LOG_ERR(("%s %d: could not allocate memory to create new datestamp", __func__ , __LINE__));
		return NULL;
	}
	LOG_ALLOC(("%s %d:	__ALLOC__	the_datetime	%p	size	%i", __func__ , __LINE__, the_datetime, sizeof(struct DateStamp)));
	
	// have Amiga populate with the current time/date
	DateStamp(the_datetime);
	
	return the_datetime;
}
*/

