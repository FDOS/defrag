/*
 * dos\gcc\dosmem.c     alloc/free conventional memory
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>
#include <signal.h>

#include "dosmem.h"

/* this is not a real nice way of handling lack of memory... */

void xabort(void)
{
    fprintf(stderr,"\nWhew... am I exhausted!\n");
    raise(SIGABRT);
    exit(1);
}

int _dosmalloc(int size, int *sel)
{
int seg;

    seg = __dpmi_allocate_dos_memory((size+15)>>4,sel);
    if (seg == -1)
        xabort();
    return seg;
}

void _setdosmem(void *buffer, int size, int selector)
{
    _movedatab(_my_ds(),(unsigned int)buffer,selector,0,size);
}

void _getdosmem(void *buffer, int size, int selector)
{
    _movedatab(selector,0,_my_ds(),(unsigned int)buffer,size);
}

void _dosfree(int selector)
{
    __dpmi_free_dos_memory(selector);
}
