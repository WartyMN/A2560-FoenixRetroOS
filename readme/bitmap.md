@subpage Bitmap Class
# Bitmap Class
Provides various functions for using the Foenix A2560's graphics mode. See below for the list of functionality.

## Status
50% complete. Next phase: Start writing code.  
I do not have any hardware yet. I'm doing all development against the f68 emulator, but that itself is also a tool in development. 

## Why you might want this:
If you have, or are considering acquiring an A2560K or GenX (68040 OR 486 + 65816), and want a simple library to work with bitmap graphics in your C programs.  
Note: **NO ONE SHOULD USE THIS, OR EVEN LOOK IN ITS DIRECTION!** (until I get some real hardware and can complete the job).

## How to use it
The A2560 kernel/OS doesn't support dynamically linked libraries yet, so for now, it is only available as a static library. In the "for_vbcc" folder, you will file a header file, and the library. Add these to your $VBCC "targets/a2560-elf" folder.  
`$VBCC/targets/a2560-elf/include/mb/lib_graphics.h`  
`$VBCC/targets/a2560-elf/lib/a2560_graphics.lib`  

Include the lib_graphics file in your C file(s):  
`#include <mb/lib_graphics.h>`

Modify your VBCC config file to include the library. Add "-la2560_graphics" to each vlink line.  

## documentation
See "docs/html/index.html".

## Expected final functionality
 * initialize/enter graphics mode
 * exit graphics mode
 * fill an entire screen of bitmap
 * fill a rect
 * draw a rect
 * allocate a bitmap
 * copy a bitmap
 * load a bitmap from disk
 * draw a circle
 * fill an enclosed area
 * draw a round rect
 * paint a round rect
 * draw a line
 * get the value of a pixel, from specified x/y coords
 * set the value of a pixel, from specified x/y coords
 * copy a rect of pixel mem from one bitmap to another
 * copy a rect of pixel mem from place to place within the same bitmap
 * copy a rect of pixel mem, apply a mask to it, and transfer to another or same bitmap
 * change LUT
 * load a LUT from disk
 * cycle LUT

### Stretch Goals
 * load a graphical (proportional width or fixed width) font from disk or memory - DONE
 * draw string using graphical font on screen, at specified x/y - DONE
 * draw string using graphical font on screen, wrapping and fitting to specified rectangle - DONE

### Super Stretch Goals
 * have a clipping system that prevents drawing to non-clipped parts of the screen
 * have a layers system that prevents drawing to portions of layers that are under other layers
 
## Completed
 * allocate a bitmap
 * initialize/enter graphics mode
 * exit graphics mode
 * fill an entire screen of bitmap
 * fill a rect
 * draw a rect
 * draw a round rect
 * draw a line
 * draw a circle
 * get the value of a pixel, from specified x/y coords
 * set the value of a pixel, from specified x/y coords
 * copy a rect of pixel mem from one bitmap to another (blit)
 * copy a rect of pixel mem from place to place within the same bitmap
 * paint a round rect
 * copy a bitmap
 * fill an enclosed area (but dangerous with small stack--need to rework into non-recursive variant?)

## ToDo
 * load a bitmap from disk
 * change LUT
 * load a LUT from disk
 * cycle LUT
 * documentation
 * unit testing