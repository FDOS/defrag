;    
;   Screen.asm - several screen routines.
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

segment _DATA class=DATA
        
        adapter_type db 3  ;; Remembers adapter type:
                           ;;   EGA = 1
                           ;;   VGA = 2 
                           
        colorval db 0      ;; Color value used by conio replacements.

segment _TEXT class=CODE        
        
;=========================================================================
;===                        HideCursor                                 ===
;===-------------------------------------------------------------------===
;===  Hides the cursor.                                                ===
;===                                                                   ===
;===  void HideCursor (void);                                          ===
;=========================================================================

        global _HideCursor
_HideCursor:

        mov  ah, 01h       ;; Define cursor shape.
        mov  cx, 2526h     ;; Scan lines out of screen. 
        int  10h            
        ret  

;=========================================================================
;===                        InvertChar                                 ===
;===-------------------------------------------------------------------===
;===  Inverts the color of the character on the current cursor         ===
;===  position.                                                        ===
;===                                                                   ===
;===  void InvertChar (void);                                          ===
;=========================================================================

        global _InvertChar
_InvertChar:

        mov  ah, 08h       ;; function: Read char and attr. on cursor pos.
        xor  bh, bh        ;; on screen page 0.
        int  10h
                           
        mov  bl, ah        
        mov  cl,  4
        shl  bl, cl        ;; Swap for and back color.
        shr  ah, cl
        add  bl, ah

        mov  ah, 09h       ;; Func: write attr. and char on cursor pos.
        mov  bh, 00h       ;; on screen page 0.
        mov  cx, 1         ;; write one char.
        int  10h            

        ret

;=========================================================================
;===                       SetHighIntensity                            ===
;===-------------------------------------------------------------------===
;===  Sets the blink bit on or off.                                    ===
;===                                                                   ===
;===  void SetHighIntensity(int onoff);                                ===
;===                                                                   ===
;===  Accepts: 1 to have high intensity, 0 not to have high intensity. ===
;=========================================================================
        
        global _SetHighIntensity
_SetHighIntensity:

        mov  bx, sp
        mov  bx, [ss:bx+02h]  ; Get parameter.
        not  bx               ; Interpret the parameter.
        and  bx, 1            
        mov  ax, 1003h        ; Set blink bit.
        int  10h
        ret

;=========================================================================
;===                       GetHighIntensity                            ===
;===-------------------------------------------------------------------===
;===  Returns wether the blink bit is on or off.                       ===
;===                                                                   ===
;===  int GetHighIntensity(void);                                      ===
;=========================================================================

        global _GetHighIntensity
_GetHighIntensity:

        xor  ax, ax
        mov  es, ax
        mov  bl, [es:465h]    ; BIOS address 0000:0465
        test bl, 20h

        jnz .EndOfProc

        mov  ax, 1

.EndOfProc:
        ret

;=========================================================================
;===                        GetCursorShape                             ===
;===-------------------------------------------------------------------===
;===  Returns a number indicating the shape of the cursor.             ===
;===                                                                   ===
;===  int GetCursorShape (void);                                       ===
;=========================================================================
        
        global _GetCursorShape
_GetCursorShape:

        mov  ah, 03h          
        mov  bx, 00h
        int  10h
        
        mov  ax, cx
        ret

;=========================================================================
;===                        SetCursorShape                             ===
;===-------------------------------------------------------------------===
;===  Sets the shape of the cursor as returned by GetCursorShape.      ===
;===                                                                   ===
;===  void SetCursorShape (int shape);                                 ===
;=========================================================================
        
        global _SetCursorShape
_SetCursorShape:
        
        mov  bx, sp
        mov  cx, [ss:bx+02h]  
        mov  ah, 01h
        int  10h

        ret

;=========================================================================
;===                        GetScreenCols                              ===
;===-------------------------------------------------------------------===
;===  Get the number of screen colons.                                 ===
;===                                                                   ===
;===  int GetScreenCols (void);                                        ===
;=========================================================================

        global _GetScreenCols
