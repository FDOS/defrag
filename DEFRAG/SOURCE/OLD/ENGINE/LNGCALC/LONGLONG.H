#ifndef LONG_LONG_H_
#define LONG_LONG_H_

struct LongLongStruct {
   unsigned long hi;
   unsigned long lo;
};

typedef struct LongLongStruct long_long;

long_long ull_add(long_long term1, long_long term2);
long_long ull_sub(long_long subtractnumber, long_long subtracter);
long_long ull_mul(long_long factor1, long_long factor2);
long_long ull_div(long_long dividenumber, long_long divider);

long_long ultoull(unsigned long number);
long_long utoull(unsigned number);

#define ulltoul(number) number.lo
#define ulltou(number) ((unsigned) number.lo)

long_long atoull(char* number);
char*     ulltoa(long_long number, char* result);

int is_ull_smaller(long_long num1, long_long num2);
int is_ull_larger(long_long num1, long_long num2);

long_long ull_max(void);
long_long ull_min(void);

#define is_ull_null(number) ((number.lo == 0) && (number.hi == 0))

#define is_ull_equal(num1, num2) ((num1.hi == num2.hi) && (num1.lo == num2.lo))

long_long hextoull(char* hex);
char* ulltohex(long_long num, char* result);

long_long ull_inc(long_long term);
long_long ull_dec(long_long term);

long_long ull_mul2(long_long fact);
long_long ull_mul256(long_long fact);
long_long ull_mul65536(long_long fact);
long_long ull_mul2P32(long_long fact);

long_long ull_div2(long_long divnum);
long_long ull_div256(long_long fact);
long_long ull_div65536(long_long fact);
long_long ull_div2P32(long_long fact);

long_long ull_mod(long_long divnum, long_long divider);

/* Basic error handling. */
#define LNGLNG_OK        0  /* No error encountered.              */
#define LNGLNG_OVERFLOW  1  /* Overflow detected.                 */
#define LNGLNG_UNDERFLOW 2  /* Underflow detected.                */
#define LNGLNG_DIVBY0    3  /* Division by zero.                  */
#define LNGLNG_INVALID   4  /* Invalid character in input string. */

int Read_ull_status(void);   /* Read & Clear error status.         */

#endif
