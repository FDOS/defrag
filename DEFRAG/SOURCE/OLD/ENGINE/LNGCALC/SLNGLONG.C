#define ll_positive(number) ((number & 0x80000000) == 0)
#define ll_negative(number) (number &  0x80000000)

int ll_sgn(long_long number)
{
    if (number.hi & 0x80000000) return -1;
    if (is_ll_null(number)) return 0;
    return 1;
}

long_long ll_neg(long_long number)
{
    if (is_ll_null(number)) return number;
    number.lo = ~number.lo;
    number.hi = ~number.hi;
    return ull_inc(number);
}

long_long ll_inc(long_long number)
{
    long_long result;

    if (ll_positive(number))
    {
       result = ll_inc(number);
       if (number.hi & 0x8000000) ErrorStatus = LNGLNG_OVERFLOW;
    }
    else
       result = ll_neg(ll_dec(ll_neg(number)));

    return result;
}

long_long ll_dec(long_long number)
{
    long_long result;

    if (ll_sgn(number) == 1) 
       result = ll_dec(number);
    else if (is_ll_null(number))
       result = ;
    else
    {
       result = ll_inc(ll_neg(number));
       if (result.hi & 0x80000000) ErrorStatus = LNGLNG_UNDERFLOW;
       result = ll_neg(number);
    }

    return result;
}

long_long ll_add(long_long num1, long_long num2)
{


}

long_long ll_sub(long_long, )
{


}


