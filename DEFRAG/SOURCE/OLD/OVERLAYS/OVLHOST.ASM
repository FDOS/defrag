;
;   Hostslct.c - host routines.
;
;   Copyright (C) 2000, Imre Leber.
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
;   You should have recieved a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

;   If you have any questions, comments, suggestions, or fixes please
;   email me at:  imre.leber@worldonline.be
;

extern _SelectHostRoutine

segment _DATA class=DATA
        savedSI DW 0
        savedDI DW 0

segment _TEXT class=CODE

;===========================================================================
;===                            GetDispatcher                            ===
;===---------------------------------------------------------------------===
;=== Returns the address of the dispatcher.                              ===
;===                                                                     ===
;=== void GetDispatcher(int* segment, int* offset);                      ===
;===========================================================================
        
        global _GetDispatcher
_GetDispatcher:
        push bp
        mov  bp, sp

        mov  bx,   [bp+04h]
        mov  ax,   cs
        mov  [bx], ax

        mov  bx,   [bp+06h]
        mov  [bx], word dispatcher

        pop  bp
        ret

;===========================================================================
;===                            dispatcher                               ===
;===---------------------------------------------------------------------===
;=== dispatcher routine which is called from the overlays.               ===
;===                                                                     ===
;=== Calls a routine:                                                    ===
;===    int SelectHostRoutine(int ax, int bx, int cx, int dx, int ds);   ===
;===                                                                     ===
;=== to do the real selection.                                           ===
;===========================================================================

dispatcher:
        push si        
        push di
        push ds
        push dx
        push cx                         ;; Push parameters on the stack.  
        push bx
        push ax                         

        mov  ax, _DATA                  ;; Set data segment.
        mov  ds, ax
        mov  es, ax
        
        mov  si, [savedSI]
        mov  di, [savedDI]

        call _SelectHostRoutine         ;; Call selection routine.

        pop  bx
        pop  bx                         ;; Take parameters of the stack.
        pop  cx
        pop  dx
        pop  ds
        pop  di
        pop  si
        retf

;===========================================================================
;===                           SaveHostState                             ===
;===========================================================================
;=== Save the host status.                                               ===
;===                                                                     ===
;=== void SaveHostState(void);                                           ===
;===========================================================================

        global _SaveHostState
_SaveHostState:
        
        mov  [savedSI], si
        mov  [savedDI], di

        ret

