;    
;   Absdsk.asm - absolute disk access.
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
;   email me at:  imre.leber@vub.ac.be
;

%macro doint 1
       mov   si, sp
       int   %1 
       cli
       mov   sp, si
       sti
%endmacro

segment ABSDSK_DATA 

        StartSector  dd 0
        count        dw 0
        BufferOffs   dw 0
        BufferSeg    dw 0

segment ABSDSK_TEXT 

;-------------------------------------------------------------------------
;---                            Setup12                                ---
;-------------------------------------------------------------------------
;--- Sets up the table before the call before absread12 and abswrite12 ---
;-------------------------------------------------------------------------

Setup12:
        mov  al, [bp+08h]
        mov  cx, [bp+0Ah]
        mov  dx, [bp+0Ch]
        mov  bx, [bp+0Eh]

        ret                             ;; Private (in same segment)

;=========================================================================
;===                            absread12                              ===
;===-------------------------------------------------------------------===
;=== int absread12(int drive, int nsects, unsigned lsect,              ===
;===               void* buffer);                                      ===
;=========================================================================
        
        global _absread12
_absread12:
        push  bp
        mov   bp, sp
        push  si

        call  Setup12
        doint 25h 

        mov   ax, 0
        jnc   .done

        dec   ax
.done:
        pop   si
        pop   bp
        retf

;=========================================================================
;===                            abswrite12                             ===
;===-------------------------------------------------------------------===
;=== int abswrite12(int drive, int nsects, unsigned lsect,             ===
;===               void* buffer);                                      ===
;=========================================================================

        global _abswrite12
_abswrite12:
        
        push  bp
        mov   bp, sp
        push  si

        call  Setup12
        doint 26h 

        mov   ax, 0
        jnc   .done

        dec   ax
.done:
        pop   si
        pop   bp
        retf

;--------------------------------------------------------------------------
;---                            Setup16                                 ---
;--------------------------------------------------------------------------
;--- Sets up the table before the call before absread16 and abswrite16. ---
;--------------------------------------------------------------------------

Setup16:
        push  ds
        mov   ax, ABSDSK_DATA
        mov   ds, ax
        
        mov   cx, [bp+0Ah]                 ;; nsects
        mov   [count], cx
        
        mov   ax, [bp+0Ch]                 ;; lsect
        mov   [StartSector], ax
        mov   ax, [bp+0Eh]                 
        mov   [StartSector+02h], ax

        mov   ax, [bp+10h]
        mov   [BufferOffs], ax             ;; void* buffer
        mov   ax, [bp+12h]
        mov   [BufferSeg], ax

        mov   al, [bp+08h]                 ;; drive

        mov   bx, StartSector
        
        pop   ds
        ret

;==========================================================================
;===                      absread16                                     ===
;===--------------------------------------------------------------------===
;=== int absread16 (int drive, int nsects, unsigned long lsect,         ===
;===                void* buffer);                                      ===
;==========================================================================

        global _absread16
_absread16:

        push  bp
        mov   bp, sp

        call  Setup16
        doint 25h 

        mov   ax, 0
        jnc   .done

        dec   ax
.done:
        pop   bp
        retf

;========================================================================
;===                       abswrite16                                 ===
;===------------------------------------------------------------------===
;===  int abswrite16(int drive, int nsects, unsigned long lsect,      ===
;===                 void* buffer);                                   ===
;========================================================================

        global _abswrite16
_abswrite16:

        push  bp
        mov   bp, sp
        
        call  Setup16
        doint 26h 

        mov   ax, 0
        jnc   .done

        dec   ax
.done:
        pop   bp
        retf

;==========================================================================
;===                       abs16supported                               ===
;===--------------------------------------------------------------------===
;=== int abs16supported(void);                                          ===
;===                                                                    ===
;=== Checks wether fat16 absread and abswrite are supported.            ===
;===                                                                    ===
;=== note: for pc-dos 3.31 this function can give the wrong result.     ===
;==========================================================================
        
        global _abs16supported
_abs16supported:

        mov  ah, 30h
        int  21h 
        
        cmp  ah, 4                 ; DOS version >= 4.0 => supported
        jae  .supported

        cmp  ah, 3                 ; DOS version != 3.31 => not supported
        jb   .notsupported
        cmp  al, 31
        jne  .notsupported

        cmp  bh, 0fdh               ; FreeDOS => supported
        je   .supported

        cmp  bh, 0                 ; DRDOS 6 => supported
        jne  .notsupported

.supported:
        mov  ax, 1
        jmp  short .EndOfProc

.notsupported:
        xor  ax, ax

.EndOfProc:
        retf
