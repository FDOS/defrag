BOOL CrunchCLusterBlock(RDWRHandle handle, 
                        CLuSTER start, CLUSTER end)
{
   CLUSTER i;     
   CLUSTER freestart, freeend;
   CLUSTER filledstart, filledlength;
   
   for (;;)
   {
       if (!FindNextFreeClusters(handle, start, end, &freestart, &freeend))
          return FALSE;
       
       if (!freestart) break;                   /* No free space         */
       if (freeend == end) break;               /* Only space at the end */

       if (!FindNextFreeClusters(handle, freeend+1, end, &filledstart, &filledlength))
          return FALSE;
       
       if (!filledstart)                       /* Last filled part in block */
       {
          filledlength = end - freeend;
       }
       else
       {
          filledlength -= filledstart;
       }
       
       for (i = 0; i < filledlength; i++)
       {
           if (!RelocateCluster(handle, filledstart+i, freestart+i))
              return FALSE;
       }
   } 
   
   return TRUE;
}

static BOOL FindNextFreeClusters(RDWRHandle handle,
				 CLUSTER start, CLUSTER end,
                                 CLUSTER* freestart, CLUSTER* freeend)
{
   CLUSTER i, cluster, fstart = 0, fend = 0;
   
   for (i = start; i <= end; i++)
   {
       if (!GetNthCluster(handle, i, cluster))
          return FALSE;
       
       if (FAT_FREE(cluster))
       {
          if (!fstart) 
          {
             fstart = cluster;
          }
          fend = cluster;
      }
      else 
      {
           if (fstart) break;
      }   
   }

   *freestart = fstart;
   *freeend   = fend;
}

