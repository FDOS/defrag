/*
   Hostslct.c - host selection interface.

   Copyright (C) 2000, Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be

*/

#include <dos.h>
#include <stdio.h>

#include "expected.h"
#include "ovlhost.h"
#include "protocol.h"

#define BUFSIZE 128

static void CopyToBuffer(char* buffer, char far* cfptr);

int SelectHostRoutine(int ax, int bx, int cx, int dx, int ds)
{
    char buffer[BUFSIZE];         /* Take buffer from stack! */
    char far* cfptr;

    UpdateInterfaceState();

    switch (ax)
    {
      case NOP: 
           break;
    
      case SMALLMESSAGE:
           cfptr = MK_FP(ds, bx);
           CopyToBuffer(buffer, cfptr);
           SmallMessage(buffer);
           break;

      case LARGEMESSAGE:
           cfptr = MK_FP(ds, bx);
           CopyToBuffer(buffer, cfptr);
           LargeMessage(buffer);
           break;

      case ONMAP:                   /* cx = cluster, dx = symbol. */
           DrawOnDriveMap(cx, dx);
           break;

      case DRAWMAP:                 /* cx = amount of clusters on drive. */
           DrawDriveMap(cx);
           break;

      default:
           puts("Unknown message send to host!");
    }

    return 0;
}

static void CopyToBuffer(char* buffer, char far* cfptr)
{
    int position = 0;

    while ((position++ < BUFSIZE) && ((*buffer++ = *cfptr++) != 0));
}
