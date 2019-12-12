#include "config.h"
uchar decToBcd(uchar dec)
{
    char result1, result2;
    result1 = (dec/10)<<4;
    result2 = (dec%10);
    return result1|result2;

}
