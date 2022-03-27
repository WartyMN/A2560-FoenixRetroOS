//! @file general.h

/*
 * general.h
 *
 *  Created on: Feb 19, 2022
 *      Author: micahbly
 */

#ifndef GENERAL_H_
#define GENERAL_H_


/* about this class
 *
 *
 *
 *** things this class needs to be able to do
 * various utility functions that any app could find useful.
 * intended to be re-usable across apps, with at most minimal differences.
 * this file should contain only cross-platform code. platform-specific code should go into general_[platformname].h/.c
 *
 *** things objects of this class have
 *
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// A2560 includes
#include <mb/a2560_platform.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

// general
#define MAX_STRING_COMP_LEN		256	// 255 + terminator is max string size for compares

// file-related
#define FILE_MAX_PATHNAME_SIZE	260	// https://www.keil.com/pack/doc/mw/FileSystem/html/fat_fs.html

// turn debug mode on/off
#define LOG_LEVEL_1		// errors
#define LOG_LEVEL_2		// warnings
#define LOG_LEVEL_3		// infos
#define LOG_LEVEL_4		// debug level info for programmer
#define LOG_LEVEL_5x		// memory allocation info for programmer

#ifdef LOG_LEVEL_1 
	#define LOG_ERR(x) General_LogError x
#else
	#define LOG_ERR(x)
#endif
#ifdef LOG_LEVEL_2
	#define LOG_WARN(x) General_LogWarning x
#else
	#define LOG_WARN(x)
#endif
#ifdef LOG_LEVEL_3
	#define LOG_INFO(x) General_LogInfo x
#else
	#define LOG_INFO(x)
#endif
#ifdef LOG_LEVEL_4
	#define DEBUG_OUT(x) General_DebugOut x
#else
	#define DEBUG_OUT(x)
#endif
#ifdef LOG_LEVEL_5
	#define LOG_ALLOC(x) General_LogAlloc x
#else
	#define LOG_ALLOC(x)
#endif

// control if target is for real machine or f68
// matters: f68 has a special log feature at 0xffffffff-4 that will not work on real machine
#define _f68_	1	// undefine "_f68_" when building for real hardware



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum LoggingLevel
{
	LogError = 0,
	LogWarning = 1,
	LogInfo = 2,
	LogDebug = 3,
	LogAlloc = 4
} LoggingLevel;



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/

//! Format a string by wrapping and trimming to fit the passed width and height. 
//! If the string cannot be displayed in the specified height and width, processing will stop, but it is not an error condition
//! @param	orig_string: pointer to a string pointer that holds the text to be formatted. Upon return, this pointer will point to the next character after the last processed character (if the string was too long to fit). If the entire string fits, this pointer will not be adjusted.
//! @param	formatted_string: pointer to a string pointer to an empty string buffer that will be filled with the formatted version of the text.
//! @param	max_chars_to_format: the length of the string to format (in characters). If max_chars_to_format is less than the length of string, processing will stop after that many characters.
//! @param	max_width: the width into which the text must fit, in pixels. 
//! @param	max_height: the height into which the text must fit, in pixels. Pass a 0 to disable the governor on vertical space. 
//! @param	one_char_width: the width in pixels, of one character. NOTE: This is only used for fixed-width, text mode operations. 
//! @param	one_row_height: the height in pixels, of one row of text, including any leading. 
//! @param	the_font: the font object to be used in measuring width. This is optional and ignore if called for text mode operations.
//! @param	measure_function: pointer to the function responsible for measuring the graphical width of a string 
//! @return Returns number of vertical pixels required. Returns -1 in any error condition.
signed int General_WrapAndTrimTextToFit(char** orig_string, char** formatted_string, signed int max_chars_to_format, signed int max_width, signed int max_height, signed int one_char_width, signed int one_row_height, Font* the_font, signed int (* measure_function)(Font*, char*, signed int, signed int, signed int));

// replacement for tolower() in c library, which doesn't seem to work here for some reason.
char General_ToLower(char the_char);

// cheapo way to do round. THINK C's and SAS/C's math.h don't include round()
// from: https://stackoverflow.com/questions/4572556/concise-way-to-implement-round-in-c
int General_Round(double the_float);

// convert a file size in bytes to a human readable format using "10 bytes", "1.4 kb", "1 MB", etc. 
//   NOTE: formatted_file_size string must have been allocated before passing here
void General_MakeFileSizeReadable(unsigned long size_in_bytes, char* formatted_file_size);

// return the first char of the last part of a file path
// if no path part detected, returns the original string
// not guaranteed that this is a FILENAME, as if you passed a path to a dir, it would return the DIR name
// amigaDOS compatibility function (see FilePart)
char* General_NamePart(const char* the_file_path);

// return everything to the left of the filename in a path. 
char* General_PathPart(const char* the_file_path);

//! Extract file extension into the passed char pointer, as new lowercased string pointer, if any found.
//! @param	the_file_name: the file name to extract an extension from
//! @param	the_extension: a pre-allocated buffer that will contain the extension, if any is detected. Must be large enough to hold the extension! No bounds checking is done. 
//! @return	Returns false if no file extension found.
boolean General_ExtractFileExtensionFromFilename(const char* the_file_name, char* the_extension);

// allocates memory and copies the passed string into it. one stop shop for getting a copy of a string
char* General_StrlcpyWithAlloc(const char* src, unsigned long max_len);

// strlcpy implementation from apple/bsd. the Amiga Strlcpy is only in 4.0. Modified with Amiga-isms
unsigned long General_Strlcpy(char* dst, const char* src, unsigned long max_len);

// strlcpy implementation from apple/bsd. the Amiga Strlcpy is only in 4.0. Modified with Amiga-isms
unsigned long General_Strlcat(char* dst, const char* src, unsigned long max_len);

// strncmp implementation from Amiga site.
// http://home.snafu.de/kdschem/c.dir/strings.dir/strncmp.c
int General_Strncmp(const char* string_1, const char* string_2, long length);

// strncasecmp (case insensitive comparison) based on code from slashdot and apple open source
// https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
// https://opensource.apple.com/source/tcl/tcl-10/tcl/compat/strncasecmp.c.auto.html
signed int General_Strncasecmp(const char* string_1, const char* string_2, long max_len);

// Safe(r) string length function: will stop processing if no terminator found before max_len reached
// Inspired by apple/bsd strnlen.
unsigned long General_Strnlen(const char *the_string, unsigned long max_len);

// function compatible with List_MergeSortedList(). Compares to strings passed as void pointers, returns true if first string is longer than second. 
// NOTE: compares to a maximum of FILE_MAX_PATHNAME_SIZE
boolean General_CompareStringLength(void* first_payload, void* second_payload);

// Find the next space, dash, or other word break character and return its position within the string. If none found before end of string or max len, returns -1.
signed int General_StrFindNextWordEnd(const char* the_string, signed int max_search_len);

// Find the next line break character and return its position within the string (+1: first char is '1'). If none found before end of string or max len, returns 0.
signed int General_StrFindNextLineBreak(const char* the_string, signed int max_search_len);

// test if 2 rectangles intersect
boolean General_RectIntersect(struct Rectangle r1, struct Rectangle r2);

// test if a point is within a rectangle
boolean General_PointInRect(signed int x, signed int y, Rectangle r);

// Convert a positive or negative string integer to a signed long integer. returns false in event of error
boolean General_StringToSignedLong(const char* the_string_value, signed long* the_conversion);

// Position one rect within the bounds of another. Horizontally: centers the hero rect within the left/right of the frame rect; Vertically: centers or or puts at 25% line
// put the frame coords into the frame_rect, and the object to be centered into the hero_rect. ON return, the frame rect will hold the coords to be used.
void General_CenterRectWithinRect(Rectangle* the_frame_rect, Rectangle* the_hero_rect, boolean at_25_percent_v);


// allocate and return  the portion of the path passed, minus the filename. In other words: return a path to the parent file.
// calling method must free the string returned
char* General_ExtractPathToParentFolderWithAlloc(const char* the_file_path);

// allocate and return the filename portion of the path passed.
// calling method must free the string returned
char* General_ExtractFilenameFromPathWithAlloc(const char* the_file_path);

// populates the passed string by safely combining the passed file path and name, accounting for cases where path is a disk root
void General_CreateFilePathFromFolderAndFile(char* the_combined_path, char* the_folder_path, char* the_file_name);




// *********  logging functionality. requires global_log_file to have been opened.
void General_LogError(const char* format, ...);
void General_LogWarning(const char* format, ...);
void General_LogInfo(const char* format, ...);
void General_DebugOut(const char* format, ...);
void General_LogAlloc(const char* format, ...);
boolean General_LogInitialize(void);
void General_LogCleanUp(void);

// debug function to print out a chunk of memory character by character
void General_PrintBufferCharacters(char* the_data, unsigned short the_len);


#endif /* GENERAL_H_ */
