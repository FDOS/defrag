struct Pipe
{
   BOOL startreached;
   CLUSTER startfrom;
   CLUSTER lowerbound;
   CLUSTER upperbound;     
   BOOL error;
};

/*
   Start is included in the partition if it is movable.
   Return value of upperbound is included in the partition.     
   If lowerbound == 0, start is not a cluster on the disk.
*/
BOOL FindNextPartition(RDWRHandle handle, CLUSTER start, 
                       CLUSTER* lowerbound, CLUSTER* upperbound)
{
   struct Pipe pipe, *ppipe = &pipe;
   
   pipe.startreached = FALSE;
   pipe.error        = FALSE;
   pipe.lowerbound   = 0;
   pipe.upperbound   = 0;
   
   if (!LinearTraverseFAT(handle, NextPartitionFinder, (void**) &ppipe))
      return FALSE;
  
   if (pipe.error)
      return FALSE;
      
   *lowerbound = pipe.lowerbound;
   
   if (pipe.upperbound)
      *upperbound = pipe.upperbound;
   else
      *upperbound = *lowerbound;
   
   return FALSE;
}

BOOL NextPartitionFinder(RDWRHandle handle, CLUSTER label, SECTOR datasector,
                         void** structure)
{
    CLUSTER current;
    struct Pipe* pipe = (struct Pipe*) *structure;
    BOOL ismovable;
    
    current = DataSectorToCluster(handle, datasector);
    if (!current)
    {
       pipe.error = TRUE;
       return FALSE;
    }
  
    if (!pipe.startreached)
    {
       if (label != current)
       {
          return TRUE;
       }
    }
    pipe.startreached = TRUE;
        
    if (!IsClusterMovable(handle, current, &ismovable))
    {
       pipe.error = TRUE;
       return FALSE;
    }
    
    if (pipe.lowerbound == 0)
    {
       if (ismovable)
       {
          pipe.lowerbound = current;
       }         
       return TRUE;
    }
    
    if (!ismovable)
    {
       return FALSE;
    }
    
    pipe.upperbound = current;
    return TRUE;
}
