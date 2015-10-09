struct Pipe
{
   BOOL PreviousFilled;
   unsigned long FoundMax;
   unsigned long count;
   CLUSTER FoundCluster;
   CLUSTER LastFreeBlock;
   
   BOOL error;
};

static BOOL LargestFreeSpaceFinder(RDWRHandle handle, 
                                   CLUSTER label,
                                   SECTOR datasector,
                                   void** structure);
                                   
BOOL FindLargestFreeSpace(RDWRHandle handle, CLUSTER* place, 
                          unsigned long* length)
{
   struct Pipe pipe, *ppipe = &pipe;
   
   pipe.PreviousFilled = FALSE;
   pipe.FoundMax       = 0;
   pipe.count          = 0;
   pipe.FoundCluster   = 0;
   pipe.error          = FALSE;
   
   if (!LinearTranverseFat(handle, LargestFreeSpaceFinder, &ppipe))
      return FALSE;
   if (pipe.error)
      return FALSE;
       
   if (!pipe.PreviousFilled)
   {
      if (pipe.count > pipe.MaxFound)
      {
         *length = pipe.count;
         *place  = pipe.LastFreeBlock;
         return TRUE;
      }
   }
   
   *length = pipe.MaxFound;
   *place  = pipe.FoundCluster;
   return TRUE;
}

static BOOL LargestFreeSpaceFinder(RDWRHandle handle, 
                                   CLUSTER label,
                                   SECTOR datasector,
                                   void** structure)
{
    struct Pipe* pipe = (struct Pipe*) *structure;
    
    if (FAT_FREE(label))
    {
       if (!PreviousFilled) 
       {
          pipe->count++;
       }    
       else
       {
          LastFreeBlock = DataSectorToCluster(handle, datasector);
          if (!LastFreeBlock)
          {        
             pipe.error = TRUE;
             return FALSE;
          }
       }
       PreviousFilled = FALSE;
    }
    else
    {
       if (!PreviousFilled)
       {
          if (pipe->count > pipe->FoundMax)
          {
             pipe->FoundMax     = pipe->count;
             pipe->count        = 0;
             pipe->FoundCluster = DataSectorToCluster(handle, datasector);
             
             if (!pipe->FoundCluster)
             {
                pipe->error = TRUE;
                return FALSE;
             }
          }
          PreviousFilled = TRUE;
       } 
    }
    
    return TRUE;
}
