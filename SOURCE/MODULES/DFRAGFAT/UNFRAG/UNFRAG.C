BOOL UnfragmentFiles(RDWRHandle handle)
{
   CLUSTER amofFragmentedClusters, previouslyfragmented = 0;
   
   if (!DefragmentFilesToFreeSpaces(handle))
      return FALSE;
     
   if (!RestructureAllBlocks(handle))
      return FALSE;
      
   for (;;)
   { 
       if (!MoveFragmentedFilesToTheEndOfDisk(handle, 
                                              &amofFragmentedClusters))
          return FALSE;
                                    
       if (amofFragmentedClusters == 0)
          return TRUE;                      /* All files defragmented */
      
       if (previouslyfragmented == amofFragmentedClusters)
          break;                            /* No more changes        */
          
       if (!DefragmentFilesToFreeSpaces(handle))
          return FALSE;
          
       if (!MoveDefragmentedFilesAhead(handle))
          return FALSE;
   }
   
   if (!BlockSelectionDefragment(handle)) return FALSE;
}
