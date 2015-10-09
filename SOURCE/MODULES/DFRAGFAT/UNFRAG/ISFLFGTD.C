struct Pipe
{
    BOOL error;
    BOOL fragmented;
};

BOOL IsFileFragmented(RDWRHandle handle, CLUSTER firstclust, 
                      BOOL* IsFragmented)
{
   struct Pipe pipe, *ppipe = &pipe;
   
   pipe.fragmented = FALSE;
   pipe.error      = FALSE;
   
   if (!FileTraverseFAT(handle, firstclust, FragmentationFinder, &ppipe))
   {
      return FALSE;
   }
   if (pipe.error)
   {
      return FALSE;
   }
   
   *IsFragmented = file->fragmented;
   return TRUE;
}

static BOOL FragmentationFinder(RDWRHandle handle, CLUSTER label,
                                SECTOR datsector, void** structure)
{
   CLUSTER cluster;
   struct Pipe* pipe = (struct Pipe*) *structure;

   if (FAT_NORMAL(label))
   {
      cluster = DataSectorToCluster(datasector);
      if (!cluster)
      {
         pipe->error = TRUE;
         return FALSE;
      }
      
      if (cluster != label-1)
      { 
         pipe->fragmented = TRUE;
         return FALSE;
      }
      return TRUE;        
  }
  if (FAT_LAST(label))
  {
     return FALSE;
  }
  else
  {
     pipe->error = TRUE;
     return FALSE;
  }
}
