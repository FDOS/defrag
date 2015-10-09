/*
 * err.c        display library function error type
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

/*
    Note: The function here is not finished. And there really
          should be a second function that returns a `const
          char *'.
*/

#include <stdio.h>

#include "dosio.h"
#include "disklib.h"

int disk_errno = 0;

/*
 * lib_error    get basic error type
 *
 */

extern void lib_error(const char *s, int i)
{
    if (s)
        printf("%s: ",s);

    if (i > 0)
        doserror(i);

    else if (i == 0)
        printf("error zero");

    else if (i < 0) {
        switch (i) {
            case DOS_ERR:
                doserror(os_error());
                break;
            case BIOS_ERR:
                printf("%s","bios");        /* not finished */
                break;
            case MEM_ERR:
                printf("%s","memory");      /* not finished */
                break;
            case DISK_ERR:
                printf("%s","disk");        /* not finished */
                break;
            case DEVICE_ERR:
                poserror();
                break;
            default:
                printf("unknown lib error");
                break;
        }
        printf("\n");
    }
}
