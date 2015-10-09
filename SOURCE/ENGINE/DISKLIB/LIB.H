/*
 * lib.h    misc. stuff
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */


extern int confirm(void);
extern int ask(const char *str);
extern void die(const char *msg, const char *fmt, int arg);

#ifdef _WIN32
extern int kbhit(void);         /* win32\wlib.c */
extern int getch(void);         /* " */
#else
#include <conio.h>              /* getch(), kbhit() */
#endif
