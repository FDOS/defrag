;
;   strings.asm - string function replacements.
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


segment STRINGS_TEXT        

;=======================================================================
;===                           StringLength                          ===
;===-----------------------------------------------------------------===
;=== Returns the length of a string.                                 ===
;===                                                                 ===
;=== int StringLength(char*);                                        ===
;=======================================================================

        global _StringLength
_StringLength:
        push bp
        mov  bp, sp
        push ds

        lds  bx, [bp+06h]

        mov  ax, 0
.CountLoop:

        cmp  [bx], byte 0
        je   .EndReached

        inc  ax
        inc  bx
        jmp  .CountLoop

.EndReached:
        pop  ds
        pop  bp
        retf

;=========================================================================
;===                           StringCopy                              ===
;===-------------------------------------------------------------------===
;=== Copy one string to another.                                       ===
;===                                                                   ===
;=== void StringCopy(char* dest, char* src);                           ===
;=========================================================================

        global _StringCopy
_StringCopy:

        push bp
        mov  bp, sp
        push si
	push di
	push ds
	push es

	lds  di, [bp+06h]
	les  si, [bp+0Ah]
.CopyLoop:
	mov  al, [es:si]
	mov  [di], al

        inc  si
        inc  di

        cmp  al, 0
        jne  .CopyLoop

	pop  es
	pop  ds
        pop  di
        pop  si
        pop  bp
        retf

