
static int entrygetter (RDWRHandle handle,
			struct DirectoryPosition* pos,
			void** buffer);

#define ENTRY 1
#define SLOT  2                        
                        
struct Pipe 
{
    /* Input */
    int slotorentry; /* Indicates wether to return an entry or a slot
                        in an entry.                                  */
    unsigned entry;
    unsigned slot;
    
    /* Processing */
    unsigned ecounter;
    unsigned scounter;
    
    /* Output */
    int error;
    struct DirectoryEntry* entry;
};

static int getentry(RDWRHandle handle,
                    CLUSTER cluster, 
                    int slotorentry,
                    unsigned entry,
                    unsigned slot, 
                    struct DirectoryEntry* result)
{
   
    struct Pipe pipe, ppipe = &pipe;
        
    pipe.slotorentry = slotorentry;
    pipe.entry = pos;
    pipe.slot  = slot;
    
    pipe.ecounter = 0;
    pipe.scounter = 0;
    
    pipe.error = FALSE;
    pipe.entry = result;
         
    if (TraverseSubdir(handle, cluster, entrygetter, &ppipe))
    {
       if (pipe.error)
          return FALSE;
       else
          return TRUE;
    }
    return FALSE;
}

int GetEntryFromDisk(void* entries, unsigned entry,  
                     struct DirectoryEntry* result)
{ 
    struct DiskEntryGetterStruct* parameters;

    parameters = (struct DiskEntryGetterStruct*) entries;
    
    return getentry(parameters->handle, parameters->cluster,
                    ENTRY, entry, 0, result);
}

int GetSlotFromDisk(void* entries, int entry, int slot,  
                    struct DirectoryEntry* result)
{ 
    struct DiskEntryGetterStruct* parameters;

    parameters = (struct DiskEntryGetterStruct*) entries;
    
    return getentry(parameters->handle, parameters->cluster,
                    SLOT, entry, slot, result);
}

static int entrygetter (RDWRHandle handle,
			struct DirectoryPosition* pos,
			void** buffer)
{
     struct Pipe** pipe;
     struct DirectoryEntry entry;
     
     if (!GetDirectory(handle, pos, &entry))
     {
        (*pipe)->error = TRUE;
        return FALSE;
     }
     
     if ((*pipe)->entry == (*pipe)->ecounter)
     {
        if ((*pipe)->slotorentry == ENTRY)
        {
           memcpy((*pipe)->entry, &entry, sizeof(struct DirectoryEntry));
           return FALSE;
        }
        else
        {
           if ((*pipe)->slot == (*pipe)->scounter)
           {
              memcpy((*pipe)->entry, &entry, sizeof(struct DirectoryEntry));
              return FALSE;
           }
           (*pipe)->scounter++;
           return TRUE;
        }  
     }
     
     if ((entry->attribute & FA_LABEL) == 0)
        (*pipe)->ecounter++;     
     
     return TRUE;   
}

