/*    
   Longlong.c - 64 bit integer support.
   
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

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "longlong.h"
#include "..\..\misc\bool.h"

static int ErrorStatus = LNGLNG_OK;

int Read_ull_status(void)
{
    int result = ErrorStatus;

    ErrorStatus = LNGLNG_OK;
    return result;
}

long_long ultoull(unsigned long number)
{
   long_long result = {0, 0};
   result.lo        = number;
   return result;
}

long_long utoull(unsigned number)
{
   return ultoull((unsigned long) number);
}

static int hexdigittoint(char digit)
{
   if ((digit >= '0') && (digit <= '9'))
      return (int) (digit - '0');
   else if ((digit >= 'a') && (digit <= 'f'))
      return (int) (digit - 'a') + 0xa;
   else if ((digit >= 'A') && (digit <= 'F'))
      return (int) (digit - 'A') + 0xA;
   else
   {
      ErrorStatus = LNGLNG_INVALID;
      return 0;
   }
}

long_long hextoull(char* hex)
{
   size_t len;
   int i, j = 0;
   long_long result = {0,0};
   unsigned long radix = 1;
   unsigned long* p;
        
   len = strlen(hex);
   if (len > 16)
   {
      ErrorStatus = LNGLNG_OVERFLOW;
      return result;
   }

   if (hex[0] == 0) return result;
   if ((hex[0] == '0') && (hex[1] == 'x')) hex += 2;
   
   p = &result.lo;
   for (i = len-1; i >= 0; i--, j++)
   {
       if (j == 8) 
       {
	  p = &result.hi;
	  radix = 1;
       }
       
       *p += (unsigned long) hexdigittoint(hex[i]) * radix;
       if (ErrorStatus == LNGLNG_INVALID) break; /* Wrong error status */
       
       radix *= 16;
   }

   return result;
}

char* ulltohex(long_long num, char* result)
{       
     int i, len, len1;
     char* buflo = "00000000";

     if (num.hi == 0)
     {
	ultoa(num.lo, result, 16);
     }
     else
     {
	ultoa(num.hi, result, 16);
	len1 = strlen(result);
	ultoa(num.lo, buflo, 16);
	len = 8 - strlen(buflo);
	memset(result + len1, '0', len);
	result[len1+len] = 0;
	strcat(result, buflo);        
     }

     return result;
}

int is_ull_smaller(long_long num1, long_long num2)
{
   if (num1.hi < num2.hi) return TRUE;
   if (num1.hi > num2.hi) return FALSE;
   return num1.lo < num2.lo;
}

int is_ull_larger(long_long num1, long_long num2)
{
   return !is_ull_equal(num1, num2) && !is_ull_smaller(num1, num2);
}

long_long ull_max(void)
{
   long_long result = {ULONG_MAX, ULONG_MAX};
   return result;
}

long_long ull_min(void)
{
   long_long result = {0, 0};
   return result;
}
long_long ull_inc(long_long term)
{
   long_long result;

   result = term;
   if (term.lo == ULONG_MAX)
   {
      if (term.hi == ULONG_MAX) ErrorStatus = LNGLNG_OVERFLOW;
      result.lo = 0;
      result.hi++;
   }
   else
      result.lo++;

   return result;
}

long_long ull_dec(long_long term)
{
   long_long result;

   result = term;
   if (term.lo == 0)
   {
      if (term.hi == 0) ErrorStatus = LNGLNG_UNDERFLOW;
      result.lo = ULONG_MAX;
      result.hi--;
   }
   else
      result.lo--;

   return result;
}

