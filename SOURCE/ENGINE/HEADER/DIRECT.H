#ifndef DIRECT_H_
#define DIRECT_H_

#include <dos.h>
#include <time.h>

#include "..\..\engine\header\rdwrsect.h"
#include "..\..\engine\header\fat.h"

struct PackedDate
{
       unsigned day:   4;
       unsigned month: 4;
       unsigned year:  8;
};

#if sizeof(struct PackedDate) != 2
#error Wrong Packed Date Size
#endif

struct PackedTime
{
       unsigned second: 4;
       unsigned minute: 6;
       unsigned year:   6;
};

#if sizeof(struct PackedTime) != 2
#error Wrong Packed Time Size
#endif

struct DirectoryEntry
{
    char              filename[8];           /* file name.                  */
    char              extension[3];          /* extension.                  */
    unsigned char     attribute;             /* file attribute.             */
    char              NTReserved;            /* reserved for Windows NT     */    
    char              MilisecondStamp;       /* Milisecond stamp at file
                                                creation.                   */
    struct PackedTime timestamp;             /* time last modified.         */
    struct PackedDate datestamp;             /* date last modified.         */
    unsigned short    LastAccessDate;        /* Date last accessed.         */
    unsigned short    firstclustHi;          /* hi part of first cluster of
                                                file only FAT32.           */
    unsigned short    LastWriteTime;         /* time of last write
                                                (or creation).              */
    unsigned short    LastWriteDate;         /* date of last write
                                                (or creation).                         */
    unsigned short    firstclustLo;          /* first cluster of file.      */
    unsigned long     filesize;              /* file size.                  */
};

#if sizeof(struct DirectoryEntry) != 32
#error Wrong directory entry structure
#endif

struct DirectoryPosition
{
        SECTOR sector;
        int    offset;  /* May be a value between 0 and 4096 / 32 */
};

struct LongFileNameEntry
{
       unsigned char NameIndex;   /* LFN record sequence and flags byte */
       unsigned      Part1[5];  
       unsigned char Attributes;  /* Attributes (0Fh)                   */
       unsigned char reserved;
       unsigned char checksum;
       unsigned      Part2[6];
       CLUSTER       firstclust;  /* First cluster number 
                                    (always 0000h for LFN records)      */
       unsigned      Part3[2];
};

#define LASTLABEL    0x00
#define CHARE5hLABEL 0x05
#define DOTLABEL     0x2E
#define DELETEDLABEL 0xE5

#define IsLastLabel(entry) ((entry).filename[0] == LASTLABEL)
#define IsDeletedLabel(entry) ((unsigned char) (entry).filename[0] == DELETEDLABEL)

#define IsCurrentDir(entry) (((entry).filename[0] == '.') && \
                            ((entry).filename[1] == ' '))

#define IsPreviousDir(entry) (((entry).filename[0] == '.') && \
                              ((entry).filename[1] == '.') && \
                              ((entry).filename[2] == ' '))

/*
   macro implementing int IsLFNEntry (DirectoryEntry* entry);
*/

#define LFN_ATTRIBUTES (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL)

#define IsLFNEntry(x) ((x)->attribute == LFN_ATTRIBUTES)


SECTOR GetDirectoryStart(RDWRHandle handle);
BOOL ReadDirEntry(RDWRHandle handle, unsigned short index,
                  struct DirectoryEntry* entry);
BOOL WriteDirEntry(RDWRHandle handle, unsigned short index,
                   struct DirectoryEntry* entry);
BOOL GetRootDirPosition(RDWRHandle handle, unsigned short index,
                       struct DirectoryPosition* pos);
BOOL IsRootDirPosition(RDWRHandle handle, struct DirectoryPosition* pos);
CLUSTER GetFirstCluster(struct DirectoryEntry* entry);
void SetFirstCluster(CLUSTER cluster, struct DirectoryEntry* entry);

/*
void UnPackTimeDateStamp(struct tm* time, short timestamp, short datestamp);
void PackTimeDateStamp(struct tm* time, short* timestamp, short* datestamp);
*/

unsigned long EntryLength(struct DirectoryEntry* entry);

#define DIRLEN2BYTES(x) (x << 5)

#endif

