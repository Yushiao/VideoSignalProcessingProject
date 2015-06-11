#ifndef QUANTIZATION_H_INCLUDED
#define QUANTIZATION_H_INCLUDED

#include<iostream>
#include<cmath>

#include"Block.h"
#include"Common.h"

using namespace std;

class Quantization{
public:
    static void createTable(int quality, int* luma, int* chroma, const int count);
    static void createTable(int quality, Block_8x8& luma, Block_8x8& chroma);
    static void quantization(int* in, int* out, int* table, const int count);
    static void quantization(Block_8x8& in, Block_8x8& out, const Block_8x8& table);
    static void iquantization(int* in, int* out, int* table, const int count);
    static void iquantization(Block_8x8& in, Block_8x8& out, const Block_8x8& table);
};

void Quantization::createTable(int quality, int* luma, int* chroma, const int count)
{
    static const int lumaTable[] = {
      16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68, 109, 103, 77,
      24, 35, 55, 64, 81, 104, 113, 92,
      49, 64, 78, 87, 103, 121, 120, 101,
      72, 92, 95, 98, 112, 100, 103, 99
    };

    static const int chromaTable[] = {
      17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99
    };

    /// quality quantization table
    int q = (quality<50) ? (5000/quality) : (200-2*quality);
    for(int i=0; i<count*count; i++){
        int lumaTemp = Common::divRound(lumaTable[i]*q, 100);
        int chromaTemp = Common::divRound(chromaTable[i]*q, 100);
        if(lumaTemp<=0) lumaTemp=1;
        if(chromaTemp<=0) chromaTemp=1;
        luma[i] = lumaTemp;
        chroma[i] = chromaTemp;
    }
}

void Quantization::createTable(int quality, Block_8x8& luma, Block_8x8& chroma)
{
    Quantization::createTable(quality, luma.data, chroma.data, 8);
}

void Quantization::quantization(int* in, int* out, int* table, const int count)
{
    int size = count*count;
    for(int i=0; i<size; i++){
        out[i] = Common::divRound(in[i], table[i]);
    }
}

void Quantization::quantization(Block_8x8& in, Block_8x8& out, const Block_8x8& table)
{
    Quantization::quantization(in.data, out.data, table.data, 8);
}

void Quantization::iquantization(int* in, int* out, int* table, const int count)
{
    int size = count*count;
    for(int i=0; i<size; i++){
        out[i] = in[i] * table[i];
    }
}

void Quantization::iquantization(Block_8x8& in, Block_8x8& out, const Block_8x8& table)
{
    Quantization::iquantization(in.data, out.data, table.data, 8);
}

#endif // QUANTIZATION_H_INCLUDED