long_long ull_add(long_long term1, long_long term2)
{
   unsigned long ov;
   long_long result = {0,0};

   if (is_ull_null(term1)) return term2;
   if (is_ull_null(term2)) return term1;

   if (term1.lo > 0)
   {
      ov = ULONG_MAX - term1.lo + 1;
      if (ov > term2.lo)
	 result.lo = term1.lo + term2.lo;
      else
      {
	 result.lo = term2.lo - ov;
	 result.hi = 1;
      }
   }
   else
      result.lo = term2.lo;

   ov = ULONG_MAX - term1.hi;
   if (ov < term2.hi) 
      ErrorStatus = LNGLNG_OVERFLOW; /* Result not specified when overflow. */
   else
   {
      ov = term1.hi + term2.hi;
      if ((ov == ULONG_MAX) && (result.hi == 1)) 
	 ErrorStatus = LNGLNG_OVERFLOW;
      else
	 result.hi += ov;
   }
   return result;
}

long_long ull_sub(long_long term1, long_long term2)
{
   unsigned long ov;
   long_long result = {0,0};

   if (is_ull_equal(term1, term2))   return result;
   if (is_ull_smaller(term1, term2))
   {
      ErrorStatus = LNGLNG_UNDERFLOW;
      return result;
   }
   
   if (is_ull_null(term2))           return term1;

   result.hi = term1.hi;

   if (term2.lo == 0)
      result.lo = term1.lo;
   else if (term1.lo < term2.lo)
   {
      ov = ULONG_MAX - term2.lo;
      result.lo = ov + term1.lo + 1;
      result.hi--;
   }
   else
      result.lo = term1.lo - term2.lo;

   result.hi -= term2.hi;

   return result;
}

long_long ull_mul2(long_long fact)
{
   long_long result;
   unsigned long bit = fact.lo >> 31;

   if (fact.hi >> 31) ErrorStatus = LNGLNG_OVERFLOW;

   result = fact;
   result.lo <<= 1;
   result.hi <<= 1;
   result.hi += bit;

   return result;
}

long_long ull_mul256(long_long fact)
{
   long_long result;

   if (fact.hi > 0xffffff) 

   result.lo = fact.lo << 8;
   result.hi = fact.hi << 8;

   result.hi += fact.lo >> 24;

   return result;
}

long_long ull_mul65536(long_long fact)
{
        return ull_mul256(ull_mul256(fact));
}

long_long ull_mul2P32(long_long fact)
{
        long_long result = {0,0};
        
        if (fact.hi)
           ErrorStatus = LNGLNG_OVERFLOW;
        else
           result.hi = fact.lo;
           
        return result;
}

long_long ull_mul(long_long fact1, long_long fact2)
{
   long_long result = {0,0};
   long_long temp;
   unsigned long results[2][4];
   int i,j, k;

   if ((fact1.hi > 0) && (fact2.hi > 0)) ErrorStatus = LNGLNG_OVERFLOW;
   
   if (is_ull_smaller(fact2, fact1)) return ull_mul(fact2, fact1);

   results[0][0] = (unsigned long)
		   (unsigned) fact1.lo * (unsigned) fact2.lo;
   results[0][1] = (unsigned long)
		   (unsigned) fact1.lo * *(((unsigned*) &fact2.lo)+1);
   results[0][2] = (unsigned long)
		   (unsigned) fact1.lo * (unsigned) fact2.hi;
   results[0][3] = (unsigned long)
		   (unsigned) fact1.lo * *(((unsigned*) &fact2.hi)+1);

   results[1][0] = (unsigned long)
		   *(((unsigned*) &fact1.lo)+1) * (unsigned) fact2.lo;
   results[1][1] = (unsigned long)
		   *(((unsigned*) &fact1.lo)+1) * *(((unsigned*) &fact2.lo)+1);
   results[1][2] = (unsigned long)
		   *(((unsigned*) &fact1.lo)+1) * (unsigned) fact2.hi;
   results[1][3] = (unsigned long)
		   *(((unsigned*) &fact1.lo)+1) * *(((unsigned*) &fact2.hi)+1);


   for (i = 0; i < 2; i++)
   {
       for (j = 0; j < 4; j++)
       {
	   temp = ultoull(results[i][j]);

	   for (k = 0; k < i+j; k++)
	       temp = ull_mul65536(temp);

	   result = ull_add(temp, result);
       }
   }

   return result;
}

