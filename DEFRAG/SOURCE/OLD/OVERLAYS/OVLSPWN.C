/*
   OVLSPWN.C - routine to spawn an overlay.

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

#include <string.h>
#include <stdlib.h>
#include <process.h>

#include "overlays.h"
#include "ovlhost.h"

/*
** Spawns an overlay.
**
** Input: overlay: overlay to spawn.
**        parameters: parameters for the overlay.
*/

static int CountParameters(char* parameters);

int SpawnOverlay(char* overlay, char* parameters)
{
    int    size, i, result, segment, offset; 
    char   SegmentString[6];
    char   OffsetString[6];
    char** argv;
    char*  argvs; 
    int    argc = CountParameters(parameters) + 5;
    char*  index, *index1;

    SaveHostState();

    /* Calculate the length of the parameter string. */
    GetDispatcher(&segment, &offset);
    itoa(segment, SegmentString, 10);
    itoa(offset, OffsetString, 10);

    size = strlen(overlay)       + 1
	 + 4                                   /* strlen(ovlcode) + 1*/ 
	 + strlen(SegmentString) + 1
	 + strlen(OffsetString)  + 1
	 + strlen(parameters)    + 1 
	 + 1;                                  /* NULL */

    /* Allocate memory for the parameter string. */
    index = argvs = malloc(size); if (!index) return -1;

    argv = malloc(argc * sizeof(char*));
    if (!argv)
    {
       free(index);
       return -1;
    }
    
    /* Copy data to the parameter string. */
    strcpy(index, overlay);
    index += strlen(index)         + 1;
    strcpy(index, OVLIDCODE);
    index += strlen(OVLIDCODE)     + 1;
    strcpy(index, SegmentString);
    index += strlen(SegmentString) + 1;
    strcpy(index, OffsetString);
    index += strlen(OffsetString)  + 1;

    strcpy(index, parameters);
    index1 = strchr(index, 0) + 1;
    
    for (i = 4; i < argc-1; i++)
    {
	index = strchr(++index, ' ');
	if (index) *index = 0;
    }
    *index1 = NULL;

    /* Fill in the array to argument string (argv). */
    argv[0] = index = argvs;
    for (i = 1; i < argc; i++)
    {
	index   = strchr(index, 0);
	argv[i] = ++index;
    }

    /* Spawn overlay. */
    result = spawnvp(P_WAIT, overlay, argv);

    /* Free memory. */
    free (argvs);
    free (argv);

    return result;
}

static int CountParameters(char* parameters)
{
   int count = 1;

   if ((parameters == NULL) || (parameters[0] == 0)) return 0;

   while ((parameters = strchr(++parameters, ' ')) != NULL) count++;

   return count;
}
