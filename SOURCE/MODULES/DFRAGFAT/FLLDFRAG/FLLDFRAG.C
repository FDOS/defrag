BOOL FullyDefragmentVolume(RDWRHandle handle)
{
    CLUSTER startfrom=0, fileclust, storefrom=0, storeat;
    BOOL pasttheend;
          
    for (;;)
    {
      if (!SelectHighestPriorityFile(handle, startfrom, &fileclust))
         return FALSE;
      
      if (!fileclust) break;
      
      for (;;)
      {
          fileclust = GetNthFileCluster(handle, fileclust, 1, &pasttheend);
          
          if (pasttheend) break;
          
          if (!GetNextAvailableCluster(handle, storefrom, &storeat))
             return FALSE;
             
          if (!GetNthCluster(handle, cluster, &cluster1))
             return FALSE;
   if (
         
    
    CLUSTER GetNthFileCluster(RDWRHandle handle, CLUSTER firstclust, unsigned n,
                          BOOL* pasttheend)
    
      }
     
      startfrom = fileclust;
    }
    
    return TRUE;
}
