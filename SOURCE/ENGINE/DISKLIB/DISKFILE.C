/* DISKFILE.C - file to disk / disk to file using DISKLIB

   An "industry standard" floppy image writer/reader. Image format
   is track 0, side 0; track 0, side 1; track 1, side 0; etc.

   version 2.0    05-Nov-1998    uses DISKLIB
   version 1.1    27-Feb-1998    better error handling

   BUGS:    In the functions trackread() and trackwrite(), there is
            a bug: if the attempt to read/write an entire track fails,
            and the track is handled by sectors, the program will crash
            with DJGPP, and Watcom and Microsoft if the latter two are
            compiled in the small model. Ideas anyone?

*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include <bios.h>                /* _DISK.... */
#include <dos.h>                 /* REGS */
#include <conio.h>               /* getch() */

#include "dosio.h"
#include "disklib.h"

#ifdef __WATCOMC__
#define _open  open
#define _read  read
#define _write write
#define _close close
#endif

int file2disk(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer);
int disk2file(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer);
int trackread(int disk, int track, int head, int nsectors, char *buffer);
int trackwrite(int disk, int track, int head, int nsectors, char *buffer);
void die(const char *msg, const char *fmt, int arg);
void ctrlc(int sig);
void outchar(int c);  /* INT 10h output with translations/cursor move */
void charout(int c);  /* INT 10h output no traslations/no cursor move */


jmp_buf Jmpbuf;                     /* abort place holder */
int Keep;                           /* keep going on bad sectors */


int main(int argc, char **argv)
{
int fd,i;
int disk,file;
char *buffer;
size_t bufsz;
int tracks,sectors,heads;

   if (argc < 3)
   {
      printf("\nusage: diskfile <file> <drive>\n");
      printf("       diskfile <drive> <file>\n");
      printf("\nwrite file to floppy, or write floppy to file\n");
      return 1;
   }

   if (argv[1][1] == ':' && argv[1][2] == '\0')          /* 1st is drive */
   {
      disk = toupper(argv[1][0]) - 'A';
      file = 2;
   }
   else if (argv[2][1] == ':' && argv[2][2] == '\0')     /* 2nd is drive */
   {
      disk = toupper(argv[2][0]) - 'A';
      file = 1;
   }
   else
   {
      printf("\ndrive specification not found (%s, %s)\n",argv[1],argv[2]);
      return 1;
   }

   if (disk < 0 || disk > 1)    /* floppies only */
   {
      printf("\ndrive specification not valid (%c:)\n",disk+'A');
      return 1;
   }

   if ((i = disk_get_physical(disk,&tracks,&sectors,&heads)) != DISK_OK)
      die("BIOS error: ",disk_error(i),0);

   if ((buffer = malloc(bufsz = sectors * 512)) == NULL)
      die("not enough memory found!"," (%u)",bufsz);

   printf("\nreseting controller... ");
   disk_reset(disk);

   printf("homing disk...");
   if ((i = disk_read_p(disk,0,1,0,buffer,1)) != DISK_OK)
      die("BIOS error: ",disk_error(i),0);

   if (file == 1) {
      printf("\rwriting \"%s\"",argv[file]);
      printf(" to %c: (%u tracks and %u sides)",disk+'A',tracks+1,heads+1);
   }
   else {
      printf("\rreading %c: (%u tracks and %u sides)",disk+'A',tracks+1,heads+1);
      printf(" to \"%s\"",argv[file]);
   }

   printf("; continue? [y/n/^C] ");
   while ((i = getch()) && i != 'y' && i != 'n')
       ;
   if (i == 'n')
      return 1;

   if (file == 2)                                     /* writing file */
   {
      if ((fd = _open(argv[file],_O_RDONLY)) != -1)
      {
         _close(fd);
         printf("\nfile %s exists; overwrite? [y/n/^C] ",argv[file]);
         while ((i = getch()) && i != 'y' && i != 'n')
             ;
         if (i == 'n')
            return 1;
         putchar('\r');
      }
      fd = _open(argv[file],O_CREAT|O_RDWR|O_BINARY|O_TRUNC,S_IREAD|S_IWRITE);
   }
   else
   {
      fd = _open(argv[file],O_RDWR|O_BINARY);
   }

   if (fd == -1)
      die("file error: ",strerror(errno),errno);

   Keep = 1;

   errno = 0;

   signal(SIGINT, ctrlc);

   /* on ^C we get back here with setjmp() returning non-zero */

   if (setjmp(Jmpbuf) == 0)
   {
      /* draw a line */

      for (i = 0; i <= tracks; i++)
      {
         if (i < 79)
            outchar('.');
         else
            charout('.');   /* (last char shouldn't move cursor) */
      }
      outchar('\r');

      /* now that everything is verified and prepared, this is the meat */

      if (file == 1)
         file2disk(fd,disk,tracks,heads,sectors,bufsz,buffer);
      else
         disk2file(fd,disk,tracks,heads,sectors,bufsz,buffer);

      printf("\nDone.");

      /* that's not hard is it... */
   }

   _close(fd);

   if ((i = disk_status(disk)) != 0)
      die("BIOS error: ",disk_error(i),0);

   if (errno)
      die("file error: ",strerror(errno),errno);

   printf("\nreseting controller... ");
   disk_reset(disk);
   printf(" homing disk...\n");
   disk_read_p(disk,0,1,0,buffer,1);

   return 0;

}

