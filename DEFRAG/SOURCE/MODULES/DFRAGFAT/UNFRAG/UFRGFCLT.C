BOOL UnfragmentFileClusters(RDWRHandle handle, CLUSTER file,
                            CLUSTER freespace)
{
  BOOL done;
  CLUSTER cluster;
  
  /* Move the file to consecutive clusters in the given available space */
  
  do {
      cluster = GetNthFileCluster(handle, file, 1, &done);
      if (!cluster) return FALSE;
            
      if (!RelocateCluster(handle, cluster, freespace))
         return FALSE;
         
      file = freespace;
      freespace++;
  } while (!done);

  return TRUE;
}
