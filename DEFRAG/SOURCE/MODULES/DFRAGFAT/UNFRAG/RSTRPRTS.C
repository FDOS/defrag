BOOL RestructureAllBlocks(RDWRHandle handle)
{
    CLUSTER lowerbound, upperbound, current=0;
    
    for (;;)
    {
       if (!FindNextPartition(handle, current, &lowerbound, &upperbound))
       {
          return FALSE;
       }
       if (!lowerbound) break;
       
       if (!RestructurePartition(handle, lowerbound, upperbound))
       {
          return FALSE;
       }
       
       current = upperbound+1;
    }
    return TRUE;
}

BOOL RestructureBlock(RDWRHandle handle, CLUSTER lowerbound,
                      CLUSTER upperbound)
{
    if (!CrunchClusterBlock(handle, lowerbound, upperbound))
       return FALSE;
    
    if (!MoveDefragmentedFilesToStartOfBlock(handle, lowerbound, upperbound))
       return FALSE;
       
    return TRUE;
}
