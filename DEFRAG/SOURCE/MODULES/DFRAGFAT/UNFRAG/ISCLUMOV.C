BOOL IsClusterMovable(RDWRHandle handle, CLUSTER cluster, BOOL* isMovable)
{
   CLUSTER current = cluster, previous=cluster, cluster1;
   struct DirectoryPosition pos;
   struct DirectoryEntry entry;
   BOOL found;
   
   /* Look for invalid input */
   if ((FAT_FREE(cluster))    ||
       (FAT_BAD(cluster))     ||
       (FAT_RESERVED(cluster))||
       (FAT_LAST(cluster)))
      return FALSE;       
      
   /* Examen the contents of the cluster in the FAT */
   if (!GetNthCluster(handle, cluster, &cluster1))
      return FALSE;
      
   if (FAT_FREE(cluster1))
   {
      isMovable = TRUE;
      return TRUE;
   } 
   if (FAT_RESERVED(cluster1))
   {
      isMovable = FALSE;
      return TRUE;
   }
        
   while (previous) 
   {
     if (!FindClusterInFAT(handle, current, &previous))
        return FALSE;
        
     if (previous) current = previous;    
   } 
   
   if (!FindClusterInDirectories(handle, current, &pos, &found))
      return FALSE;
      
   if (!found)
      return FALSE;      /* Non valid cluster */
      
   if (!GetDirectory(handle, &pos, &entry))
      return FAlSE;
    
   if ((entry->attribute & FA_HIDDEN) || (entry->attribute & FA_SYSTEM))
   {
      *isMovable = FALSE;
   }
   else
   {
      *isMovable = TRUE;
   }
   
   return TRUE;
}
