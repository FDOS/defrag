/* DF.C - file to disk / disk to file using DISKLIB

   This file is part of the BETA version of DISKLIB
   Copyright (C) 1998, Gregg Jennings

   See README.TXT for information about re-distribution.
   See DISKLIB.TXT for information about usage.

   An "industry standard" floppy image writer/reader. Image format
   is track 0, side 0; track 0, side 1; track 1, side 0; etc.

   2.3  22-Nov-1998 greggj  finally compiled (fixed error handling bugs)
                            for Windows 95/98
   2.2  22-Nov-1998 greggj  added options -s and -n
   2.1  19-Nov-1998 greggj
   2.0  05-Nov-1998 greggj  uses DISKLIB
   1.1  27-Feb-1998 greggj  better error handling
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

#include "dosio.h"
#include "disklib.h"
#include "lib.h"
#include "fat.h"

#ifdef __WATCOMC__
#define _open  open
#define _read  read
#define _write write
#define _close close
#endif

#define NO_WRITE 0                  /* for testing */

void options(int argc, char *argv[]);
int file2disk(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer);
int disk2file(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer);
int trackread(int disk, int track, int head, int nsectors, char *buffer);
int trackwrite(int disk, int track, int head, int nsectors, char *buffer);
void die(const char *msg, const char *fmt, int arg);
void setlast(int disk, int *track, int *head, int sysonly);
void erase(int n);
void usage(void);


jmp_buf Jmpbuf;                     /* abort place holder */
int Keep;                           /* keep going on bad sectors */
int Last;                           /* stop at last allocated sector */
int Disk;                           /* disk number */
int File;                           /* file number relative to argv */
int MaxTrack,MaxHead;
int Reset;
int Zero;
int Verify;
int Disk,File,ToDisk;
long int Bytes;
int SysOnly;
int NoAsk;

void usage(void)
{
    printf("\n");
    printf("usage: diskfile [-{options}] <file> <drive>\n");
    printf("       diskfile [-{options}] <drive> <file>\n");
    printf("\n");
    printf("write file to floppy, or write floppy to file\n");
    printf("\n");
    printf("options:    r  reset drive before and after\n");
    printf("            v  verify disk writes\n");
    printf("            z  zero buffers before reads\n");
    printf("            k  keep writing even if errors\n");
    printf("            l  read disk up to last allocated cluster (FAT disks only)\n");
    printf("            s  write system tracks of disk to file\n");
    printf("            n  do not ask for confirmation (unless to overwrite file)\n");

    exit(1);
}

int main(int argc, char **argv)
{
int fd,i;
char *buffer;
size_t bufsz;
int tracks,sectors,heads;

#if defined __WATCOMC__ || defined __GNUC__
    setvbuf(stdout,NULL,_IONBF,0);      /* flush stdout every printf */
#endif

    Reset = 0;
    Zero = 0;
    Keep = 0;
    Last = 0;
    Verify = 0;
    MaxTrack = MaxHead = -1;
    Bytes = 0;
    Disk = File = ToDisk = -1;
    SysOnly = 0;
    NoAsk = 0;

    options(argc,argv);

    if ((i = disk_get_physical(Disk,&tracks,&sectors,&heads)) != DISK_OK)
        die("BIOS error: ",disk_error(i),0);

    if ((buffer = malloc(bufsz = sectors * 512)) == NULL)
        die("Not enough memory found!"," (%u)",bufsz);

    printf("\n");

    if (Reset)
    {
        printf("Reseting controller...");
        disk_reset(Disk);
        printf(" Homing disk...");
        if ((i = disk_read_p(Disk,0,1,0,buffer,1)) != DISK_OK)
            die("BIOS error: ",disk_error(i),0);
        printf(" OK.\n");
    }

    printf("Drive %c: has %u tracks and %u sides.\n",Disk+'A',tracks+1,heads+1);

    if (Last)
        setlast(Disk,&MaxTrack,&MaxHead,0);

    if (SysOnly)
        setlast(Disk,&MaxTrack,&MaxHead,1);

    if (ToDisk == 1)
        printf("\nWrite file \"%s\" to disk in %c:",argv[File],Disk+'A');
    else
        printf("\nWrite disk in %c: to file \"%s\"",Disk+'A',argv[File]);

    if (!NoAsk && ask(NULL) == 0)
        return 1;

    if (ToDisk == 2)                                     /* writing file */
    {
        if ((fd = _open(argv[File],_O_RDONLY)) != -1)
        {
            _close(fd);
            printf("File \"%s\" exists",argv[File]);
            if (ask("; overwrite?") == 0)
                return 1;
        }
        fd = _open(argv[File],O_CREAT|O_RDWR|O_BINARY|O_TRUNC,S_IREAD|S_IWRITE);
    }
    else
    {
        fd = _open(argv[File],O_RDWR|O_BINARY);
    }

    if (fd == -1)
        die("file error: ",strerror(errno),errno);

    errno = 0;

    putchar('\n');

    /* on error we get back here with setjmp() returning non-zero */

    if (setjmp(Jmpbuf) == 0)
    {
        /* now that everything is verified and prepared, this is the meat */

        if (ToDisk == 1)
            i = file2disk(fd,Disk,tracks,heads,sectors,bufsz,buffer);
        else
            i = disk2file(fd,Disk,tracks,heads,sectors,bufsz,buffer);

        if (i == DISK_OK)
            printf("\nDone.            \n");

        /* that's not hard is it... */
    }
    else
    {
        puts("\n\nStopped.             \n");
        getch();    /* eat key from kbhit() */
    }

    _close(fd);

#ifdef _WIN32
    if (i)
#else
    if ((i = disk_status(Disk)) != 0)
#endif
        die("BIOS error: ",disk_error(i),0);

    if (errno)
        die("File error: ",strerror(errno),errno);

    if (Reset)
    {
        printf("\nReseting controller...");
        disk_reset(Disk);
        printf(" Homing disk...\n");
        disk_read_p(Disk,0,1,0,buffer,1);
    }

    printf("\n%ld bytes written\n",Bytes);

    return 0;

}

