/*
   Hostslct.c - test file.

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

#include <stdio.h>

#include "ovlslave.h"

/*
** This file is intended for debugging purposes. Link with this file
** if you want a single executable, instead of an interface.
*/

int main(int argc, char** argv)
{
    return OvlMain(argc, argv);
}

void SendNOP(void)
{
     puts("Doing nothing.");
}

void SmallMessage (char* message)
{
     printf("small: %s\n", message);
}

void LargeMessage(char* message)
{
     printf("large: %s\n", message);
}

void DrawOnDriveMap(int cluster, int symbol)
{
     printf("Drawing on drive map, cluster: %d, symbol: %c",
            cluster, (char) symbol);
}

void DrawDriveMap(unsigned maxcluster)
{
     printf("Drawing drive map, clusters: %u", maxcluster);
}
