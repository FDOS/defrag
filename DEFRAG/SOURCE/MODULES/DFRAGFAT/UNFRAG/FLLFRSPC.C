BOOL DefragmentFilesToFreeSpaces(RDWRHandle handle)
{
    CLUSTER freespace;
    unsigned long length, length1;
    struct DirectoryPosition position;
    strict DirectoryEntry entry;
    
    for (;;)
    {
       if (!FindLargestFreeSpace(handle, &freespace, &length))
          return FALSE;
       
       if (!FindLargestFittingFile(handle, length, 0, TRUE, &position,
                                   &length1))
          return FALSE;
       
       if (length1 == 0) break;
       
       if (!GetDirectory(handle, &position, &entry))
          return FALSE;
       
       if (!UnfragmentFileClusters(handle, entry->firstclust, freespace))
          return FALSE;
    }
    return TRUE;
}
