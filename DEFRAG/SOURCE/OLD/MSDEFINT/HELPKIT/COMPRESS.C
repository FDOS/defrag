#include <process.h>
#include <stdlib.h>

#include "mkhelp.h"

int CompressFile(char* infile, char* outfile)
{
    return spawnl(P_WAIT, "huf", "huf", infile, outfile, NULL) & 255;
}
