;
;   gscreen.asm - grab/dump screen parts.
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

segment GDSCREEN_TEXT 

;==========================================================================
;===                          GetScreenAddress                          ===
;===--------------------------------------------------------------------===
;=== Returns address of the screen page 0.                              ===
;===                                                                    ===
;=== Out: ds:si : pointer to screen page 0 .                            ===
;===                                                                    ===
;=== Remark: only call from assembly.                                   ===
;===                                                                    ===
;=== Only color cards.                                                  ===
;=== Only 80*25 text mode.                                              ===
;==========================================================================

        global GetScreenAddress
GetScreenAddress:
        
;        mov  ax, 0040h                  ;; Get offset current screen page.
;        mov  ds, ax
;        mov  si, [004eh]
        
        mov  si, 0000h

        mov  ax, 0b800h                 ;; Segment video memory (color only).
        mov  ds, ax
        
	retf

;===========================================================================
;===                         LoGrabScreen                                ===
;===---------------------------------------------------------------------===
;=== Grabs a part of the screen.                                         ===
;===                                                                     ===
;=== void LoGrabScreen(unsigned offset, int ylen, int xlen,              ===
;===                   void* dest);                                      ===
;===                                                                     ===
;=== xlen : number of characters to grab.                                ===
;=== ylen : number of lines.                                             ===
;===========================================================================

        global _LoGrabScreen  
_LoGrabScreen:        
        push bp
        mov  bp, sp

        push ds
        push es
        push si
        push di

	cld

        les  di, [bp+0Ch]                   ;; Set target to dest.

	call far GetScreenAddress
        add  si, [bp+06h]                   ;; Get screen address.

        mov  dx, [bp+0Ah]
.Loop1:
        push si
        mov  cx, [bp+08h]
        rep  movsw
        pop  si
        add  si, 160

        dec  dx
        jnz  .Loop1

        pop  di
        pop  si
        pop  es
        pop  ds

        pop  bp
        retf
        
;===========================================================================
;===                         LoDumpScreen                                ===
;===---------------------------------------------------------------------===
;=== Grabs a part of the screen.                                         ===
;===                                                                     ===
;=== void LoDumpScreen(unsigned offset, int ylen, int xlen,              ===
;===                   void* src);                                       ===
;===                                                                     ===
;=== xlen : number of characters to grab.                                ===
;=== ylen : number of lines.                                             ===
;===========================================================================

        global _LoDumpScreen  
_LoDumpScreen:        
        push bp
        mov  bp, sp

        push ds
        push es
        push si
        push di

        push ds

	call far GetScreenAddress
	mov  di, si
	add  di, [bp+06h]                   ;; Get screen address.
	push ds
	pop  es
	pop  ds

	cld

	lds  si, [bp+0Ch]                   ;; Set source to src.

        mov  dx, [bp+0Ah]
.Loop1:
        push di
        mov  cx, [bp+08h]
        rep  movsw
        pop  di
        add  di, 160
        
        dec  dx
        jnz  .Loop1

        pop  di
        pop  si
        pop  es
        pop  ds
        pop  bp
        retf
