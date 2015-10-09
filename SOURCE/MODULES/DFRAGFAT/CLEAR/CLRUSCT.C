static int UnusedSectorClearer(RDWRHandle handle, CLUSTER label, 
                        SECTOR datasector, void** structure);

BOOL ClearUnusedSectors(RDWRHandle handle)
{
    return LinearTraverseFAT(handle, UnusedSectorClearer, NULL);
}

static int UnusedSectorClearer(RDWRHandle handle, CLUSTER label, 
                        SECTOR datasector, void** structure);
{
  SECTOR sector;
  char sectbuf[BYTESPERSECTOR];
  
  memset(sectbuf, 0xfd, BYTESPERSECTOR); 
    
  if (FAT_FREE(label))  /* Notice that this leaves unused sectors alone */
  {
     for (sector = 0; sector < GetSectorsPerCluster(handle); sector++)
         WriteDataSectors(handle, 1, datasector+sector, sectbuf);
  } 
  
  return TRUE;
}
