BOOL BlockSelectionDefragment(RDWRHandle handle)
{
   CLUSTER freespace, startfrom = 0, current;
   unsigned long flength, i, freelength=0;
   BOOL hasfreespace, pasttheend;
   struct DirectoryPosition firstfragpos;
   struct DirectoryEntry firstfragentry;
   unsigned char sectorspercluster;
   unsigned long bytespercluster;
   
   if (!FindFirstFreeSpace(handle, &freespace, &flength))
      return FALSE;
   
   hasfreespace = freespace != 0;
   
   for (;;)
   {
      /*
         Get the first fragmented cluster, search the file that it belongs to
         and write the entire file to the start of available space. If there
         is still free space then this is the first free cluster. Otherwise 
         we will have to swap with the first cluster which belongs to a
         fragmented file. 
      */
      if (!FindFirstFragmentedFile(handle, startfrom, &firsfragpos))
         return FALSE;
   
      if (!firstfragpos) return TRUE;
      
      if (!GetDirectory(handle, &firstfragpos, &firstfragentry))
         return FALSE;
      
      sectorsperclusters = GetSectorsPerCluster(handle); 
      bytespercluster    = sectorspercluster * BYTESPERSECTOR;
      
      flength = (firstfragentry.filesize / bytespercluster) +
                (firstfragentry.filesize % bytespercluster > 0);
   
      current = firstfragentry.firstclust;
   
      for (i = 0; i < flength; i++)
      { 
          if (hasfreespace)
          {
             if (freelength == 0)
             {
                if (!FindFirstFreeSpace(handle, &freespace, &freelength))
                   return FALSE;
             }
             else
             {
                 freespace++; 
             }
          }
          else
          {
             if (!FindNthFragmentedCluster(handle, startfrom, i, &freespace))
                return FALSE;
          }
      
          current = GetNthFileCluster(handle, current, 
                                      1, &pasttheend);
          if (pasttheend)
             return FALSE;                /* Invalid file size in entry */
       
       
          if (hasfreespace)
          {
             if (!RelocateCluster(handle, current, freespace))
                return FALSE;
          }
          else
          {
             if (current != freespace)
                if (!SwapClusters(handle, current, freespace))
                   return FALSE;
          }  
       }
       startfrom = firstfragentry.firstclust;
   }
}
