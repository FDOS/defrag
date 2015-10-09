struct Pipe
{
   CLUSTER count;
   CLUSTER amoftofind;
   CLUSTER startfrom;
   CLUSTER result;
   BOOL    error;
};

/*
   Find the nth cluster that is part of a fragmented file, start counting
   after startfrom.
*/

BOOL FindNthFragmentedCluster(RDWRHandle handle, CLUSTER startfrom, 
                              CLUSTER count, CLUSTER* result)
{
   struct Pipe pipe, *ppipe = &pipe;
   
   pipe.count = 0;
   pipe.amoftofind = count;
   pipe.startfrom = startfrom;
   pipe.result = 0;
   pipe.error = FALSE;
   
   if (!LinearTraverseFat(handle, NthFragmentedClusterFinder, &ppipe))
      return FALSE;
      
   if (pipe.error)
      return FALSE;
   
   *result = pipe.result;   
   return TRUE;
}

static BOOL NthFragmentedClusterFinder(RDWRHandle handle, CLUSTER label,
                                       SECTOR datasector, void** structure)
{
   CLUSTER cluster;
   BOOL IsPartOf;
   struct Pipe* pipe = *((struct Pipe**) structure);
   
   cluster = DataSectorToCluster(handle, datasector);
   if (!cluster)
   {
      pipe->error = TRUE;
      return FALSE;
   }
   
   if (cluster <= pipe->startfrom) 
      return FALSE;
   
  if (!IsPartOfFragmentedFile(handle, cluster, &IsPartOf))
  {
     pipe->error = TRUE;
     return FALSE;
  }
  
  if (IsPartOf)
  {
     pipe->count++;
     if (pipe->count == pipe->amoftofind)
     {
        pipe->result = cluster;
        return FALSE;
     }
  }
  
  return TRUE;
}
