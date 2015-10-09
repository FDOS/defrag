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

#include "dosio.h"
#include "disklib.h"
#include "debug.h"
#include "dosmem.h"

/*
 * err_dump     display DOS error information
 *
 * Note: This function is generally called by the "low-level" functions
 *       (such as IOCTL) when the library is compiled with TRACE defined.
 */

extern void err_dump(const char *str)
{
struct _DOSERROR d;

    if (str)
        printf("%s : ",str);
    _dosexterr(&d);
    printf("error: %d, class: %d, action: %d, locus: %d\n",
            d.exterror,d.class,d.action,d.locus);
}

/*
 * os_error     get last OS error
 *
 */

extern int os_error(void)
{
struct _DOSERROR d;

    return _dosexterr(&d);
}

extern void poserror(void)
{
    doserror(os_error());
}

/*
 * set_error        set OS error
 *
 */

extern void set_error(int error, int class, int action, int location)
{
__dpmi_regs regs DBG_0;
int seg,sel;
struct errinfo {
    unsigned short ax;
    unsigned short bx;
    unsigned short cx;
    unsigned short dx;
    unsigned short na[7];
} err = {0};

    if ((seg = _dosmalloc(sizeof(struct errinfo),&sel)) == -1)
        return;

    _setdosmem(&err,sizeof(struct errinfo),sel);

    err.ax = error;
    err.bx = (class << 8) | action;
    err.cx = location << 8;

    regs.x.ax = 0x5d0a;
    regs.x.ds = seg;
    regs.x.dx = 0;
    __dpmi_int(0x21,&regs);
    dosfree(sel);

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
   "Non DOS Disk",
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
