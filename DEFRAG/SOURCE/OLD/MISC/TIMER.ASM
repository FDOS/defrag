;    
;   timer.asm - timer interrupt handler interface.
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


segment _DATA class=DATA
segment _TEXT class=CODE
        
        ;; in code segment so we access these from the interrupt handler
        ;; without setting ds.
        int13address DD 0   ;; address of int 13h handler.
        inint13      DB 0   ;; remember if in int 08h handler.

        int10address DD 0   ;; address if int 10h handler.
        inint10      DB 0   ;; remember if in int 10h handler.  
        
        realhandler  DW 0   ;; address of c function to call.
        
        int08address DD 0   ;; address of int 08h handler. 
        int08existed DB 0   ;; remembers wether int 08h allready had a 
                            ;; vector (just to be sure).

        indosaddress DD 0   ;; address of Indos flag.

;**************************************************************************
;***                       int10handler                                 ***
;**************************************************************************
;*** int 10 handler.                                                    ***
;***                                                                    ***
;*** Remember if int 10h has been called.                               ***
;**************************************************************************

int10handler:
        mov [cs:inint10], byte 1     
        pushf
        call far [cs:int10address]
        mov [cs:inint10], byte 0
        iret

;**************************************************************************
;***                       int13handler                                 ***
;**************************************************************************
;*** int 13 handler.                                                    ***
;***                                                                    ***
;*** Remember if int 13h has been called.                               ***
;**************************************************************************

int13handler:
        mov [cs:inint13], byte 1     
        pushf
        call far [cs:int13address]
        mov [cs:inint13], byte 0
        iret
            
;**************************************************************************
;***                       int08handler                                 ***
;**************************************************************************
;*** int 08 handler.                                                    ***
;***                                                                    ***
;*** Calls the old int 08h handler and if int 13h was not called calls  ***
;*** the given c function.                                              ***
;**************************************************************************

int08handler: 

        cmp [cs:int08existed], byte 0   ; ^
        je  .try13h                     ; |
                                        ; |
        pushf                           ; |  no registers get changed!
        call far [cs:int08address]      ; |
                                        ; |
.try13h:                                ; |
        cmp [cs:inint13], byte 1        ; |
                                        ; |
        jne .Ok                         ; |
        iret                            ; |
                                        ; v
.Ok:
        push ax
        push bx
        push cx
        push dx
        push bp
        push si
        push di
        push es
        push ds
        sti
        
        les  bx, [cs:indosaddress]
        cmp  [es:bx], byte 0
        jne  .StillDontCall

        mov  ax, _DATA
        mov  ds, ax

        call [cs:realhandler]

.StillDontCall:
        pop  ds
        pop  es
        pop  di
        pop  si
        pop  bp
        pop  dx
        pop  cx
        pop  bx
        pop  ax
        iret

;*************************************************************************
;***                     SetTimerInterrupt                             ***
;*************************************************************************
;*** void SetTimerInterrupt(void (*handler)(void));                    ***
;***                                                                   ***
;*** Sets up the c function to be called every 18.2 times a second.    ***
;*************************************************************************

        global _SetTimerInterrupt
_SetTimerInterrupt:

        push bp
        mov  bp, sp
        push ds

        mov  ah, 34h
        int  21h
        mov  ax, es
        mov  [cs:indosaddress+02h], ax        ;; Remember address indos flag.
        mov  [cs:indosaddress], bx

        mov  ax, 3510h                        ;; Get int 10h handler.
        int  21h                              

        mov  ax, es
        mov  [cs:int10address+02h], ax
        mov  [cs:int10address], bx            
        
        mov  ax, 3513h                        ;; Get int 13h handler.
        int  21h                              

        mov  ax, es
        mov  [cs:int13address+02h], ax
        mov  [cs:int13address], bx            

        mov  ax, 3508h                        ;; Get int 08h handler.
        int  21h
        
        mov  ax, es                           

        cmp  bx, 0
        jne  .Next
        cmp  ax, 0
        jne  .Next
        jmp  .Over

.Next:
        mov  [cs:int08existed], byte 1
        mov  [cs:int08address+02h], ax
        mov  [cs:int08address], bx

.Over:        
        mov  ax, [bp+04h]                     ;; Set real handler. 
        mov  [cs:realhandler], ax

        mov  ax, 2510h                        ;; Set int 10h handler.
        push cs
        pop  ds
        mov  dx, int10handler
        int  21h
        
        mov  ax, 2513h                        ;; Set int 13h handler.
        push cs
        pop  ds
        mov  dx, int13handler
        int  21h

        mov  ax, 2508h                        ;; Set int 08h handler.
        push cs
        pop  ds
        mov  dx, int08handler
        int  21h

        pop  ds
        pop  bp
        ret

;*************************************************************************
;***                     RenewTimerInterrupt                           ***
;*************************************************************************
;*** void RenewTimerInterrupt(void (*handler)(void));                  ***
;***                                                                   ***
;*** Sets up a new c function to be called every 18.2 times a second.  ***
;*************************************************************************

        global _RenewTimerInterrupt
_RenewTimerInterrupt:

        mov  bx, sp
        mov  ax, [ss:bx+02h]
        mov  [cs:realhandler], ax
        ret

;*************************************************************************
;***                     RestoreTimerInterrupt                         ***
;*************************************************************************
;*** void RestoreTimerInterrupt(void);                                 ***
;***                                                                   ***
;*** Restores the original interrupt handlers.                         ***
;*************************************************************************

        global _RestoreTimerInterrupt
_RestoreTimerInterrupt:

        push ds

        cmp  [cs:int08existed], byte 0
        je   .Next

        lds  dx, [cs:int08address]
        mov  ax, 2508h
        int  21h

.Next:
        lds  dx, [cs:int13address]
        mov  ax, 2513h
        int  21h

        lds  dx, [cs:int10address]
        mov  ax, 2510h
        int  21h

        pop  ds
        ret
