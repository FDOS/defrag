#include "fte.h"

int RawDumpBoot(struct BootSectorStruct* boot);
int InterpretedDumpBoot(RDWRHandle handle);

int main(int argc, char** argv)
{
    RDWRHandle handle;
    struct BootSectorStruct boot;
    
    if (argc != 2)
    {
       printf("Test for boot sector info\n"
	      "\tbootinfo <drive or image file>");
       return 1;
    }

    if (!InitReadSectors(argv[1], &handle))
    {
       printf("Could not access %s\n", argv[1]);
       return 1;
    }

    if (!ReadBootSector(handle, &boot))
    {
       printf("Unable to read boot sector.\n");
       return 1;
    }

    if (RawDumpBoot(&boot)) return 1;

    if (InterpretedDumpBoot(handle)) return 1;

    return 0;
}

int RawDumpBoot(struct BootSectorStruct* boot)
{
    int i;

    printf("RAW BOOT INFO\n");

    printf("Boot info:\n"
           "----------\n"
           "FAT type is: FAT%d\n",
           DetermineFATType(boot));

    printf("Information directly from disk:\n"
           "Shared info:\n"
           "\tBytes per sector:       %u\n"
           "\tSectors per cluster:    %u\n"
           "\tReserved sectors:       %u\n"
           "\tNumber of FATs:         %u\n"
           "\tNumber of Root entries: %u\n"
           "\tNumber of sectors(1):   %u\n"
           "\tdescriptor:             %u\n"
           "\tSectors per fat:        %u\n"
           "\tSectors per track:      %u\n"
           "\tHeads:                  %u\n"
           "\tHidden sectors:         %lu\n"
           "Number of sectors(2):     %lu\n\n",
           (unsigned) boot->BytesPerSector,
           (unsigned) boot->SectorsPerCluster,
           (unsigned) boot->ReservedSectors,
           (unsigned) boot->Fats,
           (unsigned) boot->NumberOfFiles,
           (unsigned) boot->NumberOfSectors,
           (unsigned) boot->descriptor,
           (unsigned) boot->SectorsPerFat,
           (unsigned) boot->SectorsPerTrack,
           (unsigned) boot->Heads,
           boot->HiddenSectors,
           boot->NumberOfSectors32);

    printf("FAT12/16 information\n"
           "Bios drive number:  %u\n"
           "Signature:          %u\n"
           "Disk serial number: %lu\n"
           "Last two bytes:     %x\n",
           (unsigned) boot->fs.spc1216.DriveNumber,
           (unsigned) boot->fs.spc1216.Signature,
           boot->fs.spc1216.VolumeID,
           boot->fs.spc1216.LastTwoBytes);
           
    printf("Volume label:");
    for (i = 0; i < 11; i++)
       printf("%c", boot->fs.spc1216.VolumeLabel[i]);
    puts("");

    printf("File system string:");
    for (i = 0; i < 8; i++)
       printf("%c", boot->fs.spc1216.FSType[i]);
    puts("");

    printf("FAT32 information\n"
           "Sectors per fat:    %lu\n"
           "Extended flags:     %u\n"
           "FAT32 version:      %u\n"
           "Root cluster:       %lu\n"
           "FSinfo at:          %u\n"
           "Backup boot at:     %u\n"
           "Bios drive number:  %u\n"
           "Signature:          %u\n"
           "Disk serial number: %lu\n"
           "Last two bytes:     %x\n",
           boot->fs.spc32.SectorsPerFat,
           boot->fs.spc32.ExtendedFlags,
           boot->fs.spc32.FSVersion,
           boot->fs.spc32.RootCluster,
           boot->fs.spc32.FSInfo,
           boot->fs.spc32.BackupBoot,
           (unsigned) boot->fs.spc32.DriveNumber,
           (unsigned) boot->fs.spc32.Signature,
           boot->fs.spc32.VolumeID,
           boot->fs.spc32.LastTwoBytes);

    if (CHECK_LAST_TWO_BYTES(boot))
       printf("Last two bytes OK");
    else
       printf("Last two bytes WRONG");
           
    printf("Volume label:");
    for (i = 0; i < 11; i++)
       printf("%c", boot->fs.spc32.VolumeLabel[i]);
    puts("");

    printf("File system string:");
    for (i = 0; i < 8; i++)
       printf("%c", boot->fs.spc32.FSType[i]);
    puts("");

    return 0;
}

