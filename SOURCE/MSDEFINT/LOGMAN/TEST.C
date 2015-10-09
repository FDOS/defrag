int main(int argc, char *argv[])
{
int i;

//   for (i=0; i<20; i++)
//   PrintChar1('A');

   gotoxy1(10,10); PrintChar1('C');
   gotoxy1(20,20); PrintChar1('D');
   gotoxy1(2,2); PrintChar1('C'); PrintChar1('D');

   for (i = 0; i < 22; i++)
   Scroll1Up();

   ShowScreenPage(1);

   getch();

   ShowScreenPage(0);
   
 return 0;
}
