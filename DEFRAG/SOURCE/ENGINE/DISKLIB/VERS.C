#include <stdio.h>

#include "disklib.h"

int main()
{
    printf("\nLib Ver: %x\n",lib_ver());
    printf("OS Ver: %x\n",dos_ver());
    printf("Win Ver: %x\n",win_ver());
    return 0;
}
