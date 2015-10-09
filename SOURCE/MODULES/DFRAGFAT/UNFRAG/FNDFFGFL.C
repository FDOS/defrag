struct Pipe
{
   CLUSTER startfrom;     
   CLUSTER result;     
   BOOL error;
}

/*
   Return the first cluster, after startfrom, that is part of a fragmented
   file AND is the head of the file
*/

BOOL FindFirstFragmentedFile(RDWRHandle handle, CLUSTER startfrom, 
                             CLUSTER* firsfragpos)
{
   struct Pipe pipe, *ppipe = &pipe;   
     
    pipe.startfrom = startfrom;
    pipe.result    = 0;
    pipe.error     = FALSE; 
     
   if (!LinearTraverseFat(handle, FirstFragmentedFileFinder, &ppipe))
      return FALSE;
                                                
   if (pipe.error) 
      return FALSE;
   
  *firstfragpos = pipe.result;
}

static BOOL FirstFragmentedFileFinder(RDWRHandle handle, CLUSTER label,
                                      SECTOR datasector, void** structure)
{
  CLUSTER cluster;
  struct DirectoryPosition dirpos;
  struct Pipe* pipe = *((struct Pipe**) structure);
  BOOL found;
  
  if (FAT_FREE(label)     ||
      FAT_BAD(label)      ||
      FAT_RESERVED(label))
     return TRUE;
  
  cluster = DataSectorToCluster(handle, datasector);
  if (!cluster)
  {
     pipe->error = TRUE;     
     return FALSE;
  }
  if (current <= pipe.startfrom)
     return TRUE;
  
  if (!FindClusterInDirectories(handle, cluster, &dirpos, &found))
  {
     pipe->error = TRUE;
     return FALSE;
  }
      
  if (found)
  {
     if (!IsFileFragmented(handle, cluster, &IsFragmented))
     {
        pipe.error = TRUE;
        return FALSE;     
     }
     
     if (IsFragmented)
     {
        pipe->result = cluster;
        return FALSE;
     }
  }
  
  return TRUE;
}
