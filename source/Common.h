#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include<iostream>
#include"Block.h"

using namespace std;

class Common{
public:
    static int divCeil(int dividend, int divisor);
    static int divRound(int dividend, int divisor);
    static double MSE(int* arr0, int* arr1, int size);
    static double MSE(Block_8x8 b0, Block_8x8 b1);
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

double Common::MSE(int* arr0, int* arr1, int size)
{
    double result = 0.0;
    for(int i=0; i<size; i++){
        result += (arr0[i]-arr1[i])*(arr0[i]-arr1[i]);
    }
    return result/size;
}

double Common::MSE(Block_8x8 b0, Block_8x8 b1)
{
    return Common::MSE(b0.data, b1.data, 64);
}

#endif // COMMON_H_INCLUDED