_GetScreenCols:

        mov  ah, 0Fh          ; Get screen mode.
        int  10h
        mov  al, ah
        xor  ah, ah           ; Screen cols in ax.

        ret

;=========================================================================
;===                        GetScreenLines                             ===
;===-------------------------------------------------------------------===
;===  Get the number of screen lines.                                  ===
;===                                                                   ===
;===  int GetScreenLines (void);                                       ===
;=========================================================================

        global _GetScreenLines
_GetScreenLines:
        push bp                ;; BP gets destroyed!

        xor  bh, bh
        mov  ax, 1130h         ;; Get information from character generator.
        xor  dl, dl
        int  10h

        cmp  dl, 0
        je   .Next
        inc  dl
        jmp  short .EndOfProc

.Next:
        mov  dl, 25

.EndOfProc:

        xor  dh, dh
        mov  ax, dx
        pop  bp
        ret

;=========================================================================
;===                              isEGA                                ===
;===-------------------------------------------------------------------===
;=== Check if we have at least EGA.                                    ===
;===    Return 1 if EGA, 2 if VGA/PGA/MCGA, else 0.                    ===
;===                                                                   ===
;===  int GetScreenLines (void);                                       ===
;=========================================================================

        global _isEGA
_isEGA:
        cmp  [adapter_type], byte 3
        jne  .RealEndOfProc

        mov  ax, 1a00h      ;; Ask Primary and secondary video-adapter.
        int  10h

        cmp  al, 1ah
        je   .tryVGA

        cmp  bl, 4          ;; look at the primary.
        je   .isEGA         ;; EGA with EGA or multisync monitor.
        cmp  bl, 5
        jne  .tryVGA        ;; EGA with monochrome monitor.

.isEGA:
        mov  ax, 1
        jmp  short .EndOfProc

.tryVGA:
        cmp  bl, 6            ;; PGA.
        je   .isVGA
        cmp  bl, 7            ;; VGA with analog monochrome monitor.
        je   .isVGA
        cmp  bl, 8            ;; VGA with analog color monitor.
        je   .isVGA
        cmp  bl, 10           ;; MCGA with CGA monitor.
        je   .isVGA
        cmp  bl, 11           ;; MCGA with analog monochrome monitor.
        je   .isVGA
        cmp  bl, 12           ;; MCGA with analog color monitor.
        jne  .noEGA
.isVGA:
        mov  ax, 2
        jmp  short .EndOfProc
                              ;; int 10, funct. 1Ah, subfunct. 00h not
                              ;; supported.

        mov  ah, 12h          ;; ask EGA configuration.
        mov  bl, 10h
        mov  bh, 0ffh
        int  10h
        cmp  bh, 0ffh
        je   .noEGA

        mov  ax, 1
        jmp  short .EndOfProc

.noEGA:
        mov  ax, 0

.EndOfProc:
        mov  [adapter_type], al

.RealEndOfProc:
        ret

;=========================================================================
;===               set_scan_lines_and_font (private).                  ===
;===-------------------------------------------------------------------===
;=== Sets scan lines and font.                                         ===
;===                                                                   ===
;===  in: dl : scan lines (0: 200, 1: 350, 2: 400).                    ===
;===      ax : font       (1111h: 8x14, 0x1112: 8x8, 0x1114: 8x16).    ===
;===                                                                   ===
;===  This routine is based on the sources of DJGPP's libc.            ===
;=========================================================================

set_scan_lines_and_font:
        push ax         ;; Remember font.

        mov  al, dl     ;; Set number of scan lines.
        mov  ah, 12h    ;; al : 0 => 200.
        mov  bl, 30h    ;;    : 1 => 350.
        int  10h        ;;    : 2 => 400.

        mov  ah, 0Fh    ;; ax = (current_mode == 7 ? 7 : 3)
        int  10h

        cmp  al, 7
        je   .Is7

        mov  al, 3
.Is7:
        xor  ah, ah        ;;Set screen mode!
        int  10h

        pop  ax            ;; Get font.
        xor  bl, bl
        int  10h           ;; Load ROM bios font.
        ret


