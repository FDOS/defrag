#include <stdio.h>
#include <time.h>

int main()
{
   time_t t1, t2;
   time(&t1);    

   WasteTime(19);

   time(&t2);

   printf("%u", t2-t1);
}
