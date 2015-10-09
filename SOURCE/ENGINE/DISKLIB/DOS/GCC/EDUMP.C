/*
 * dos\gcc\edump.c      display DOS error information
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>
#include <dos.h>

void err_dump(const char *str)
{
struct _DOSERROR d;

    if (str)
        fprintf(stderr,"%s : ",str);
    _dosexterr(&d);
    fprintf(stderr,"error %d (class: %d, action: %d, locus: %d)\n",
            d.exterror,d.class,d.action,d.locus);
}