int file2disk(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer)
{
int h,i,j,k;

   for (i = 0; i <= tracks; i++)
   {
      for (h = 0; h <= heads; h++)
      {
         if (kbhit())
            longjmp(Jmpbuf, 1);

         if (Zero)
            memset(buffer,0,bufsz);

         if (i < tracks-1)
            printf("\rreading file     ");

         if (_read(fd,buffer,bufsz) <= 0)
            return 0;                           /* RETURN */

         printf("\rtrack %d, side %d ",i,h);

         if ((j = trackwrite(disk,i,h,sectors,buffer)) != DISK_OK)
            return j;                           /* RETURN */

         Bytes += ((sectors * 512) - (j * 512));

         if (j != 0)    /* (this actually never happens) */
         {
            printf(" %d bad sectors",j);
            if (Keep)
               printf(" ignored");
            else
               return j;                        /* RETURN */
         }

         k = 0;
         if (Verify)
         {
            printf("verifying...");

            /* weird... verify is not zero based */

            k = disk_verify_p(disk,i,1,h,sectors);
            erase(12);
         }

         if (k != 0)
            printf(" verify error (%xh)",k);

         if (j || k)
            printf("\n");
      }
   }

   return 0;
}

int disk2file(int fd, int disk, int tracks, int heads, int sectors,
              size_t bufsz, char *buffer)
{
int h,i,j,k;

   for (i = 0; i <= tracks; i++)
   {
      for (h = 0; h <= heads; h++)
      {
         if (kbhit())
            longjmp(Jmpbuf, 1);

         if (Zero)
            memset(buffer,0,bufsz);

         printf("\rtrack %d, side %d ",i,h);

         if ((j = trackread(disk,i,h,sectors,buffer)) != DISK_OK)
            return j;

         if (i < tracks-1 && (MaxTrack == -1 && i != MaxTrack && h != MaxHead))
            printf("\rwriting file     ");

         if ((k = _write(fd,buffer,bufsz)) == -1)
            return 0;

         Bytes += k;

         if (j != 0)    /* (this actually never happens) */
         {
            printf("%d bad sectors\n",j);
            if (Keep)
               printf(" ignored\n");
            else
               return j;
         }

         if (MaxTrack != -1 && i == MaxTrack && h == MaxHead)
            return 0;
      }
   }

   return 0;
}

int trackread(int disk, int track, int head, int nsectors, char *buffer)
{
int i,j;
char *b;

#if NO_WRITE
   return DISK_OK;
#endif

   if ((i = disk_read_p(disk,track,1,head,buffer,nsectors)) != DISK_OK)
   {
      if (i == 0x80)        /* abort if no disk in drive */
         return i;

      b = buffer;

      for (j = 1; j <= nsectors; j++)
      {
         if (kbhit())
            longjmp(Jmpbuf, 1);

         putchar('.');
         putchar('\b');

         if ((i = disk_read_p(disk,track,j,head,b,1)) != DISK_OK)
         {
            putchar('x');
            if (!Keep)
                return i;
         }
         else
            putchar('.');
         b += 512;
      }

      for (j = 1; j <= nsectors; j++)
         putchar('\b');
      for (j = 1; j <= nsectors; j++)
         putchar(' ');
      for (j = 1; j <= nsectors; j++)
         putchar('\b');

   }

   return DISK_OK;
}

