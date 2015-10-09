;
;  ovlslave.asm - slave routines.
;
;  Copyright (C) 2000, Imre Leber.
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have recieved a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;  If you have any questions, comments, suggestions, or fixes please
;  email me at:  imre.leber@worldonline.be
;

segment _DATA class=DATA

        HostAddress   DD 0             ;; Host address.

segment _TEXT class=CODE

;=======================================================================
;===                      SetHostAddress                             ===
;===-----------------------------------------------------------------===
;=== Prepares the call to the host.                                  ===
;===                                                                 ===
;===  void SetHostAddress(int segment, int offset);                  ===
;===                                                                 ===
;=== Returns 0 if host not found, 1 if host found.                   ===
;=======================================================================

        global _SetHostAddress
_SetHostAddress:
        push bp        
        mov  bp, sp

        mov  ax, [bp+06h]
        mov  [HostAddress], ax     ;; offset.

        mov  ax, [bp+04h]
        mov  [HostAddress+02h], ax ;; segment

        pop  bp                    
        ret

;========================================================================
;===                          CallHost                                ===
;===------------------------------------------------------------------===
;=== Host call routine.                                               ===
;========================================================================

        global CallHost
CallHost:
        call far [HostAddress]
        ret
