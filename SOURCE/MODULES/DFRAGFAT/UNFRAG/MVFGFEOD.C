struct LastFragmentedPipe
{
   CLUSTER upuntil;
   CLUSTER result;
   
   BOOL error;
};

BOOL MoveFragmentedFilesToTheEndOfDisk(RDWRHandle handle, 
                                       CLUSTER* amofFragmentedClusters)
{
     cluster lastFragmented, upuntil = 0, freeclust;
     amofFragmentedClusters = 0;
         
     for (;;)
     {
         if (!FindLastFragmentedCluster(handle, upuntil, &lastFragmented))
            return FALSE;
         
         if (!LastFragmented)
            return TRUE;
         
         upuntil = LastFragmented;
         amofFragmentedClusters++;  
         
         if (!FindLastFreeCluster(handle, &freeclust))
            return FALSE;
         
         if (!freeclust) return TRUE;
         
         if (LastFragmented < LastFree)
         {
           if (!RelocateCluster(handle, LastFragmented, LastFree))
              return FALSE;
         }
     }
}

/*
   If upuntil == 0, the entire FAT is searched
*/
static BOOL FindLastFragmentedCluster(RDWRHandle handle, CLUSTER upuntil,
                                      CLUSTER* last)
{
   struct LastFragmentedPipe pipe, *ppipe = &pipe;
   
   pipe.upuntil = upuntil;
   pipe.result  = 0;
   pipe.error   = FALSE;
   
   if (!LinearTraverseFat(handle, LastFragmentedClusterFinder, (void**) &ppipe))
      return FALSE;
   
   if (pipe.error)
      return FALSE;
      
   *last = pipe.result;    
   return TRUE;
}

static BOOL LastFragmentedClusterFinder(RDWRHandle handle,
                                        CLUSTER label,
                                        SECTOR datasector,
                                        void** structure)
{
   BOOL isPartOfFF, isMovable;
   CLUSTER cluster;
   struct LastFragmentedPipe* pipe = *((struct LastFragmentedPipe**)structure);
   
   cluster = DataSectorToCluster(handle, datasector);
   if (!cluster)
   {
      pipe->error = TRUE;
      return FALSE;
   }
   
   if (cluster == pipe->upuntil)
      return FALSE;
   
   if (!IsPartOfFragmentedFile(handle, cluster, &isPartOfFF))
      return FALSE;
      
   if (isPartOfFF)
   {
      if (!IsClusterMovable(handle, cluster, &isMovable))
         return FALSE;
      
      if (isMovable)
      {
         pipe.result = cluster;
         return TRUE;
      }
   }
   return TRUE;
}

                                      