int file2disk(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer)
{
int i,j;

   for (i = 0; i <= tracks; i++)
   {
      charout('r');
      if (_read(fd,buffer,bufsz) <= 0)
         break;

      charout('0');
      if ((j = trackwrite(disk,i,0,sectors,buffer)) != DISK_OK)
         break;

      if (heads)
      {
         charout('r');
         if (_read(fd,buffer,bufsz) <= 0)
            break;

         charout('1');
         if ((j = trackwrite(disk,i,1,sectors,buffer)) != DISK_OK)
            break;
      }
      if (j == -1)
         outchar('o');
   }
   return 0;
}

int disk2file(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer)
{
int i,j;

   for (i = 0; i <= tracks; i++)
   {
      charout('0');
      if ((j = trackread(disk,i,0,sectors,buffer)) != DISK_OK)
         break;

      charout('w');
      if (_write(fd,buffer,bufsz) == -1)
         break;

      if (heads)
      {
         charout('1');
         if ((j = trackread(disk,i,1,sectors,buffer)) != DISK_OK)
            break;

         charout('w');
         if (_write(fd,buffer,bufsz) == -1)
            break;
      }
      if (j == -1)
         outchar('o');
   }
   return 0;
}

int trackread(int disk, int track, int head, int nsectors, char *buffer)
{
int i,j;

   if ((i = disk_read_p(disk,track,1,head,buffer,nsectors)) != DISK_OK)
   {
      char *b = buffer;
      charout('x');
      for (j = 1; j <= nsectors; j++)
      {
         if ((i = disk_read_p(disk,track,j,head,b,1)) != DISK_OK && !Keep)
            break;
         b += (j * 512);
      }
   }
   return i;
}

int trackwrite(int disk, int track, int head, int nsectors, char *buffer)
{
int i,j;

   if ((i = disk_write_p(disk,track,1,head,buffer,nsectors)) != DISK_OK)
   {
      char *b = buffer;
      charout('x');
      for (j = 1; j <= nsectors; j++)
      {
         if ((i = disk_write_p(disk,track,j,head,b,1)) != DISK_OK && !Keep)
            break;
         b += (j * 512);
      }
   }
   return i;
}

void die(const char *msg, const char *fmt, int arg)
{
   printf("\n%s",msg);
   if (fmt)
      printf(fmt,arg);
   printf("\n");
   exit(1);
}

void ctrlc(int sig)
{
   signal(SIGINT, SIG_IGN);
   puts("\nStop.");
   longjmp(Jmpbuf, 1);
}

void outchar(int c)
{
union REGS r;

    r.x.ax = 0x0E00 + c;
    r.h.bh = 0;
    int86(0x10,&r,&r);
}

void charout(int c)
{
union REGS r;

    r.x.ax = 0x0A00 + c;
    r.h.bh = 0;
    r.x.cx = 1;
    int86(0x10,&r,&r);
}
