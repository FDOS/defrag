/*
 * win32\ver.s      version numbers (not finished)
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdlib.h>

#include "win32.h"
#include "disklib.h"

/*
 * lib_ver      version of DISKLIB
 *
 * Note: Currently returns which OS compiled on.
 *
 */

extern int lib_ver(void)
{
#ifdef _WINNT
    return LIB_WINNT;
#else
    return LIB_WIN95;
#endif
}

/*
 * dos_ver      DOS(BOX) version
 *
 */

extern int dos_ver(void)
{
#ifdef __WATCOMC__
    return (_osmajor << 8) | _osminor;
#else
    return 0;
#endif
}

/*
 * win_ver      Windows version
 *
 */

extern int win_ver(void)
{
#if defined _MSC_VER || (defined __WATCOMC__ && __WATCOMC__ > 1060)
    return (_winmajor << 8) | _winminor;
#else
int v;
OSVERSIONINFO ver;

    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&ver);
    v =  (int)ver.dwMajorVersion << 8;
    v |= (int)ver.dwMinorVersion;

    return v;
#endif
}
