#include "c-repl.h"

int main(int argc, char** argv)
{
   char copy[20];

   printf("%d\n", StringLength("Hello world!"));

   StringCopy(copy, "I LOVE FreeDOS!");

   printf("%d\n", StringLength(copy));
   puts(copy);

}