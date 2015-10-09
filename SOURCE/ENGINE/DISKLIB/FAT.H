/*
 * fat.h
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <limits.h>

#if LONG_MAX > INT_MAX
#define NEED_LARGE
#define LARGE __huge                /* a necessary evil for 16 bit DOS */
#else
#define LARGE
#endif

unsigned short LARGE *readfat(int disk, unsigned short nclusters, long sector,
                              int secsfat);
unsigned short searchfat12(int disk, int nclusters, long sector, int secsfat,
                           unsigned short value);
unsigned short searchfat16(int disk, unsigned short nclusters, long sector,
                           int secsfat, unsigned short value);
