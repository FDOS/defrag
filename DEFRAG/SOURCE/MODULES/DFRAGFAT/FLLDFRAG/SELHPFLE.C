struct Pipe
{
    CLUSTER startfrom;
    CLUSTER result;
    
    unsigned long datetime;
        
    BOOL seendirectory;
    BOOL error;
};

/*
   If 0 is returned in result, then there are no more files right of
   startfrom (we are all done defragmenting).
   
   Please note that this function returns:
   - directories first with the newest first
   - files second with the newest first
   
   
*/
BOOL SelectHighestPriorityFile(RDWRHandle handle,CLUSTER startfrom,
                               CLUSTER* result)
{
    struct Pipe pipe, *pipe = &pipe;
    
    pipe.startfrom     = 0;
    pipe.result        = 0;
    pipe.datetime      = 0;
    pipe.seendirectory = FALSE;
    pipe.error         = FALSE;
    
    if (!WalkDirectoryTree(handle, FileFinder, (void**) &ppipe))
       return FALSE;
    
    if (pipe.error)
       return FALSE;
       
    *result = pipe.result;   
    return TRUE;
}

static BOOL FileFinder(RDWRHandle handle, struct DirectoryPosition* position,
                       void** structure)
{
    struct Pipe* pipe = *((struct Pipe**) structure);
    struct DirectoryEntry entry;
    struct tm t;
    time_t datetime;
    unsigned long currentdatetime;
    CLUSTER result;
    
    if (!GetDirectory(handle, position, &entry))
    {
       pipe.error = TRUE;
       return FALSE;
    }
    
    if ((IsLFNEntry(&entry))  ||
        (IsPreviousDir(entry))||
        (IsCurrentDir(entry)))
    {
        return TRUE;
    }
       
    if (entry.firstclust <= pipe->startfrom)
       return TRUE;
       
    currentdatetime = ((unsigned long) entry.datestamp << 16) +
                      entry.timestamp; // Correct ?  
    
    if (entry.attribute & FA_DIREC)
    {
       if (pipe->seendirectory &&
           (pipe->datetime <= currentdatetime))
       {
          return TRUE;
       }
       pipe.seendirectory = TRUE;
    }
    else
    {
       if (pipe.seendirectory)
          return TRUE;
          
       if ((pipe->datetime) &&
           (pipe->datetime <= currentdatetime))
       {
          return TRUE;
       }
    }
    
    if (!IsFileMovable(handle, &entry, &ismovable))
       return FALSE; /* Not yet written, is movable if not hidden or 
                        system and there are no bad clusters in the
                        file */
    
    if (ismovable)
    {
       pipe.result = entry.firstclust;
       return FALSE;
    }
    else
    {
       return TRUE;
    }
}