long_long ull_div2(long_long divnum)
{
  long_long result;
  unsigned long bit = divnum.hi << 31;

  result = divnum;
  result.lo >>= 1;
  result.hi >>= 1;
  result.lo += bit;

  return result;
}

long_long ull_div256(long_long fact)
{
   long_long result;

   result.lo = fact.lo >> 8;
   result.hi = fact.hi >> 8;

   result.lo += fact.hi << 24;

   return result;
}

long_long ull_div65536(long_long fact)
{
   return ull_div256(ull_div256(fact));
}

long_long ull_div2P32(long_long fact)
{
   long_long result = {0,0};
   
   result.lo = fact.hi;
      
   return result;
}

static long_long ull_div_bin_search(long_long divnum, long_long divider)
{
    long_long result = {0,0}, lo = {0,0}, hi = {0, 1}, temp;
    int i = 0;

    if (ErrorStatus != LNGLNG_OK) return result;

    /* Calculate lo and hi. */
    temp = divider;
    while (!is_ull_null(temp))
    {
	  temp = ull_div2(temp);
	  i++;
    }
    
    while (i--) lo = ull_div2(temp);
    hi = ull_mul2(lo);    

    for (;;)
    {
	result = ull_add(lo, ull_div2(ull_sub(hi, lo)));
	ErrorStatus = LNGLNG_OK;               /* Disregard any overflow. */

	temp = ull_mul(result, divider);

	if (ErrorStatus == LNGLNG_OK)
	{
	   if (is_ull_equal(temp, divnum)) return result;

	   if (is_ull_smaller(temp, divnum))
	   {
	      temp = ull_add(temp, divider);
	      if (is_ull_larger(temp, divnum)) return result;
	      if (is_ull_equal(temp, divnum))  return ull_inc(result);
	      lo = result;
	   }
	   else
	      hi = result;
	}
	else /* if (Read_ll_status() == LNGLNG_OVERFLOW) */
	   hi = result;
    }
}

long_long ull_div(long_long divnum, long_long divider)
{
    long_long result = {0,0}, num1 = {0,1};

    if (is_ull_null(divider))
    {
       ErrorStatus = LNGLNG_DIVBY0;
       return result;
    }

    if (is_ull_null(divnum)) return result;
    if (is_ull_equal(divnum, divider)) return num1;

    if (divider.hi & 0x8000000)
    {
       if (is_ull_smaller(divnum, divider)) return result;
       return num1;
    }
    
    if (divider.hi == 0)
    {
       if (divider.lo == 1) return divnum;

       if (divnum.hi == 0)
       {
	  result.lo = divnum.lo / divider.lo;
	  return result;
       }

       if ((divnum.hi % divider.lo == 0))
       {
	  result.hi = divnum.hi / divider.lo;
	  result.lo = divnum.lo / divider.lo;
	  return result;
       }
    }

    return ull_div_bin_search(divnum, divider);
}

long_long ull_mod(long_long divnum, long_long divider)
{
   return ull_sub(divnum, ull_mul(ull_div(divnum, divider), divider));
}

char* ulltoa(long_long number, char* result)
{
    int i;
    long_long temp = number, temp1, num10 = {0,10};
    char* temp2 = result;

    while (!is_ull_null(temp))
    {
       temp1 = ull_div(temp, num10);
       *result++ = (char)ulltou(ull_sub(temp, ull_mul(temp1, num10)))+'0';
       temp = temp1;
    }

    *result = 0;

    return strrev(temp2);
}

long_long atoull(char* number)
{
   int i;
   long_long result = {0,0}, radix = {0,1}, num10 = {0,10};

   if (number[0] == 0)   return result;
   if (number[1] == 'x') return hextoull(number+2);

   for (i = 0; i < strlen(number); i++)
   {
       if (!isdigit(number[i])) ErrorStatus = LNGLNG_INVALID;
       result = ull_add(result, ull_mul(radix, utoull(number[i] - '0')));
       radix = ull_mul(radix, num10);
   }

   return result;
}


