/*
 * win32\wlib.c     misc. stuff for Windows
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>
#include "win32.h"

static int iscontrolkey(int vkey);

#define InRange(v,a,b)  ((v) >= (a) && (v) <= (b))

int kbhit(void)
{
HANDLE hStdIn;
INPUT_RECORD inputBuffer;
DWORD dwInputEvents;

    inputBuffer.EventType = 0;      /* Must Do This! */

    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    PeekConsoleInput(hStdIn,&inputBuffer, 1, &dwInputEvents);

    if (inputBuffer.EventType == KEY_EVENT)
    {
        if (inputBuffer.Event.KeyEvent.bKeyDown)
        {
            if (inputBuffer.Event.KeyEvent.uChar.AsciiChar != 0 ||
                !iscontrolkey(inputBuffer.Event.KeyEvent.wVirtualKeyCode))
            {
#if 0   /* test */
                printf(" key");
                printf(" % 3d",inputBuffer.Event.KeyEvent.bKeyDown);
                printf(" '%c' ",inputBuffer.Event.KeyEvent.uChar.AsciiChar);
                printf(" % 3d ",inputBuffer.Event.KeyEvent.uChar.AsciiChar);
                printf(" %02x %02x  %05lx",
                                     inputBuffer.Event.KeyEvent.wVirtualKeyCode,
                                     inputBuffer.Event.KeyEvent.wVirtualScanCode,
                                     inputBuffer.Event.KeyEvent.dwControlKeyState);
                printf(" %d",inputBuffer.Event.KeyEvent.wRepeatCount);
#endif
                return 1;
            }
        }

        /* why? I forget. */
        //FlushConsoleInputBuffer(hStdIn);    /* MUST eat non-key keys! */

        ReadConsoleInput(hStdIn, &inputBuffer, 1, &dwInputEvents);
    }
    return 0;
}

int getch(void)
{
HANDLE hStdIn;
INPUT_RECORD inputBuffer;
DWORD dwInputEvents;
static int ext;

    if (ext)
    {
        inputBuffer.Event.KeyEvent.uChar.AsciiChar = ext;
        ext = 0;
        return (int)(unsigned char)inputBuffer.Event.KeyEvent.uChar.AsciiChar;
    }

    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    for (;;)
    {
        ReadConsoleInput(hStdIn, &inputBuffer, 1, &dwInputEvents);
        if (inputBuffer.EventType == KEY_EVENT)
        {
            if (inputBuffer.Event.KeyEvent.bKeyDown)
            {
                if (inputBuffer.Event.KeyEvent.uChar.AsciiChar != 0 ||
                    !iscontrolkey(inputBuffer.Event.KeyEvent.wVirtualKeyCode))
                {
                    if ((inputBuffer.Event.KeyEvent.dwControlKeyState |
                        (LEFT_ALT_PRESSED & RIGHT_ALT_PRESSED)) ||
                        inputBuffer.Event.KeyEvent.uChar.AsciiChar == 0)
                    {
                        ext = inputBuffer.Event.KeyEvent.wVirtualScanCode;
                        inputBuffer.Event.KeyEvent.uChar.AsciiChar = 0;
                    }

                    break;
                }
            }
        }
    }
    return (int)(unsigned char)inputBuffer.Event.KeyEvent.uChar.AsciiChar;
}

static int iscontrolkey(int vkey)
{
    return

    vkey == VK_SHIFT || vkey == VK_CONTROL || vkey == VK_MENU ||
    vkey == VK_NUMLOCK || vkey == VK_SCROLL || vkey == VK_CAPITAL;

}
