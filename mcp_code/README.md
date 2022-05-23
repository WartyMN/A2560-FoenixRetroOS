# About the code in this folder

This code is used with permissions, from the Foenix MCP project. 
https://github.com/pweingar/FoenixMCP

All code in this folder (except my hacks) is:
Copyright (c) 2021, Peter J. Weingartner
All rights reserved.
(Please see below)

## Modifications
The code deals with the keyboard and mouse drivers. I have modified each of them very slightly to inject events into the OS/f event manager's buffer. The code changes are as follows:
- ring_buffer.c > rb_word_put(): creates an OS/f event record instead of inserting the scan code (or whatever, but I'm only using this for scan codes) into the MCP's circular buffer
- ps2.c > mouse_handle_irq(): a few lines of additional code at the end to get the X/Y from VICKY, and get R/L button status, and insert an event manager event record. 

## MCP License

BSD 3-Clause License

Copyright (c) 2021, Peter J. Weingartner
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.