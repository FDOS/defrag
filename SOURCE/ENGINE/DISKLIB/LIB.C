/*
 * lib,c        misc, stuff (see also win32\wlib.c)
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>
#include <stdlib.h>

extern int confirm(void)
{
#if LONG_INPUT          /* full word input only */
char buf[BUFSIZ];

    printf("\nAre you sure? [yes,No] ");
    fgets(buf,BUFSIZ,stdin);
    return strcmp(buf,"yes\n") == 0;

#else                   /* a 'y' will do */
int c;

    printf("\nAre you sure? [y,N] ");
    c = getchar();
    if (c != '\n')
        while (getchar() != '\n')
            ;
    return c == 'y';
#endif
}

extern int ask(const char *str)
{
#if LONG_INPUT          /* full word input only */
char buf[BUFSIZ];

    if (str)
        printf("%s",str);
    printf(" [yes,No] ");
    fgets(buf,BUFSIZ,stdin);
    return strcmp(buf,"yes\n") == 0;

#else                   /* a 'y' will do */
int c;

    if (str)
        printf("%s",str);
    printf(" [y,N] ");
    c = getchar();
    if (c != '\n')
        while (getchar() != '\n')
            ;
    return c == 'y';
#endif
}


extern void die(const char *msg, const char *fmt, int arg)
{
    printf("\n%s",msg);
    if (fmt)
        printf(fmt,arg);
    printf("\n");
    exit(1);
}
