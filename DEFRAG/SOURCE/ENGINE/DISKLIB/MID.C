/*
 * mid.c        set media type
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
#include <string.h>
#include <ctype.h>

#include <conio.h>

#include "dosio.h"
#include "disklib.h"

void display(struct MID *mid);
int getlong(unsigned long *v);
int getstring(char *s, int len);
int getstringx(char *s, int len);

int main(int argc, char **argv)
{
int i,j,disk;
char tmp[12];
struct MID mid = {0};

    /*
        This program will allow you to change a disk's serial number
        or volume label. (Except only the serial number on NT.)

        Note that the BOOT sector volume label is not the same as the
        one created by the LABEL command.

    */

#if defined __WATCOMC__ || defined __GNUC__
   setvbuf(stdout,NULL,_IONBF,0);      /* flush stdout every printf */
#endif

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        disk = get_drive();

    printf("\nDrive %c:\n",disk+'A');

    if ((i = disk_getmedia(disk,&mid)) != DISK_OK)
    {
        lib_error("mid",i);
    }
    else
    {
        display(&mid);
        printf("\nChange? (s,v) ");
        i = getch();
        if (i == 's' || i == 'v')
        {
            switch (i) {
                case 's':
                    printf("\rSerial Number: ");
                    j = getlong(&mid.serialnum);
                    break;
                case 'v':
                    printf("\rVolume Label: ");
                    j = getstring(tmp,11);
                    if (j > 0) {
                        memset(&mid.vollabel,' ',11);
                        memcpy(&mid.vollabel,tmp,strlen(tmp));
                    }
                    break;
                default:
                    j = 0;
                    break;
            }
            if (j > 0) {
                printf("\nwrite ");
                if (i == 'v') printf("%s",tmp);
                if (i == 's') printf("%lx",mid.serialnum);
                printf(" [y/N] ");
                if (getch() == 'y')
                    disk_setmedia(disk,&mid);
            }
        }
    }

    printf("\n");
    return 0;
}

void display(struct MID *mid)
{
    printf("\nMedia ID\n\n");
    printf("File System:        %.8s\n",mid->filesys);
    printf("Serial Number:      %lx\n",mid->serialnum);
    printf("Volume Label:       %.11s\n",mid->vollabel);
}

int getlong(unsigned long *v)
{
long l,t;
char buf[10];

    if (getstringx(buf,8) < 0)
        return -1;

    l = t = 0;
    switch (sscanf(buf,"%lx-%lx",&l,&t)) {
        case 1:
            break;
        case 2:
            l <<= 16;
            l |= (t & 0xFFFF);
            break;
        default:
            return -1;
            break;
    }

    *v = l;
    return 1;
}

int getkey(void)
{
int c;

	skipext:
	if ((c = getch()) == 0) {
		getch();
		goto skipext;
	}
	return c;
}


int getstring(char *s, int len)
{
int c,i;

   for (c = i = 0; ; )
   {
	  if ((c = getkey()) == '\033' || c == '\r')
         break;

	  if (c == '\b' && i) {
		 printf("\b \b");
		 --i;
	  }

      if (i == len)
        continue;

      if (c < 0x100 && isprint(c))     /* ctype args must be < 255 */
      {
         putchar(c);                   /* echo and save */
         s[i++] = (char)c;
      }
   }
   s[i] = '\0';                        /* terminate buffer */
   return (c == '\033') ? -2 : (i-1);  /* -1 for no input */
}

int getstringx(char *s, int len)
{
int c,i;

   for (c = i = 0; ; )
   {
	  if ((c = getkey()) == '\033' || c == '\r')
         break;

	  if (c == '\b' && i) {
		 printf("\b \b");
		 --i;
	  }
      if (i == len)
        continue;

      if (c < 0x100 && isxdigit(c))    /* ctype args must be < 255 */
      {
         putchar(c);                   /* echo and save */
         s[i++] = (char)c;
      }
   }
   s[i] = '\0';                        /* terminate buffer */
   return (c == '\033') ? -2 : (i-1);  /* -1 for no input */
}
