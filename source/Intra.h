#ifndef INTRA_H_INCLUDED
#define INTRA_H_INCLUDED

#include<iostream>
#include<cmath>
#include"Block.h"
#include"Transform.h"
#include"Quantization.h"
#include"Common.h"
using namespace std;

class Intra{
public:
    static void intra_process(BlockSet& image, BlockSet& quantized, Block_8x8& table);
    static void intra_prediction(BlockSet& image, Block_8x8& now, Block_8x8& residual, int mode);
    static void intra_recover(BlockSet& image, Block_8x8& now, Block_8x8& recover, int mode);

    static void vertical_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void horizontal_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void dc_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void diagonal_down_left_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void diagonal_down_right_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);

    //....
    static void rerange(Block_8x8& recover); // let recover block between 0~255
    static int minMSE(Block_8x8& now, Block_8x8* predArray);

    static const int count = 8;
    static const int size = 64;
    static const int modeNum=3;
};

void Intra::intra_process(BlockSet& image, BlockSet& quantized, Block_8x8& table)
{
    Block_8x8 canaidate[5];
    Block_8x8 dct_canaidate[5];
    Block_8x8 quant_canaidate[5];
    Block_8x8 iquant_canaidate[5];
    Block_8x8 idct_canaidate[5];
    Block_8x8 re_canaidate[5];

    for(int i=0; i<image.total; i++){
    //for(int i=0; i<1; i++){
        for(int j=0; j<modeNum; j++){ /// test every prediction mode

            intra_prediction( image, image.block[i], canaidate[j], j);

            Transform::transform(canaidate[j], dct_canaidate[j]);
            Quantization::quantization(dct_canaidate[j], quant_canaidate[j], table);
            Quantization::iquantization(quant_canaidate[j], iquant_canaidate[j], table);
            Transform::itransform(iquant_canaidate[j], idct_canaidate[j]);
//            cout << "after quantization[" <<i<< "]:\n";
//            quant_canaidate[j].print();
//            cout << "after itransform[" <<i<< "]:\n";
//            idct_canaidate[j].print();
            idct_canaidate[j].setneighbor(image.block[i]);
            intra_recover( image, idct_canaidate[j], re_canaidate[j], j);
            rerange(re_canaidate[j]);

            cout << "original" << endl;
            image.block[i].print();
            cout << "prediction result" <<endl;
            canaidate[j].print();
            cout << "recover" << endl;
            re_canaidate[j].print();
            cout << "quantized" << endl;
            quant_canaidate[j].print();

            cout << "MSE" << Common::MSE( re_canaidate[j], image.block[i]) << endl;
        }
        /// find minimum MSE and update image
        int best = minMSE(image.block[i], re_canaidate);
        image.block[i] = re_canaidate[best];
        quantized.block[i] = quant_canaidate[best];
        quantized.block[i].mode = best;
    }
}

void Intra::intra_prediction(BlockSet& image, Block_8x8& now, Block_8x8& residual, int mode)
{
    Block_8x8 pred;
    switch(mode){
        case 0:
            vertical_pred( image, now, pred);
            break;
        case 1:
            horizontal_pred( image, now, pred);
            break;
        case 2:
            dc_pred(image, now, pred);
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
    residual = pred - now;
}

void Intra::vertical_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred)
{
    if(now.upper==-1){
        pred.setnumber(128);
    }
    else{
        for(int i=0; i<size; i++){
            pred.data[i] = image.block[now.upper].data[i%count+(count-1)*count];
        }
        image.block[now.upper].print();
    }
}

void Intra::horizontal_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred)
{
    if(now.left==-1){
        pred.setnumber(128);
    }
    else{
        for(int i=0; i<size; i++){
            pred.data[i] = image.block[now.left].data[(i/count)*count + count-1];
        }
    }
}

void Intra::dc_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred)
{
    int sum = 0;
    if(now.left!=-1 && now.upper!=-1){
        for(int i=0; i<count; i++){ // left
            sum += image.block[now.left].data[i*count+count-1];
        }
        for(int i=0; i<count; i++){ // upper
            sum += image.block[now.upper].data[(count-1)*count+i];
        }
        pred.setnumber(Common::divRound(sum,8));
    }
    else if(now.left==-1 && now.upper!=-1){
        for(int i=0; i<count; i++){ // upper
            sum += image.block[now.upper].data[(count-1)*count+i];
        }
        pred.setnumber(Common::divRound(sum,4));
    }
    else if(now.left!=-1 && now.upper==-1){
        for(int i=0; i<count; i++){ // left
            sum += image.block[now.left].data[i*count+count-1];
        }
        pred.setnumber(Common::divRound(sum,4));
    }
    else{
        pred.setnumber(128);
    }
}

void Intra::diagonal_down_left_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred)
{

}

void Intra::diagonal_down_right_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred)
{

}

void Intra::intra_recover(BlockSet& image, Block_8x8& now, Block_8x8& recover, int mode)
{
    Block_8x8 pred;
    switch(mode){
        case 0:
            vertical_pred(image, now, pred);
            break;
        case 1:
            horizontal_pred(image, now, pred);
            break;
        case 2:
            dc_pred(image, now, pred);
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
    recover = pred - now;
}

void Intra::rerange(Block_8x8& recover)
{
    for(int i=0; i<size; i++){
        if(recover.data[i] < 0) recover.data[i] = 0;
        if(recover.data[i] > 255) recover.data[i] = 255;
    }
}

int Intra::minMSE(Block_8x8& now, Block_8x8* predArray)
{
    int minTerm=0;
    double minValue = Common::MSE( predArray[0], now );

    for(int i=1; i<modeNum; i++){
        if( minValue > Common::MSE( predArray[i], now )){
            minValue = Common::MSE( predArray[i], now );
            minTerm = i;
        }
    }
    return minTerm;
}

#endif // INTRA_H_INCLUDED
