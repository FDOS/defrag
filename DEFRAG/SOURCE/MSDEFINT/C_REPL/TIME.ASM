;
;   time.asm - time functions
;   Copyright (C) 2000 Imre Leber
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;   If you have any questions, comments, suggestions, or fixes please
;   email me at:  imre.leber@worldonline.be
;


segment TIME_TEXT 

;==========================================================================
;===                          WasteTime                                 ===
;===--------------------------------------------------------------------===
;=== Waits for a number of clock ticks.                                 ===
;===                                                                    ===
;=== void WasteTime(int ticks);                                         ===
;==========================================================================

        global _WasteTime
_WasteTime:
        mov  bx, sp
        mov  bx, [ss:bx+04h]

        push si
        push di

        mov  ah, 00h
        int  1Ah
        
        mov  di, cx
        mov  si, dx

.Waster:        
        mov  ah, 00h
        int  1Ah

        sub  dx, si
        sbb  cx, di

        cmp  cx, 0
        jne  .EndOfProc

        cmp  dx, bx
        jb   .Waster

.EndOfProc:
        pop  di
        pop  si
        retf

