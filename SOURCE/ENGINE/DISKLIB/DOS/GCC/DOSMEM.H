/*
 * dos\gcc\dosmem.h
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <dos.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/movedata.h>

/* link with dosmem.c or use the macros */

int _dosmalloc(int size, int *sel);
void _setdosmem(void *buffer, int size, int selector);
void _getdosmem(void *buffer, int size, int selector);
void _dosfree(int selector);

#define dosmalloc(siz,sel)	  __dpmi_allocate_dos_memory((siz+15)>>4,sel)
#define setdosmem(b,siz,sel)  _movedatab(_my_ds(),(unsigned int)b,sel,0,siz)
#define getdosmem(b,siz,sel)  _movedatab(sel,0,_my_ds(),(unsigned int)b,siz)
#define dosfree(sel)		  __dpmi_free_dos_memory(sel)
#define _MK_FP(seg) 		  (void *)(seg << 16)
