
/* Notice that LFN entries are stored in reverse order! */

int IsLFNLastEntry(struct LongFileNameEntry* entry)
{
    return (entry->NameIndex & 0x40);
}

int IsLFNDeletedEntry(struct LongFileNameEntry* entry)
{
    return (entry->NameIndex & 0x80);     
}

/*
  This function retreives the addresses of the first and last 
  entries that are fully contained in the given buffer.
  
  If there are no such entries it returns 0 in both return 
  values.
  
  Buflen is counted in multiples of sizeof(struct DirectoryEntry)
*/
void GetContainedEntries(char* buffer, unsigned buflen,
                         struct DirectoryEntry** pEntry1,
                         struct DirectoryEntry** pEntry2)
{
    int i = 0;
    struct DirectoryEntry *pentry1, *pentry2;
    
    *pEntry1 = NULL;
    *pEntry2 = NULL;
    
    pentry1 = (struct DirectoryEntry*)buffer;
    for (i=0; (i < buflen) && 
              IsLFNEntry(pentry) && 
              !IsLFNLastEntry((struct LongFileNameEntry*)pentry1);
         i++, pentry1++); 
    
    if (i == buflen) return;
    
    pentry2 = ((struct DirectoryEntry*)buffer)+(buflen-1);
    for (i=buflen; (i > 0) && IsLFNEntry(pentry2); i--, pentry2--); 
    
    if (i == 0) return;
       
    *pEntry1 = pentry1;
    *pEntry2 = pentry2;
}

/*
   Counts the entries, possibly LFNs,  contained in a buffer.
*/
unsigned CountContainedEntries(char* buffer, unsigned buflen)
{
    int counter=1;
    struct DirectoryEntry *begin, *end;
    
    GetContainedEntries(buffer, buflen, &begin, &end);
    
    if ((begin == end) && (end == 0))
       return 0;
       
    while (begin != end)
    {
        if (!IsLFNEntry(begin)) counter++;
        begin++;
    }
 
    return counter;
}

#if 0
unsigned char CalculateSFNCRC(char* shortname)        
{
    /* Still problematic when the length of the file name != 11. */ 
    /* We wont need this in defrag though.                       */
       
    unsigned char           crc = 0; 
    unsigned char             i = 0;

    crc = 0;  
    for (i=0; i<11; i++)
        crc = ((crc<<7) | (crc>>1)) + shortname[i];

    return crc;    
}
#endif
