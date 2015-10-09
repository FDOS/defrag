/*
 * winnt\open.c     open drive
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
#include "debug.h"

extern HANDLE opendrivephy(int disk)
{
HANDLE h;
char buf[64];

    if (disk < 2)
        sprintf(buf,"\\\\.\\%c:",disk+'A');
    else
        sprintf(buf,"\\\\.\\PHYSICALDRIVE%c",disk+'0'-2);

    h = CreateFile(buf, GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, 0, NULL);

    DBG_err_dump("open(phys)");

    return h;

}

extern HANDLE opendrive(int disk)
{
HANDLE h;
char buf[] = "\\\\.\\C:";

    buf[4] = (char)(disk+'A');

    h = CreateFile(buf, GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, 0, NULL);

    DBG_err_dump("open");

    return h;
}

extern void closedrive(HANDLE dev)
{
    CloseHandle(dev);
}
