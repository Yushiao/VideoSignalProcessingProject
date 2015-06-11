#ifndef INTRA_H_INCLUDED
#define INTRA_H_INCLUDED

#include<iostream>
#include<cmath>
#include"Block.h"
#include"Transform.h"
#include"Common.h"
using namespace std;

class Intra{
public:
    static void intra_process(BlockSet& ori, BlockSet& result, int count, Block_8x8& table);
    static void intra_prediction(BlockSet& result, Block_8x8& square, Block_8x8& canaidate, int count, int mode);
    static void intra_recover(BlockSet& result, Block_8x8& square, Block_8x8& canaidate, Block_8x8& recover,int count, int mode);

    static void vertical_pred( BlockSet& result, Block_8x8& square, Block_8x8& vert, int count);
    static void horizontal_pred( BlockSet& result, Block_8x8& square, Block_8x8& hori, int count);

    static void ivertical_pred( BlockSet& result, Block_8x8& square, Block_8x8& vert, Block_8x8& ivert, int count);
    static void ihorizontal_pred( BlockSet& result, Block_8x8& square, Block_8x8& hori, Block_8x8& ihori, int count);

    //....

    static void reconstruct( BlockSet& result, Block_8x8* canaidate_array, int index );


};

void Intra::intra_process(BlockSet& ori, BlockSet& result, int count, Block_8x8& table)
{

    int modeNum=1;
    Block_8x8 canaidate[5];
    Block_8x8 dct_canaidate[5];
    Block_8x8 quant_canaidate[5];
    Block_8x8 iquant_canaidate[5];
    Block_8x8 idct_canaidate[5];
    Block_8x8 re_canaidate[5];

    //for(int i=0; i<ori.total; i++){
    for(int i=0; i<4; i++){
        for(int j=0; j<modeNum; j++){
            if(i%4==0){
                cout << "ori[" <<i<< "]:\n";
                ori.block[i].print();
            }
            intra_prediction( result, ori.block[i], canaidate[j], count, j);
            if(i%4==0){
                cout << "intra prediction[" <<i<< "]:\n";
                canaidate[j].print();
            }

            Transform::transform(canaidate[j], dct_canaidate[j]);
            Quantization::quantization(dct_canaidate[j], quant_canaidate[j], table);
            Quantization::iquantization(quant_canaidate[j], iquant_canaidate[j], table);
            Transform::itransform(iquant_canaidate[j], idct_canaidate[j]);
//            if(i%4==0){
//                cout << "after quantization[" <<i<< "]:\n";
//                quant_canaidate[j].print();
//                cout << "after itransform[" <<i<< "]:\n";
//                idct_canaidate[j].print();
//            }

            intra_recover( result, ori.block[i], idct_canaidate[j], re_canaidate[j], count, j);
            if(i%4==0){
                cout << "recover[" <<i<< "]:\n";
                re_canaidate[j].print();
            }

            Common::MSE( re_canaidate[j], ori.block[i]);
        }

        reconstruct( result, quant_canaidate, i);
    }
}

void Intra::intra_prediction(BlockSet& result, Block_8x8& square, Block_8x8& canaidate, int count, int mode)
{
    switch(mode){
        case 0:
            vertical_pred( result, square, canaidate, count);
            break;
        case 1:
            //horizontal_pred( result, square, canaidate, count);
            break;
        case 2:
            //horizontal_pred
            break;
        case 3:
            //horizontal_pred
            break;
        case 4:
            //horizontal_pred
            break;
        default :
            break;
    }

}

void Intra::vertical_pred( BlockSet &result, Block_8x8 &square, Block_8x8 &canaidate, int count)
{
    if(square.upper==-1){
        for(int i=0; i<count*count; i++){
            canaidate.data[i] = 128 - square.data[i];
        }
    }
    else{
        cout << "up ref: " << square.upper << endl;
        for(int i=0; i<count*count; i++){
            canaidate.data[i] = result.block[ square.upper].data[i%count+(count-1)*count];
        }
        for(int i=0; i<count*count; i++){
            canaidate.data[i] = canaidate.data[i] - square.data[i];
        }
    }
}

void Intra::horizontal_pred( BlockSet &result, Block_8x8 &square, Block_8x8 &canaidate, int count)
{
    if(square.left==-1){
        for(int i=0; i<count*count; i++){
            canaidate.data[i] = 128 - square.data[i];
        }
    }
    else{
        cout << "left ref: " << square.left << endl;
        for(int i=0; i<count*count; i++){
            canaidate.data[i] = result.block[ square.left].data[(i/count)*count-1];
        }
        for(int i=0; i<count*count; i++){
            canaidate.data[i] = canaidate.data[i] - square.data[i];
        }
    }
}

void Intra::intra_recover(BlockSet& result, Block_8x8& square, Block_8x8& canaidate, Block_8x8& recover,int count, int mode)
{
    switch(mode){
        case 0:
            ivertical_pred( result, square, canaidate, recover, count);
            break;
        case 1:
            //horizontal_pred
            break;
        case 2:
            //horizontal_pred
            break;
        case 3:
            //horizontal_pred
            break;
        case 4:
            //horizontal_pred
            break;
        default :
            break;
    }

}

void Intra::ivertical_pred( BlockSet& result, Block_8x8& square, Block_8x8& vert, Block_8x8& ivert, int count)
{
    if(square.upper==-1){
        for(int i=0; i<count*count; i++){
            ivert.data[i] = 128 - vert.data[i];
        }
    }
    else{
        cout << "up ref: " << square.upper << endl;
        for(int i=0; i<count*count; i++){
            ivert.data[i] = result.block[ square.upper].data[i%count+(count-1)*count] - vert.data[i];
        }
    }
}

void Intra::reconstruct( BlockSet& result, Block_8x8* canaidate_array, int index )
{
    for(int i=0; i<8*8; i++){
        result.block[index].data[i] = canaidate_array[0].data[i];
    }
}

#endif // INTRA_H_INCLUDED
