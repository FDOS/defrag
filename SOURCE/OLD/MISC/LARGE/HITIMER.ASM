;    
;   hitimer.asm - specific implementation of the timer interrupt handler
;                 for this application.
;
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


%assign TIMETICKS   18        ;; adjust time on screen every 16 clock ticks.
%assign DIALOGTICKS 45        ;; put the dialog of the screen after 45 
                              ;; clock ticks.

extern _RestoreTimerInterrupt
extern _SetTimerInterrupt
extern _PasteScreen
extern _DrawTime
extern _UnhideMapArea

;**********************************************************************
;***                      HideDialog                                ***
;**********************************************************************
;*** Macro to hide a dialog box.                                    ***
;**********************************************************************

%macro HideDialog 0
       xor  ax, ax
       push ax
       push ax
       call far _PasteScreen   
       pop  ax
       pop  ax

       call far _UnhideMapArea
%endmacro      

;**********************************************************************
;***                      AdjustTime                                ***
;**********************************************************************
;*** Macro to set the elapsed time on the screen.                   ***
;**********************************************************************

%macro AdjustTime 0
       mov  ah, 2Ch
       int  21h

       xor  ax, ax
       mov  al, ch      ; ax = hours
       xor  ch, ch      ; cx = minutes
       mov  dl, dh
       xor  dh, dh      ; dx = seconds

       cmp  dl, [startseconds]
       jb   .borow1

       sub  dl, [startseconds]
       jmp  .calcminutes
.borow1:

       dec  cx
       add  dx, 60
       sub  dl, [startseconds]
.calcminutes:

       cmp  cl, [startminutes]
       jb   .borow2

       sub  cl, [startminutes]
       jmp  .calchours

.borow2:
       dec  ax
       add  cx, 60
       sub  cl, [startminutes]

.calchours:
       sub  al, [starthours]

.donecalcing:

       push dx                      ; seconds
       push cx                      ; minutes
       push ax                      ; hours
       call _DrawTime       
       pop  ax
       pop  ax
       pop  ax
%endmacro

;================================ DATA ======================================

segment _DATA class=DATA

        timeticker    db TIMETICKS
        dialogticker  db DIALOGTICKS 

        dialogvisible  db 0            ;; Dialog on screen?
        timerinstalled db 0            ;; Timer installed?

        starthours     db 0
        startminutes   db 0            ;; base time.
        startseconds   db 0

;================================ CODE ======================================

segment _TEXT class=CODE

;***************************************************************************
;***                               OnTimer                               ***
;***************************************************************************
;*** Our timer handler.                                                  ***
;***                                                                     ***
;*** Sets the elapsed time on the screen and hides any dialogs after a   ***
;*** period of time.                                                     ***
;***************************************************************************

OnTimer:
        dec byte [timeticker]
        cmp byte [timeticker], 0
        jne .tryDialogTicker

        mov  [timeticker], byte TIMETICKS
        AdjustTime 
        
.tryDialogTicker:

        cmp [dialogvisible], byte 0
        je  .EndOfProc

        dec byte [dialogticker]
        cmp [dialogticker], byte 0
        jne .EndOfProc

        mov [dialogvisible], byte 0
        mov [dialogticker], byte DIALOGTICKS 
        HideDialog

.EndOfProc:
        ret

;***************************************************************************
;***                            IsDialogVisible                          ***
;***************************************************************************
;*** int DialogVisible(void);                                            ***
;***                                                                     ***
;*** Returns wether if any dialog has been taken off screen.             ***
;***************************************************************************

        global _IsDialogVisible
_IsDialogVisible:

        mov  al, [dialogvisible]
        xor  ah, ah

        ret

;***************************************************************************
;***                         SetDialogVisibility                         ***
;***************************************************************************
;*** void SetDialogVisibility(int visible);                              ***
;***                                                                     ***
;*** Tells the handler that there is a dialog that has to be taken off   ***
;*** screen.                                                             ***
;***************************************************************************

        global _SetDialogVisibility
_SetDialogVisibility:

        mov  [dialogticker], byte DIALOGTICKS

        mov  bx, sp
        mov  ax, [ss:bx+02h]
        mov  [dialogvisible], al
        ret

;***************************************************************************
;***                         SetTimerOff                                 ***
;***************************************************************************
;*** void SetTimerOff(void);                                             ***
;***                                                                     ***
;*** Sets the timer off if it has been installed.                        ***
;***************************************************************************

        global _SetTimerOff
_SetTimerOff:
        cmp [timerinstalled], byte 0
        je  .EndOfProc

        call _RestoreTimerInterrupt
        mov  [timerinstalled], byte 0

.EndOfProc:
        ret

;***************************************************************************
;***                         SetTimerOn                                  ***
;***************************************************************************
;*** void SetTimerOn(void);                                              ***
;***                                                                     ***
;*** Sets the timer on if it has not allready been installed.            ***
;***************************************************************************

        global _SetTimerOn
_SetTimerOn:

        cmp [timerinstalled], byte 1
        je  .EndOfProc

        mov  ah, 2Ch
        int  21h
        mov  [starthours], ch
        mov  [startminutes], cl
        mov  [startseconds], dh

        mov  ax, OnTimer
        push ax
        call _SetTimerInterrupt
        pop  ax

        mov  [timerinstalled], byte 1

.EndOfProc:
        ret
