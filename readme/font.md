# Text Class
Provides bitmapped proportional-width font functionality for the A2560. No support for outline, true type, etc. type fonts is provided. The interface is based on MacOS QuickDraw text handling routines as documented in Inside Macintosh. 

## Acquiring Fonts
Macintosh-compatible fonts can be used, if they are in "FONT" type resources. This means that you can create and edit fonts on any Macintosh (system 6-9) emulator. 

## Status
25% complete. Next phase: clean up, unit testing, documentation  

## Why you might want this:
If you have, or are considering acquiring an A2560K or GenX (68040 OR 486 + 65816), and want to be able to use proportional and/or bitmap fonts in your C programs.  If your program is ok limiting itself to aligning text on the 8x8 text cell grid of the A2560, consider using the Text class instead, or directly hitting the A2560's text RAM with text overlay mode.
Note: **NO ONE SHOULD USE THIS, OR EVEN LOOK IN ITS DIRECTION!** (until I get some real hardware and can complete the job).

## Expected final functionality
 * load a font from disk
 * Draw a char to a specified x, y pixel coord
 * display a string at a specified x, y pixel coord (no wrap)
 * display a pre-formatted string in a rectangular block on the screen, breaking on \n characters
 * display a string in a rectangular block on the screen, with wrap
 * display a string in a rectangular block on the screen, with wrap, taking a hook for a "display more" event, and scrolling text vertically up after hook func returns 'continue' (or exit, returning control to calling func, if hook returns 'stop')
 * measure the width of string in pixels


### Stretch Goals
 * Maintain a list of loaded fonts, and allow programs to switch between them without loading from disk again, and also free fonts when done
 * Free a font when the last bitmap that references it switches font, or is closed. Implies tracking a list of bitmaps and their fonts.

### Super Stretch Goals
 * 
 
## Completed
 * Draw a char to a specified x, y pixel coord
 * display a string at a specified x, y pixel coord (no wrap)
 * display a pre-formatted string in a rectangular block on the screen, breaking on \n characters
 * display a string in a rectangular block on the screen, with wrap
 * display a string in a rectangular block on the screen, with wrap, taking a hook for a "display more" event, and scrolling text vertically up after hook func returns 'continue' (or exit, returning control to calling func, if hook returns 'stop')
 * measure the width of string in pixels

## ToDo
 * load a font from disk (no disk support yet in the f68 emulator)
 * optimization
 * ~~make some decisions about when to pass font vs bitmap vs screen, and refactor~~
 * documentation
 * unit testing