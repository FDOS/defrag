
/*
   Notice that there is a function that returns the FAT determination string
   in disklib, the value returned there is only informative and has no
   real value. The type of FAT is ONLY determined by the cluster number.
*/

int DetermineFATType(struct BootSectorStruct* boot)
{
    unsigned long RootDirSectors, FatSize, TotalSectors, DataSector;
    unsigned long CountOfClusters;
    
    RootDirSectors = ((boot->NumberOfFiles * 32) +
                      (boot->BytesPerSector-1)) /
                      (boot->BytesPerSector);
                      
    if (boot->SectorsPerFat != 0)
        FatSize = boot->SectorsPerFat;
    else
        FatSize = boot->FAT32.SectorsPerFat;
        
    if (boot->NumberOfSectors != 0)
       TotalSectors = boot->NumberOfSectors;
    else
       TotalSectors = boot->NumberOfSectors32;
       
    DataSector = TotalSectors - 
                     (boot->ReservedSectors + 
                          (boot->Fats * FatSize) +
                             RootDirSectors);
                             
   CountOfClusters = DataSector / boot->SectorsPerCluster;
   
   if (CountOfClusters < 4085)
   {
      return FAT12;
   }
   else if (CountOfClusters < 65525)
   {
      return FAT16;
   }
   else
   {
      return FAT32;
   }    
}
