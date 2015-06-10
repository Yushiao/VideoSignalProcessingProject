#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include<iostream>

using namespace std;

class Common{
public:
    static int divCeil(int dividend, int divisor);
    static int divRound(int dividend, int divisor);
};

int Common::divCeil(int dividend, int divisor)
{
    /// dividend and divisor must be positive
    return ((dividend-1) / divisor) + 1;
}

int Common::divRound(int dividend, int divisor)
{
    /// divisor must be positive
    if(dividend < 0){
        return -1*(-1*dividend + (divisor/2))/divisor;
    }else{
        return (dividend + (divisor/2))/divisor;
    }
}

#endif // COMMON_H_INCLUDED
