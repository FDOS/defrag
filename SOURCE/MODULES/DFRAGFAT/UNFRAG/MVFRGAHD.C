BOOL MoveDefragmentedFilesAhead(RDWRHandle handle)
{
     CLUSTER space, i;
     unsigned long length, foundlength;
     struct DirectoryPosition foundpos;
     
     if (!FindFirstFreeSpace(handle, &space, &length))
        return FALSE;
     
     if (!space) return TRUE;
        
     do {
         if (!FindLargestFittingFile(handle, length,
                                     space+length,
                                     FALSE,
                                     &foundpos,
                                     &foundlength))
         {
            return FALSE;
         }
         
         if (foundpos)
         {
            for (i = 0; i < foundlength; i++)
            {
                if (!RelocateCluster(handle, space+i, foundspace+i))
                   return FALSE;
            }
         }
         
         if (!FindNextFreeSpace(handle, &space, &length))
            return FALSE;
                           
     } while (space);
     
     return TRUE;
}
