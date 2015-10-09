/*    
   Walktree.c - call a function for every entry on a disk.
   
   Copyright (C) 2000 Imre Leber

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be
*/

#include "fte.h"

struct PipeStruct {
       int (*func) (RDWRHandle handle,
		    struct DirectoryPosition* position,
		    void** structure);
       void** structure;
       
       BOOL error;
};

static int WalkTree(RDWRHandle handle,
		    CLUSTER cluster,
		    void** structure);

static int ActionWalker(RDWRHandle handle,
			struct DirectoryPosition* pos,
			void** buffer);

static int TreeRecurser(RDWRHandle handle,
			struct DirectoryPosition* pos,
			void** buffer);

BOOL WalkDirectoryTree(RDWRHandle handle,
		      int (*func) (RDWRHandle handle,
				   struct DirectoryPosition* position,
				   void** structure),
		      void** structure)
{
    struct PipeStruct pipe, *ppipe = &pipe;

    pipe.func      = func;
    pipe.structure = structure;
    pipe.error     = FALSE;

    if (!TraverseRootDir(handle, ActionWalker, (void**)&ppipe, TRUE))
       return FALSE;
    
    if (!TraverseRootDir(handle, TreeRecurser, (void**)&ppipe, TRUE))
       return FALSE;
    
    if (pipe.error)
       return FALSE;

    return TRUE;
}

static int WalkTree(RDWRHandle handle,
		    CLUSTER cluster,
		    void** structure)
{
    struct PipeStruct* pipe = *((struct PipeStruct**) structure); 

    if (!TraverseSubdir(handle, cluster, ActionWalker, structure, TRUE))
       return FALSE;
    if (!TraverseSubdir(handle, cluster, TreeRecurser, structure, TRUE))
       return FALSE;
       
    if (pipe->error) return FALSE;
    
    return TRUE;
}

static int ActionWalker(RDWRHandle handle,
			struct DirectoryPosition* pos,
			void** buffer)
{
    struct PipeStruct* pipe = *((struct PipeStruct**) buffer);

    return pipe->func(handle, pos, pipe->structure);
}

static int TreeRecurser(RDWRHandle handle,
			struct DirectoryPosition* pos,
			void** buffer)
{
    CLUSTER firstcluster;    
    struct DirectoryEntry entry;
    struct PipeStruct* pipe = *((struct PipeStruct**) buffer);

    if (!GetDirectory(handle, pos, &entry))
    {
       pipe->error = TRUE;     
       return FALSE;
    }

    firstcluster = GetFirstCluster(&entry);
    
    if (!IsLFNEntry((&entry))        &&
	(entry.attribute & FA_DIREC) &&
	(!IsDeletedLabel(entry))     &&
	(!IsCurrentDir(entry))       &&
	(!IsPreviousDir(entry)))
       return WalkTree(handle, firstcluster, (void**) &pipe);
    else
       return TRUE;
}
