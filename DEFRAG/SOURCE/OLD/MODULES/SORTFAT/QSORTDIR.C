
/*
** Generic routine to sort entries in a directory.
**
** Different implementatiotions should give the possibility 
** to both make use of all available memory and still allow
** for VERY low (< 8Kb) memory conditions.
**
** Returns FALSE if not been able to sort, TRUE otherwise.
**
** TODO: make iterative?
*/

static int QuickSortDirEntries(void* entries,
                               int (*cmpfunc)(struct DirectoryEntry* e1,
                                              struct DirectoryEntry* e2,
                                              void*  entries,
                                              int (*getslot) (void* entries, 
                                                              struct DirectoryEntry* entry,
                                                              int slot)),
                               int (*filterfunc)(int x),
                               int (*getentry)(void* entries, unsigned pos
                                               struct DirectoryEntry* result),
                               int (*swapentry)(void* entries, unsigned pos1, unsigned pos2),
                               int (*getslot) (void* entries, int slot),
                               int from, int to)
{
     int i, j, cmpres;
     struct DirectoryEntry entry, entry1;

     i = from; j = to;
     if (!getentry(entries, (i + j) / 2, &entry))
        return FALSE;
          
     do {
         if (!getentry(entries, i, &entry1)) return FALSE;
         
         
         while (1)
         {
            cmpres = cmpfunc(&entry1, &entry, entries, getslot);
            if (cmpres == -2)
                return FALSE;
               
            if (filterfunc(cmpres) != -1) break;
            i++;
            
            if (!getentry(entries, i, &entry1)) return FALSE;
         }
         
         if (!getentry(entries, j, &entry1)) return FALSE;
            
         while (1)
         {       
            cmpres = cmpfunc(&entry1, &entry, entries, getslot);
            if (cmpres == -2)
               return FALSE;
            
            if (filterfunc(cmpres) !=  1) break; 
            j--;
            
            if (!getentry(entries, j, &entry1)) return FALSE;
         }

         if (i < j)
         {
            if (!swapentry(entries, i, j))
               return FALSE;

            i++;
            j--;
         }
     } while (i <= j);

     if (j > from)
        if (!SortDirEntries(entries, cmpfunc, filterfunc, 
                                     getentry, swapentry,
                                     getslot,
                            from, j))
           return FALSE;
           
     if (j < to)
        if (!SortDirEntries(entries, cmpfunc, filterfunc, 
                                     getentry, swapentry,
                                     getslot,
                            i, to))
           return FALSE;
}
