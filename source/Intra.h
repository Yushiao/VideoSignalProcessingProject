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
    //static void reconstruct( BlockSet& result, Block_8x8* canaidate_array, int index );

    static const int count = 8;
    static const int size = 64;
};

void Intra::intra_process(BlockSet& image, BlockSet& quantized, Block_8x8& table)
{

    int modeNum=3;
    Block_8x8 canaidate[5];
    Block_8x8 dct_canaidate[5];
    Block_8x8 quant_canaidate[5];
    Block_8x8 iquant_canaidate[5];
    Block_8x8 idct_canaidate[5];
    Block_8x8 re_canaidate[5];

    //for(int i=0; i<ori.total; i++){
    for(int i=0; i<1; i++){
        for(int j=0; j<modeNum; j++){ /// test every prediction mode
            /*if(i%4==0){
                cout << "ori[" <<i<< "]:\n";
                ori.block[i].print();
            }*/
            intra_prediction( image, image.block[i], canaidate[j], j);
            /*if(i%4==0){
                cout << "intra prediction[" <<i<< "]:\n";
                canaidate[j].print();
            }*/
            canaidate[j].print();
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
            idct_canaidate[j].setneighbor(image.block[i]);
            intra_recover( image, idct_canaidate[j], re_canaidate[j], j);

            cout << "original" << endl;
            image.block[i].print();
            cout << "recover" << endl;
            //re_canaidate[j].print();

            rerange(re_canaidate[j]);

            re_canaidate[j].print();
            cout << "quantized" << endl;
            quant_canaidate[j].print();

            /*if(i%4==0){
                cout << "recover[" <<i<< "]:\n";
                re_canaidate[j].print();
            }*/

            cout << "MSE" << Common::MSE( re_canaidate[j], image.block[i]) << endl;
        }
        /// find minimum MSE and update image
        //image.block[i] = re_canaidate[best];
        //quantized.block[i] = quant_canaidate[best];
        //quantized.block[i].mode = best;
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
    }else{
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
    }else{
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
    }else if(now.left==-1 && now.upper!=-1){
        for(int i=0; i<count; i++){ // upper
            sum += image.block[now.upper].data[(count-1)*count+i];
        }
        pred.setnumber(Common::divRound(sum,4));
    }else if(now.left!=-1 && now.upper==-1){
        for(int i=0; i<count; i++){ // left
            sum += image.block[now.left].data[i*count+count-1];
        }
        pred.setnumber(Common::divRound(sum,4));
    }else{
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

/*void Intra::reconstruct( BlockSet& result, Block_8x8* canaidate_array, int index )
{
    for(int i=0; i<8*8; i++){
        result.block[index].data[i] = canaidate_array[0].data[i];
    }
}*/

#endif // INTRA_H_INCLUDED