int trackwrite(int disk, int track, int head, int nsectors, char *buffer)
{
int i,j;
char *b;

#if NO_WRITE
   return DISK_OK;
#endif

   if ((i = disk_write_p(disk,track,1,head,buffer,nsectors)) != DISK_OK)
   {
      if (i == 0x80)        /* abort if no disk in drive */
         return i;

      b = buffer;
      for (j = 1; j <= nsectors; j++)
      {
         if (kbhit())
            longjmp(Jmpbuf, 1);
         if ((i = disk_write_p(disk,track,j,head,b,1)) != DISK_OK)
         {
            if (!Keep)
                return i;
         }
         b += 512;
      }
   }
   return DISK_OK;
}

void setlast(int disk, int *track, int *head, int sysonly)
{
int i;
struct DEVICEPARAMS dp;
long fatsector,lastsec;
int secsfat,datasec;
unsigned int nclusters;
unsigned short maxcluster;

    if ((i = disk_getparams(disk,&dp)) != DISK_OK)
    {
        /* LIB TODO: make a strliberror() function (like strerror()) */
        lib_error("getlast()",i);
        exit(1);
    }

    if (sysonly)
    {
        datasec = data_sector(&dp);
        physical(track,NULL,head,datasec,dp.hidden_sectors,dp.secs_track,
                 dp.num_heads);
        printf("System extends to track %d, side %d (logical sector %d).\n",
               MaxTrack,MaxHead,datasec);
        return;
    }

    nclusters = num_clusters(&dp);
    fatsector = dp.reserved_secs;
    secsfat = dp.secs_fat;
    maxcluster = searchfat12(disk,(unsigned short)nclusters,
                             fatsector,secsfat,0);
    if (maxcluster == 0)
        die("There seems to be no data on the disk in ","%c:.",disk+'A');

    datasec = data_sector(&dp);
    lastsec = cluster_to_sector(maxcluster,datasec,dp.secs_cluster);
    lastsec += (dp.secs_cluster-1);

    physical(track,NULL,head,lastsec,dp.hidden_sectors,dp.secs_track,
             dp.num_heads);

    printf("Last allocated cluster, %u, is on ",maxcluster);
    printf("track %d, side %d.\n",MaxTrack,MaxHead);
}

void options(int argc, char *argv[])
{
int i;

    if (argc < 3)
    {
        printf("\nNot enough arguments.\n");
        usage();
    }

    for (i = 1; i < argc; i++)
    {
        /* option? */

        if (argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                case 'r':   Reset = 1; break;
                case 'v':   Verify = 1; break;
                case 'z':   Zero = 1; break;
                case 'k':   Keep = 1; break;
                case 'l':   Last = 1; break;
                case 's':   SysOnly = 1; break;
                case 'n':   NoAsk = 1; break;
                default:
                    printf("\nInvalid option: '%c'.\n",
                           (argv[i][1]) ? argv[i][1] : '-');
                    usage();
                    break;
            }
        }

        /* disk? */

        else if (argv[i][1] == ':' && argv[i][2] == '\0')
        {
            if (Disk != -1)
            {
                printf("\nToo many drive specifications.\n");
                usage();
            }
            Disk = toupper(argv[i][0]) - 'A';
        }

        /* file. */

        else
        {
            /*
                Note that File is and argv index, ToDisk is based
                on argumnet order.
            */

            if (File != -1)
            {
                printf("\nToo many file specifications.\n");
                usage();
            }

            File = i;
            ToDisk = (Disk == -1) ? 1 : 2;
        }
    }

    /*
        Note that Disk, File and ToDisk have to have been intitialzed
        all to -1.
    */

    if (Disk == -1)
    {
        printf("\nDrive specification not found.\n");
        usage();
    }

    if (ToDisk == -1)
    {
        printf("\nFile specification not found.\n");
        usage();
    }

    if (Disk < 0 || Disk > 1)    /* floppies only */
    {
        printf("\nDrive specification not valid.\n");
        usage();
    }

    if (Last && File == 1)
    {
        printf("\nIncompatible option '-l'.\n");
        usage();
    }

    if (SysOnly && File == 1)
    {
        printf("\nIncompatible option '-s'.\n");
        usage();
    }

    if (Verify && File == 2)
    {
        printf("\nIncompatible option '-v'.\n");
        usage();
    }
}

void erase(int n)
{
int j;

   for (j = 1; j <= n; j++)
      putchar('\b');
   for (j = 1; j <= n; j++)
      putchar(' ');
   for (j = 1; j <= n; j++)
      putchar('\b');
}

#if NO_WRITE
int _write(int fd, void *buffer, int bufsz)
{
    return bufsz;
}
#endif
