# Text Class
Provides basic functions for the A2560's text mode

## Status
75% complete. Next phase: Bug-fixing, smarter configuration, etc., No optimization done, no testing against hardware.  

## Why you might want this:
If you have, or are considering acquiring an A2560K or GenX (68040 OR 486 + 65816), and want a simple way to put text on the screen in your C programs.  
Note: **NO ONE SHOULD USE THIS, OR EVEN LOOK IN ITS DIRECTION!** (until I get some real hardware and can complete the job).

## Expected final functionality
 * work with either channel A or channel B
 * clear / fill an entire screen of text characters
 * clear / fill an entire screen of text attributes
 * invert the colors of a screen
 * clear / fill a smaller-than-screen rectangular area of text/attrs
 * Draw a char to a specified x, y coord
 * Get the currently displayed character at the specified coord
 * Set the foreground and background colors at the specified coord
 * Set the attribute value at the specified coord
 * Get the attribute value at the specified coord
 * Get the foreground or background color at the specified coord
 * draw a line using "graphic" characters
 * draw a box using "graphic" characters
 * copy a full screen of text or attr from an off-screen buffer
 * copy a full screen of text or attr TO an off-screen buffer
 * copy a full screen of text and attr between channel A and B
 * copy a rectangular area of text or attr TO/FROM an off-screen buffer
 * display a pre-formatted string in a rectangular block on the screen, breaking on \n characters
 * Draw a string to a specified x, y coord (no wrap)
 * format a string for a given height and width, wrapping words as necessary
 * display a formatted string in a rectangular block on the screen, with wrap
 * display a string in a rectangular block on the screen, with wrap, taking a hook for a "display more" event, and scrolling text vertically up after hook func returns 'continue' (or exit, returning control to calling func, if hook returns 'stop')
 * replace current text font with another, loading from specified ram loc.
 * configure at compile time for use in various Foenix machines
 * Auto recognize the screen resolution, num text cols, size of borders, etc. 
 * ability to change screen resolution
 
## Completed
 * work with either channel A or channel B
 * clear / fill an entire screen of text characters
 * clear / fill an entire screen of text attributes
 * invert the colors of a screen
 * clear / fill a smaller-than-screen rectangular area of text/attrs
 * Draw a char to a specified x, y coord
 * Get the currently displayed character at the specified coord
 * Set the foreground and background colors at the specified coord
 * Set the attribute value at the specified coord
 * Get the attribute value at the specified coord
 * Get the foreground or background color at the specified coord
 * draw a line using "graphic" characters
 * draw a box using "graphic" characters
 * copy a full screen of text or attr from an off-screen buffer
 * copy a full screen of text or attr TO an off-screen buffer
 * copy a rectangular area of text or attr TO/FROM an off-screen buffer
 * Draw a string to a specified x, y coord (no wrap)
 * format a string for a given height and width, wrapping words as necessary
 * display a formatted string in a rectangular block on the screen, with wrap
 * display a string in a rectangular block on the screen, with wrap, taking a hook for a "display more" event, and scrolling text vertically up after hook func returns 'continue' (or exit, returning control to calling func, if hook returns 'stop')
 * replace current text font with another, loading from specified ram loc.
 * documentation (for text lib, have not started on general lib)
 * prep and clean up to be a standalone static library
 * Auto recognize the screen resolution, num text cols, size of borders, etc. 
 * ability to change screen resolution

## ToDo
 * copy a full screen of text and attr between channel A and B
 * ~~clean up the A2560U/K vs morfe addresses, default colors, etc.~~
 * optimization
 * ~~add function(s) to switch resolutions, and automatically adjust number of cols/rows~~
 * ~~add function(s) to adjust border sizes, and automatically adjust number of visible rows/columns~~
 * ~~configure at RUN time for use in various A2560 machines~~
 * ~~fix bug with word wrap declining to present the final line in the string if string is shorter than box.~~
 * clean up text wrap buffers etc
 * strategy for handling DEBUG macros (build 2 versions of library? one debug mode, one not? TBD)
 * document a2560_platform.h
 * document lib_general.c
 
