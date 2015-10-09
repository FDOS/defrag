/*    
   Unicode.c - unicode routines.

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

/*
   To compare Unicode and ASCII we do the following:
   For every character (in any character set).
   - if high byte of unistring != 0, the string is deemed
     greater.
   - if low byte of unistring == 0, the high byte is discarded
     and the characters are compared as if they were both ascii. 
     
     Returns:  1: unicode > ascii
              -1: unicode < ascii
               0: unicode == ascii
               
    Note also that this is in no way an official way to compare
    apples and pears.
*/
int UnicodeAsciiCmp(unsigned* unistring, unsigned char* asciistring,
                    unsigned unilength)
{
   int i;     
   unsigned short uc;
   unsigned char  ac;
   
   for (i=0; *asciistring && (i < unilength); i++)
   {
       uc = unistring[i];
       ac = asciistring[i];
       
       if (uc > 0xff)
          return 1;
       else
       {
          if ((unsigned char)uc < ac) 
             return -1;
          else if ((unsigned char) uc > ac)
             return 1;
       }
   }
   
   if (*asciistring)
      return 1;
   if (i < unilength)
      return -1;
   
   return 0;  
}