;=========================================================================
;===                        SetScreenLines                             ===
;===-------------------------------------------------------------------===
;=== Sets screen lines.                                                ===
;===                                                                   ===
;=== void SetscreenLines(int nlines).                                  ===
;===                                                                   ===
;=== This routine is based on the sources of DJGPP's libc.             ===
;=========================================================================

        global _SetScreenLines
_SetScreenLines:
        push  bp
        mov   bp, sp
        push  ds

        call  _isEGA
        cmp   ax, 0             ;; Check if we have at least EGA.
        jne   .L02
        jmp   .EndOfProc
.L02:
        mov  dx, [bp+04h]

        ;;  ax = adapter type.
        ;;  dx = screen mode.

        cmp  dx, 25             ;; screen mode 25?
        jne  .try43

        push ax
        mov  bl, 30h            ;; Set scan lines.
        add  ax, 1200h
        int  10h
        pop  ax

        xor  bl, bl
        cmp  ax,  1             ;; Load ROM BIOS font.
        jne  .VGA01

        mov  al, 11h
        jmp  short .L01

.VGA01:
        mov  al, 14h            ;; 8x16 for VGA.

.L01:
        mov  ah, 11h            ;; 8x14 for EGA.

        int  10h

        mov  ah, 0fh
        int  10h
        cmp  al, 7
        je   .Is7

        mov  al, 3
.Is7:
        xor  ah, ah             ;;Set screen mode!
        int  10h
        jmp  short .EndOfProc


.try43:
        cmp  dx, 43             ;; screen mode 43?
        jne  .try28

        mov  dl, 1
        mov  ax, 1112h
        call set_scan_lines_and_font
        jmp  short .EndOfProc

.try28:
        cmp  ax, 2
        jne  .EndOfProc         ;; Check if we have at least VGA.

        cmp  dx, 28             ;; screen mode 28?
        jne  .try50

        mov  dl, 2
        mov  ax, 1111h
        call set_scan_lines_and_font
        jmp  short .EndOfProc


.try50:
        cmp  dx, 50             ;; screen mode 50?
        jne  .EndOfProc

        mov  dl, 2
        mov  ax, 1112h
        call set_scan_lines_and_font

.EndOfProc
        pop   ds
        pop   bp
        ret

;=========================================================================
;===                        DOSWipeScreen                              ===
;===-------------------------------------------------------------------===
;=== Clears the screen in the dos colors.                              ===
;===                                                                   ===
;=== void DOSWipeScreen(void).                                         ===
;=========================================================================

        global _DOSWipeScreen
_DOSWipeScreen:

        mov  ah, 0fh            ;; Get screen mode.
        int  10h
        mov  ah, 00h            ;; Set screen mode.
        int  10h
        
        mov  ah, 02h            ;; Change cursor position.
        xor  bh, bh             ;; on page 0.
        xor  dx, dx             ;; Line 0, column 0.
        int  10h
        
        ret

;=========================================================================
;===                      ReadCursorChar                               ===
;===-------------------------------------------------------------------===
;===  Reads the character on the cursor position.                      ===
;===                                                                   ===
;===  char ReadCursorChar (void);                                      ===
;===                                                                   ===
;===  Returns: character on the cursor position.                       ===
;=========================================================================
                                                                          
        global _ReadCursorChar 
_ReadCursorChar:
        
        mov  ah, 08h       ;; function: Read char and attr. on cursor pos.
        xor  bh, bh        ;; on screen page 0.
        int  10h            

        xor  ah, ah        ;; Ignore attribute.
        ret

;=========================================================================
;===                        ReadCursorAttr                             ===
;===-------------------------------------------------------------------===
;=== Reads the attribute on the current cursor position.               ===
;===                                                                   ===
;=== int ReadCursorAttr(void);                                         ===
;=========================================================================
        global _ReadCursorAttr
