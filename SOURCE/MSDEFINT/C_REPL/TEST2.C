#include "gdscreen.h"

char buffer[4000];

int main()
{
   int i,j;

   clrscr();

   gotoxy(5,5); printf("A");
   gotoxy(10,10); printf("A");

   gotoxy(4,4); printf("B");
   gotoxy(11,11); printf("B");

   gotoxy(14,14); printf("C");
   gotoxy(21,21); printf("C");

   GrabScreen(5, 5, 10, 10, buffer);
   DumpScreen(15, 15, 20, 20, buffer);

   getch();
}