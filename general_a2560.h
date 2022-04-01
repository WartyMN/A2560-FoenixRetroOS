/*
 * general_a2560.h
 *
 *  Created on: Feb 19, 2022
 *      Author: micahbly
 */

#ifndef GENERAL_A2560_H_
#define GENERAL_A2560_H_


/* about this class
 *
 *
 *
 *** things this class needs to be able to do
 * TBD -- just a holding tank for now. Will likely build out into a graphics lib in future. 
 *
 *** things objects of this class have
 *
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/



/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/
// 
// // pop up a requester box with message to user. var args are for the message. 
// unsigned long General_ShowAlert(struct Window* the_window, const char* the_title, bool is_error, bool with_cancel, const char* message_format, ...);
 
// // draw a line in an intuition window
// void General_DrawLine(struct RastPort* the_rastport, signed long x1, signed long y1, signed long x2, signed long y2, char the_color_pen);
// 
// // draw a poly in an intuition window
// void General_DrawPoly(struct RastPort* the_rastport, short num_coords, short* the_coordinates, char the_color_pen);
// 
// // draw a rectangle in the rastport passed. If do_undraw is TRUE, try to undraw it (unimplemented TODO)
// void General_DrawBox(struct RastPort* the_rastport, signed short x1, signed short y1, signed short x2, signed short y2, bool do_undraw, char the_color_pen);
// 
// // checks a file exists without locking the file. tries to get a lock on the dir containing the file, then checks contents until it matches
// // SLOW, and probably pointless, but struggling with issue of locks not unlocking when checking for existence of an icon file.
// bool General_CheckFileExists(char* the_file_path);
// 
// // return current date/time as a timestamp. 
// struct DateStamp* General_GetCurrentDateStampWithAlloc(void);





#endif /* GENERAL_A2560_H_ */
