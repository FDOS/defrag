/*
   DOSEMU.C - routines to check wether a DOS program is running under
	      linux DOSemu.

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

/*
 * This file is based on detect.h which is used
 * by DOSEMU supplied DOS-tools to detect DOSEMU and has been released with  
 * the copyright notice:
 *
 * (C) Copyright 1992, ..., 1999 the "DOSEMU-Development-Team".
 *
 * for details see file COPYING in the DOSEMU distribution
 *
 * Copright (C) under GPL, The DOSEMU Team 1997
 * Author: Hans Lermen@fgan.de
 *
 */

#include "DOSemu.h"

static int old_dosemu_compatibility(void)
{
    static unsigned long far *given = (void far *) DOSEMU_BIOS_DATE_LOCATION;
    static unsigned long *expected = (unsigned long *)DOSEMU_BIOS_DATE;

    return (given[0] == expected[0] && given[1] == expected[1]);
}

unsigned long is_dosemu(void)
{
    static struct dosemu_detect far *given = (void far *) DOSEMU_MAGIC_LOCATION;
    static struct dosemu_detect expected = {DOSEMU_MAGIC};

    if (   (expected.u.magic[0] == given->u.magic[0])
	   && (expected.u.magic[1] == given->u.magic[1]) )
		    return given->version;
    return old_dosemu_compatibility();
}