int InterpretedDumpBoot(RDWRHandle handle)
{
    int i;
    char label[11];
    char FSType[8];

    unsigned      SValue;
    unsigned long LValue;

    printf("INTERPRETED BOOT INFO\n");

    SValue = GetNumberOfRootEntries(handle);
    if (SValue)
        printf("Number of ROOT entries:   %u\n", SValue);
    else
    {
        printf("ERROR (nre)\n");
        return 1;
    }
    
    SValue = GetSectorsPerCluster(handle);
    if (SValue)
        printf("Sectors per cluster:      %u\n", SValue);
    else
    {
        printf("ERROR(spc)\n");
        return 1;
    }

    SValue = GetReservedSectors(handle);
    if (SValue)
        printf("Reserved sectors:         %u\n", SValue);
    else
    {
        printf("ERROR(rs)\n");
        return 1;
    }

    SValue = GetNumberOfFats(handle);
    if (SValue)
        printf("Number of FATs:           %u\n", SValue);
    else
    {
        printf("ERROR(nof)\n");
        return 1;
    }

    SValue = GetMediaDescriptor(handle);
    if (SValue)
        printf("Mediadescriptor:          %u\n", SValue);
    else
    {
        printf("ERROR(desc)\n");
        return 1;
    }

    LValue = GetNumberOfSectors(handle);
    if (LValue)
        printf("Number of sectors:        %lu\n", LValue);
    else
    {
        printf("ERROR(nos)\n");
        return 1;
    }

    LValue = GetSectorsPerFat(handle);
    if (LValue)
        printf("Sectors per FAT:          %lu\n", LValue);
    else
    {
        printf("ERROR(spf)\n");
        return 1;
    }

    SValue = GetSectorsPerTrack(handle);
    if (SValue)
        printf("Sectors per track:        %u\n", SValue);
    else
    {
        printf("ERROR(spt)\n");
        return 1;
    }

    SValue = GetReadWriteHeads(handle);
    if (SValue)
        printf("Read/write heads:         %u\n", SValue);
    else
    {
        printf("ERROR(rwh)\n");
        return 1;
    }
    
    LValue = GetClustersInDataArea(handle);
    if (LValue)
        printf("Clusters in data area:    %lu\n", LValue);
    else
    {
        printf("ERROR(cid)\n");
        return 1;
    }

    SValue = GetBiosDriveNumber(handle);
    if (SValue != 0xff)
        printf("BIOS drive number:        %u\n", SValue);
    else
    {
        printf("ERROR(bdn)\n");
        return 1;
    }

    printf("Volume data filled:       ");
    switch (IsVolumeDataFilled(handle))
    {
       case 0xff:
            printf("ERROR!\n");
            return 1;

       case FALSE:
            printf("NO\n");
            break;

       case TRUE:
            printf("YES\n");
            break;

       default:
            printf("INVALID RETURN VALUE\n");
    }

    LValue = GetDiskSerialNumber(handle);
    if (LValue)
         printf("Disk serial number:       %lu\n", LValue);
    else
    {
        printf("ERROR(dsn)\n");
        return 1;
    }

    SValue = GetFAT32Version(handle);
    if (SValue)
         printf("FAT32 version:            %u\n", SValue);
    else
    {
        printf("ERROR(FAT32V)\n");
        return 1;
    }

    LValue = GetFAT32RootCluster(handle);
    if (LValue)
         printf("FAT32 root cluster:       %lu\n", LValue);
    else
    {
        printf("ERROR(FAT32RC)\n");
        return 1;
    }
    
    SValue = GetFSInfoSector(handle);
    if (SValue)
         printf("FSInfo sector:            %u\n", SValue);
    else
    {
        printf("ERROR(FSInfo)\n");
        return 1;
    }

    SValue = GetFAT32BackupBootSector(handle);
    if (SValue)
         printf("FAT32 backup boot sector: %u\n", SValue);
    else
    {
        printf("ERROR(FAT32BB)\n");
        return 1;
    }

    if (!GetBPBVolumeLabel(handle, label))
    {
       printf("Unable to read volume label\n");
       return 1;
    }

    printf("BPB volume label: ");
    for (i = 0; i < 11; i++)
    {
        printf("%c", label[i]);
    }
    puts("");
    
    if (!GetBPBFileSystemString(handle, FSType))
    {
       printf("Unable to read file system string\n");
       return 1;
    }

    printf("File system string: ");
    for (i = 0; i < 8; i++)
    {
        printf("%c", FSType[i]);
    }
    puts("");
    return 0;
}
