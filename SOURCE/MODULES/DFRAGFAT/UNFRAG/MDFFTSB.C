BOOL MoveDefragmentedFilesToStartOfBlock(RDWRHandle handle, 
                                         CLUSTER lowerbound, 
                                         CLUSTER upperbound)
{
    CLUSTER i, j, cluster;
    BOOL IsPartOfFF;
    
    for (i = lowerbound; i < upperbound; i++)
    {
        if (!IsPartOfFragmentedFile(handle, i, &IsPartOfFF))
           return FALSE;
        
        if (!IsPartOfFF)
           continue;
           
        /* See if we haven't already reached to free space in this
           block */
        if (!GetNthCluster(handle, i, cluster))
           return FALSE;   
    
        if (FAT_FREE(cluster))
           break;           
           
        for (j = i; j <= upperbound; j++)
        {
            /* See if we haven't already reached to free space in this
               block */
            if (!GetNthCluster(handle, j, cluster))
               return FALSE;   
    
            if (FAT_FREE(cluster))
               break;
                        
            if (!IsPartOfFragmentedFile(handle, j, &IsPartOfFF))
               return FALSE;
           
            if (!IsPartOfFF)
               break;
        }
        
        if (j > upperbound)
           break;
        
        if (FAT_FREE(cluster))
           continue;
           
        if (!SwapClusters(handle, i, j));
           return FALSE;
    }
 
    return TRUE;
}