_ReadCursorAttr:
        
        mov  ah, 08h       ;; function: Read char and attr. on cursor pos.
        xor  bh, bh        ;; on screen page 0.
        int  10h            
        
        mov  al, ah        ;; Ignore char.
        xor  ah, ah
        ret

;==========================================================================
;===                        ChangeCursorPos                             ===
;===--------------------------------------------------------------------===
;=== Changes the position of the cursor.                                ===
;===                                                                    ===
;=== void ChangeCursorPos(int x, int y);                                ===
;==========================================================================

        global _ChangeCursorPos
_ChangeCursorPos:

        push bp
        mov  bp, sp

        mov  dx, [bp+04h]
        mov  bx, [bp+06h]
        mov  dh, bl
        xor  bx, bx

        sub  dx, 0101h

        mov  ah, 02h
        int  10h

        pop  bp
        ret


;==========================================================================
;===                        SetForColor                                 ===
;===--------------------------------------------------------------------===
;=== Set forground color.                                               ===
;===                                                                    ===
;=== void SetForColor(int color);                                       ===
;==========================================================================

        global _SetForColor
_SetForColor:

        mov  bx, sp
        mov  ax, [ss:bx+02h]

        and  al, 0Fh
        and  [colorval], byte 0F0h
        add  [colorval], al

        ret

;==========================================================================
;===                        SetBackColor                                ===
;===--------------------------------------------------------------------===
;=== Set background color.                                              ===
;===                                                                    ===
;=== void SetBackColor(int color);                                       ===
;==========================================================================

        global _SetBackColor
_SetBackColor:

        mov  bx, sp
        mov  ax, [ss:bx+02h]

        shl  al, 4
        and  [colorval], byte 0Fh
        add  [colorval], al

        ret

;==========================================================================
;===                      DrawChar                                      ===
;===--------------------------------------------------------------------===
;=== Puts a number of characters on the screen.                         ===
;===                                                                    ===
;=== void DrawChar(int asciichar, int repeat);                          ===
;===                                                                    ===
;=== repeat: the number of times the char needs to be written.          ===
;===                                                                    ===
;=== note: the cursorposition is not changed.                           ===
;==========================================================================

        global _DrawChar
_DrawChar:
        push  bp
        mov   bp, sp

        xor  bx, bx
        mov  bl, [colorval]
        mov  ax, [bp+04h]
        mov  cx, [bp+06h]

        mov  ah, 09h
        int  10h

        pop  bp
        ret

;==========================================================================
;===                      PrintChar                                     ===
;===--------------------------------------------------------------------===
;=== Puts a number of characters on the screen.                         ===
;===                                                                    ===
;=== void PrintChar(int asciichar, int repeat);                         ===
;===                                                                    ===
;=== repeat: the number of times the char needs to be written.          ===
;===                                                                    ===
;=== note: the cursorposition is changed.                               ===
;===       only works for text modus 80*25.                             ===
;===       the cursor is not advanced to the next line!                 ===
;==========================================================================

        global _PrintChar
_PrintChar:
        push bp
        mov  bp, sp

        mov  ax, [bp+06h]
        push ax
        mov  ax, [bp+04h]
        push ax

        call _DrawChar

        pop  ax
        pop  ax

        mov  ah, 03h
        xor  bh, bh
        int  10h

        add  dl, [bp+06h]
        mov  ah, 02h
        int  10h 

.EndOfProc
        pop  bp
        ret

;=========================================================================
;===                        PrintString                                ===
;===-------------------------------------------------------------------===
;=== Prints a string on the screen.                                    ===
;===                                                                   ===
;=== void PrintString(char* string);                                   ===
;=========================================================================

        global _PrintString
_PrintString:
        push di                          
        mov  bx, sp
        mov  di, [ss:bx+04h]             ;; Get pointer.
        
        mov  ax, 1
.L01:        
        cmp  [di], byte 0
        je   .EndOfProc

        push ax
        push word [di]
        
        call _PrintChar

        pop  ax
        pop  ax

        inc  di

        jmp  short .L01

.EndOfProc:
        pop  di
        ret

