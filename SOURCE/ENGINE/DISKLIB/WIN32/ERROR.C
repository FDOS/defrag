/*
 * win32\edump.c    display WIN32 error number
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
#include "dosio.h"
#include "disklib.h"

extern void err_dump(const char *str)
{
    if (str)
        printf("%s, ",str);
    poserror();
}

/*
 * os_error     get last OS error
 *
 */

extern int os_error(void)
{
    return (int)GetLastError();
}

/*
 * set_error        set OS error
 *
 */

extern void set_error(int error, int class, int action, int location)
{
    SetLastError(error);
}

/* DOS INT 21/59 error messages */

extern void doserror(int error)
{
static char *doserr_msg[] = {
   "Error 0",
   "Invalid function number",
   "File not found",
   "Path not found",
   "Too many open files",
   "Access denied",
   "Invalid handle",
   "Arena trashed",
   "Not enough memory",
   "Invalid memory block",
   "Bad environment",
   "Bad format",
   "Invalid access",
   "Invalid data",
   "Unknown error",
   "Invalid drive specified",
   "Attempt to remove CurDir",
   "Not same device",
   "No more files",
   "Write protect",
   "Bad unit",
   "Drive not ready",
   "Bad Command",
   "CRC Error",
   "Bad Length",
   "Seek Error",
   "Not DOS Disk",
   "other error"
};

    if (error < 0 || error > (int)(sizeof(doserr_msg)/sizeof(char*)))
        printf("error = %02xh",error);
    else
        printf("%s",doserr_msg[error]);
}

/* INT 25h/26h error translation */

extern void setdosioerror(int error)
{
int exterr;
static int diskioerr[] = {
    DOS_EWRITE, 0xf, DOS_ENREADY, DOS_EINVFNC,
    0x1, DOS_EINVDAT, 0x1b, DOS_EINVFMT, 0x1b, 0x1c,
    0x1d, 0x1e, 0x1f, 0x1f, 0x1f, 0x22
};

    if (error >= 0 && error <= 15)
        exterr = diskioerr[error];
    else
        exterr = 0x1f;

    set_error(exterr,0xb,0,2);
}


/*
 * poserror     a perror for Windows
 *
 */

extern void poserror(void)
{
LPVOID ebuf;
DWORD error;

    error = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,error,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&ebuf,0,NULL);

    printf("%s",ebuf);

    /* FormatMessage() appends a '\n' which may be annoying... */

    LocalFree(ebuf);
}

/*
 * stroserror       a strerror for Windows
 *
 */

extern const char *stroserror(void)
{
DWORD error;
LPVOID ebuf;
static sbuf[128];   /* is this large enough? */

    error = GetLastError();
    SetLastError(0);
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,error,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&ebuf,0,NULL);

    memcpy(sbuf,ebuf,strlen(ebuf)-2);

    LocalFree(ebuf);

    return (const char *)sbuf;
}
