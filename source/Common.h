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
    static void zigzag(int* in, int* out, const int count);
    static void zigzag(Block_8x8& in, Block_8x8& out);
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

void Common::zigzag(int* in, int* out, const int count)
{
    int* temp = new int[count*count];
    int* iter = temp;
    for(int s=0; s<2*count-1; s++){
        if(s<count){ /// upper-left block
            if(s%2==0){
                for(int i=s; i>=0; i--){
                    *iter = in[i*count+s-i];
                    iter++;
                }
            }else{
                for(int i=0; i<=s; i++){
                    *iter = in[i*count+s-i];
                    iter++;
                }
            }
        }else{ /// lower-right block
            if(s%2==0){
                for(int i=count-1; s-i<count; i--){
                    *iter = in[i*count+s-i];
                    iter++;
                }
            }else{
                for(int i=s-count+1; i<count; i++){
                    *iter = in[i*count+s-i];
                    iter++;
                }
            }
        }
    }
    for(int i=0; i<count*count; i++){
        out[i] = temp[i];
    }
    delete []temp;
}

void Common::zigzag(Block_8x8& in, Block_8x8& out)
{
    Common::zigzag(in.data, out.data, 8);
}

#endif // COMMON_H_INCLUDED
