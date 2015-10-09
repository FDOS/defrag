struct Pipe
{
    unsigned long maxlength;
    CLUSTER smallestallowed;
    BOOL mustbefragmented;
    struct DirectoryPosition* foundplace;
    unsigned long foundlength;
    
    BOOL error;
};

BOOL FindLargestFittingFile(RDWRHandle handle, unsigned long length,
                            CLUSTER smallestallowed,
                            BOOL mustbefragmented,
                            struct DirectoryPosition* foundplace,
                            unsigned long* foundlength)
{
    struct Pipe pipe, *ppipe = &pipe;
    struct DirectoryPosition foundpos;
    unsigned long foundlength=0;
    
    pipe.maxlength        = length;
    pipe.smallestallowed  = smallestallowed;
    pipe.mustbefragmented = mustbefragmented;
    pipe.foundplace       = &pos;
    pipe.foundlength      = &foundlength;
    pipe.error            = FALSE;
    
    if (!WalkDirectoryTree(handle, FileFinder, &ppipe))
       return FALSE;

    if (pipe.error) 
       return FALSE;
       
    memcpy(foundplace, &foundpos, sizeof(struct DirectoryPosition));
    *foundlength = foundlength;
    return TRUE;
}

static BOOL FileFinder(RDWRHandle handle, struct DirectoryPosition* pos,
                       void** structure)
{
    struct Pipe* pipe = (struct Pipe*) *structure;
    struct DirectoryEntry entry;
    unsigned long filelength, bytespersector, sectorspercluster,
                  bytespercluster;
    BOOL isfragmented;
        
    if (!GetDirectory(handle, pos, &entry))
    {
       pipe->error = TRUE;
       return FALSE;
    }
       
    if (IsLFNEntry(&entry))
       return TRUE;
    if (entry.attribute & FA_HIDDEN) /* Don't move unmovable clusters */
       return TRUE;
    if (entry.attribute & FA_SYSTEM)
       return TRUE;
       
    if (entry.firstclust < pipe->smallestallowed)
       return TRUE;
    
    if (!IsFileFragmented(handle, entry.firstclust, &isfragmented))
    {
       pipe->error = TRUE;
       return FALSE;
    }
    
    if (isfragmented != pipe->mustbefragmented)
       return TRUE;
   
     
    /* See how many clusters this file occupies (round off to the next 
       cluster) */   
    bytespersector = GetBytesPerSector(handle);
    if (!bytespersector) return FALSE;
    sectorspercluster = GetSectorsPerCluster(handle);
    if (!sectorspercluster) return FALSE;
    bytespercluster = bytespersector * sectorspercluster;   
       
    filelength = entry.filesize / bytespercluster +
                 (entry.filesize % bytespercluster > 0);
    
    if (filelength == pipe->maxlength)
    {
       pipe->foundlength = filelength;
       memcpy(pipe->foundplace, pos, sizeof(struct DirectoryPosition));
       return FALSE;
    }
    
    if ((filelength > pipe->foundlength) &&
        (filelength < pipe->maxlength))
    {
       pipe->foundlength = filelength;
       memcpy(pipe->foundplace, pos, sizeof(struct DirectoryPosition));
       return TRUE;
    }
    return TRUE;
}
