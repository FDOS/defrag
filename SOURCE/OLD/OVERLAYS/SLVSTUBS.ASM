;
;   Slvtubs.asm - slave stub routines.
;
;  Copyright (C) 2000, Imre Leber.
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
;
;   If you have any questions, comments, suggestions, or fixes please
;   email me at:  imre.leber@worldonline.be
;



extern CallHost

%include 'protocol.inc'

segment _TEXT class=CODE

;=======================================================================
;===                        SendNOP                                  ===
;===-----------------------------------------------------------------===
;=== Sends a no processing message to the host, so that the internal ===
;=== state of the host can be updated.                               ===
;===                                                                 ===
;=== void SendNOP(void);                                             ===
;=======================================================================
        
        global _SendNOP
_SendNOP:
        mov  ax, NOP
        call CallHost

        ret

;=======================================================================
;===                        SmallMessage                             ===
;===-----------------------------------------------------------------===
;=== Sends a request to show a small message.                        ===
;===                                                                 ===
;=== void SmallMessage (char* message);                              ===
;=======================================================================

        global _SmallMessage
_SmallMessage:
        mov  bx, sp
        mov  bx, [ss:bx+02h]
        mov  ax, SMALLMESSAGE

        call CallHost

        ret

;=======================================================================
;===                        LargeMessage                             ===
;===-----------------------------------------------------------------===
;=== Sends a request to show a large message.                        ===
;===                                                                 ===
;=== void LargeMessage (char* message);                              ===
;=======================================================================

        global _LargeMessage
_LargeMessage:
        mov  bx, sp
        mov  bx, [ss:bx+02h]
        mov  ax, LARGEMESSAGE

        call CallHost

        ret

;=======================================================================
;===                    DrawOnDriveMap                               ===
;===-----------------------------------------------------------------===
;=== Draws on the drive map.                                         ===
;===                                                                 ===
;=== void DrawOnDriveMap(int cluster, int symbol);                   ===
;=======================================================================

        global _DrawOnDriveMap
_DrawOnDriveMap:

        push bp
        mov  bp, sp

        mov  cx, [bp+04h]
        mov  dx, [bp+06h]
        mov  ax, ONMAP

        call CallHost

        pop  bp
        ret

;=======================================================================
;===                      DrawDriveMap                               ===
;===-----------------------------------------------------------------===
;=== Draws the drive map.                                            ===
;===                                                                 ===
;=== void DrawDriveMap(unsigned maxcluster);                         ===
;=======================================================================                                                                     

        global _DrawDriveMap
_DrawDriveMap:

        mov  bx, sp

        mov  cx, [ss:bx+02h]
        mov  ax, DRAWMAP

        call CallHost
       
        ret
