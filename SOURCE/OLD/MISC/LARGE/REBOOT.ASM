;    
;   Boot.asm - routines to reboot the computer (cold & warm).
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

segment _TEXT class=CODE

        jumpaddress dd 0ffff0000h

;=========================================================================
;===                             WarmReboot                            ===
;=========================================================================
;=== void WarmReboot(void);                                            ===
;===                                                                   ===
;=== Performs a warm reboot.                                           ===
;=========================================================================

        global _WarmReboot
_WarmReboot:

        mov  ax, 40h               ; put 1234h at address 0040:0072
        mov  ds, ax
        mov  [72h], word 1234h 
        
        jmp far [cs:jumpaddress]   ; jump to address ffff:0000
        
;=========================================================================
;===                             ColdReboot                            ===
;=========================================================================
;=== void ColdReboot(void);                                            ===
;===                                                                   ===
;=== Performs a warm reboot.                                           ===
;=========================================================================

        global _ColdReboot
_ColdReboot:

        mov  ax, 40h               ; put 0 at address 0040:0072
        mov  ds, ax
        mov  [72h], word 0 
        
        jmp far [cs:jumpaddress]   ; jump to address ffff:0000
