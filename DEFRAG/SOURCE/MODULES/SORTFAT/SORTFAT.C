#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "fte.h"
#include "sortfatf.h"
#include "misc.h"
#include "sortcfgf.h"
#include "expected.h"

struct CriteriumFunction
{
    int (*func)(struct DirectoryEntry* e1, struct DirectoryEntry* e2);
};

struct OrderFunction
{
    int (*func)(int x);
};

static struct CriteriumFunction CriteriumFunctions[] =
{
    CompareNames,
    CompareExtension,
    CompareDateTime,    
    CompareSize
};

static struct OrderFunction OrderFunctions[] =
{
    AscendingFilter,
    DescendingFilter
};

/*
** For constants to use as parameters look in ..\..\modlgate\defrpars.h
*/

int SortFAT(char* drive, int criterium, int order)
{
    int retVal;
    RDWRHandle handle;

    /* Mention what comes next */
    LargeMessage("Sorting directory entries . . .");
    SmallMessage(" Sorting directory entries . . .");

    /* Notice that we assume right input from the interface */
    SetCompareFunction(CriteriumFunctions[criterium-1].func);
    SetFilterFunction(OrderFunctions[order].func);

   /* Try opening the drive or image file. */
   if (!InitReadWriteSectors(drive, &handle)) return FALSE;
   retVal = SortDirectoryTree(handle);
   CloseReadWriteSectors(&handle);

   if (retVal) LogMessage("Directories successfully sorted.\n");
   return retVal;
}
