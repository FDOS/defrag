/* demonstrate bug in watcom compiler */

#include <stdio.h>
#include <malloc.h>

int main()
{
#ifdef BUG              /* define BUG and see what happens */
unsigned short i;
#else
unsigned int i;
#endif
int __huge *buf;

    if ((buf = (int __huge *)halloc(50000L,sizeof(int))) == NULL)
        return 0;

    for (i = 0; i < 50000U; i++)
        buf[i] = i;

    printf("ok");

    return 1;
}
