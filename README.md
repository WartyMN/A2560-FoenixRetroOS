@mainpage Foenix A2560 Foenix Retro OS: fr/OS
# A2560-FoenixRetroOS
This project provides 2 things:
1. A static C library/framework that anyone can use to build applications for the [Foenix](https://c256foenix.com/?v=3e8d115eb4b3) retro computers. Included functionality: Windowing, events, bitmapped graphics including proportional fonts, support for "text mode".
2. A single-tasking operating system and file browser that sits atop the [Foenix MCP Kernel](https://github.com/pweingar/FoenixMCP), and lets you access and manage the files on your hard drive, floppy, and SD cards. 

## What's an A2560?
The [A2560](https://c256foenix.com/?v=3e8d115eb4b3) is a newly-created retro computer based around a Motorolla 680x0. It is not a new Amiga, Atari ST, Mac, NeXT: it is a new thing, designed for hobbyist users who want to experience coding and using the "simpler" computers of the late 1980s/early 1990s. It is part of a family of computers that share much of the same architecture. Predecessors include the C256 Foenix, running a 65816 (fast) 8-bit processor. An upcoming variant includes a 65816 CPU, and push-button to switch between the processors. Future planned versions a 486 in the place of the 680x0.

## About this project
This is (currently) a one-person project to build a new OS and file manager for the A2560. That is the ultimate goal, but when that is accomplished, and how ambitious/complete version 1.0 will be is yet to be determined!  

## Why you might want this:
If you have, or are considering acquiring an A2560K or GenX (68040 OR 486 + 65816), and want to make or port applications (as opposed to games). As a development framework, it is primarily geared towards application development, but mouse- and text-based games would be viable candidates for building/porting under this framework. If you are making a fast action game, you will most likely want to bang directly on the hardware.
Note: **NO ONE SHOULD USE THIS, OR EVEN LOOK IN ITS DIRECTION!** (until I get some real hardware and can complete the job).

## How to use it

### Development platform
Currently, for the A2560, all development is cross-compiling from a modern computer to the Foenix. All the tools you need are available for Windows, Linux, and Mac. At a high-level, the process involves setting up a C environment, writing code, compiling to the A2560 target, and then opening the resulting executable file in an emulator, or transferring it to a real A2560 and running it there.  

### Setting up your development tool chain
The only tested tool chain is the VBCC C compiler, with the [A2560 VBCC target](https://github.com/daschewie/Foenix_vbcc_target). Another C compiler toolchain, [Calypsi](https://github.com/hth313/Calypsi-m68k-Foenix), is available, but I have not had a chance to test it. 

The A2560 kernel/OS doesn't support dynamically linked libraries yet, so for now, it is only available as a static library. In the "for_vbcc" folder in this project, you will find several header files, and the system library.
1. Download and install the [f68 emulator](https://github.com/paulscottrobson/f68-emulator) (or buy a Foenix). 
1. Download/build/install VBCC
1. Download the [A2560 VBCC target](https://github.com/daschewie/Foenix_vbcc_target)
1. Add these to your $VBCC "targets/a2560-elf" folder.  
`$VBCC/targets/a2560-elf/include/mb/`  
`$VBCC/targets/a2560-elf/lib/a2560_sys.lib` 

1. Modify your VBCC config file to include the library: Add "-la2560_sys" to each vlink line.  
1. Include the relevant header files in your C file(s):  
`#include <mb/lib_sys.h>`  
`#include <mb/window.h>`  
etc.  

1. Create a makefile or other script to build. I prefer a shell script system I have configured for my development environment (BBEdit on Mac), but whatever you normally do will work fine. Take a look at the _build_vbcc.sh shell script to check the warning on the .s28 files generated from VBCC and how they need to be tweaked after generation. 

I will add instructions for developing with the Calipsi tool chain once I have had a chance to test it out.


### Documentation

#### Doxygen Docs
Doxygen low-level documentation can be found in [docs/html/index.html](docs/html/index.html).

#### Examples/Demos
Each major category of functionality in the platform has a xxxx_demo.c file, and a corresponding .s28 executable in the "build_vbcc" folder. I recommend loading up each of those in turn, and tapping through them to see an example of what the key functions do and how they are used. Most demos have a "hit any key to advance" style. 

#### Programmer's Reference Guide
I do hope to publish a Programmer's Reference Guide once the framework graduates from frothy liquid to vaguely gelatinous. 

#### fr/OS User's Guide
I really hope so! I love the "Congratulations on the purchase of your new XXXX computer" documentation from the 1980s and 1990s. Just need to write the OS and file manager first... 

#### Breakdown of functionality -- Application framework
 * [Bitmap functions](readme/bitmap.md)
 * [Font functions](readme/font.md)
 * [Text mode functions](readme/text.md)
 * [Window functions](readme/window.md)

#### Expected functionality -- File Manager / Desktop environment
 * Icon view
 * List view
 * Column view (a la NeXT or OS X)
 * Custom themes - window colors, widgets, fonts, size, background pattern, icons.
 * Basic file management: delete, duplicate, drag-n-drop, move, new folder, etc. 
 * Ability to launch executables
 * Easter eggs: probably very limited: This is an open source project, so it would be hard to hide them.
 * Super-cool-idea: too early to share :)

## Status
Framework: 10% complete. Current phase: continue to build out windowing functionality  
File Manager: 0% complete. Current phase: wait until enough base functionality is present in the framework.   
I do not have any hardware yet. I'm doing all development against the f68 emulator, but that itself is also a tool in development. 

### Completed
 * 

### ToDo
 * 