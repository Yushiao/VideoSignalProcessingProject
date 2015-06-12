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
    static void intra_process(BlockSet& image, BlockSet& quantized, int qp);
    static void intra_prediction(BlockSet& image, Block_8x8& now, Block_8x8& residual, int mode);
    static void intra_recover(BlockSet& image, Block_8x8& now, Block_8x8& recover, int mode);

    static void intra_iprocess(BlockSet& quantized, BlockSet& image, int qp);

    static void vertical_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void horizontal_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void dc_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void diagonal_down_left_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);
    static void diagonal_down_right_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred);

    //....
    static void ftq(Block_8x8& in, Block_8x8& out, int qp); // forward transform and quantize
    static void iqt(Block_8x8& in, Block_8x8& out, int qp); // inverse quantize and transform

    static void rerange(Block_8x8& recover); // let recover block between 0~255
    static int minMSE(Block_8x8& now, Block_8x8* predArray);

    static const int count = 8;
    static const int size = 64;
    static const int modeNum = 5;
};

void Intra::intra_process(BlockSet& image, BlockSet& quantized, int qp)
{
    Block_8x8 canaidate[5];
    Block_8x8 quant_canaidate[5];
    Block_8x8 idct_canaidate[5];
    Block_8x8 re_canaidate[5];

    for(int i=0; i<image.total; i++){
        for(int j=0; j<modeNum; j++){ /// test every prediction mode

            intra_prediction( image, image.block[i], canaidate[j], j);

            ftq(canaidate[j], quant_canaidate[j], qp);

            iqt(quant_canaidate[j], idct_canaidate[j], qp);

            idct_canaidate[j].setneighbor(image.block[i]);
            intra_recover( image, idct_canaidate[j], re_canaidate[j], j);

            rerange(re_canaidate[j]);

            /*cout << "original" << endl;
            image.block[i].print();
            cout << "prediction result" <<endl;
            canaidate[j].print();
            cout << "recover" << endl;
            re_canaidate[j].print();
            cout << "quantized" << endl;
            quant_canaidate[j].print();
            cout << "MSE" << Common::MSE( re_canaidate[j], image.block[i]) << endl;*/
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
            vertical_pred(image, now, pred);
            break;
        case 1:
            horizontal_pred(image, now, pred);
            break;
        case 2:
            dc_pred(image, now, pred);
            break;
        case 3:
            diagonal_down_left_pred(image, now, pred);
            break;
        case 4:
            diagonal_down_right_pred(image, now, pred);
            break;
        default :
            break;
    }
    residual = pred - now;
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
            diagonal_down_left_pred(image, now, pred);
            break;
        case 4:
            diagonal_down_right_pred(image, now, pred);
            break;
        default :
            break;
    }
    recover = pred - now;
}

void Intra::intra_iprocess(BlockSet& quantized, BlockSet& image, int qp)
{
    Block_8x8 idct;
    Block_8x8 reconstruct;

    for(int i=0; i<quantized.total; i++){
        iqt(quantized.block[i], idct, qp);

        idct.setneighbor(quantized.block[i]);
        intra_recover( image, idct, reconstruct, quantized.block[i].mode);

        rerange(reconstruct);

        image.block[i] = reconstruct;
    }
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
    int upper_right=-1;
    if(now.upper!=-1){
        upper_right = image.block[now.upper].right;
    }

    int ref[2*count];

    if( now.upper!=-1 && upper_right!= -1 ){
        for(int i=0; i<count; i++){
            ref[i] = image.block[now.upper].data[count*(count-1)+i];
        }
        for(int i=0; i<count; i++){
            ref[i+count] = image.block[upper_right].data[count*(count-1)+i];
        }
    }
    else if(  now.upper==-1 && upper_right!= -1 ){
        for(int i=0; i<count; i++){
            ref[i] = 128;
        }
        for(int i=0; i<count; i++){
            ref[i+count] = image.block[upper_right].data[count*(count-1)+i];
        }
    }
    else if(  now.upper!=-1 && upper_right== -1 ){
        for(int i=0; i<count; i++){
            ref[i] = image.block[now.upper].data[count*(count-1)+i];
        }
        for(int i=0; i<count; i++){
            ref[i+count] = 128;
        }
    }
    else{
        for(int i=0; i<2*count; i++){
            ref[i] = 128;
        }
    }

    for(int i=0; i<count*count; i++){
        pred.data[i] = (ref[i/count+i%count]+ref[i/count+i%count+1])/2;
    }
}

void Intra::diagonal_down_right_pred(BlockSet& image, Block_8x8& now, Block_8x8& pred)
{
    int upper_ref[count];
    int left_ref[count];

    if(now.left!=-1){
        for(int i=0; i<count; i++){ // left
            left_ref[i] = image.block[now.left].data[i*count+count-1];
        }
    }
    else{
        for(int i=0; i<count; i++){ // left
            left_ref[i] = 128;
        }
    }
    if(now.upper!=-1){
        for(int i=0; i<count; i++){ // upper
            upper_ref[i] = image.block[now.upper].data[(count-1)*count+i];
        }
    }
    else{
        for(int i=0; i<count; i++){ // upper
            upper_ref[i] = 128;
        }
    }

    for(int i=0; i<count; i++){
        for(int j=0; j<count; j++){
            if(i<j){ // upper-right plane
                pred.data[i*count+j] = (upper_ref[j-i-1]+upper_ref[j-i])/2;
            }
            else if(i>j){ // bottom-left plane
                pred.data[i*count+j] = (left_ref[i-j-1]+left_ref[i-j])/2;
            }
            else{ // i=j, diagonal
                pred.data[i*count+j] = (upper_ref[0]+left_ref[0])/2;
            }
        }
    }
}

void Intra::ftq(Block_8x8& in, Block_8x8& out, int qp)
{
    double* dctin = new double[size];
    double* dctout = new double[size];
    for(int i=0; i<size; i++){
        dctin[i] = in.data[i];
    }
    Transform::dct2d(dctin, dctout, 8);
    // forward quantize
    static const double qst[52] = {0.625, 0.6875, 0.8125, 0.875, 1, 1.125, 1.25, 1.375, 1.625, 1.75, 2, 2.25, 2.5, 2.75, 3.25, 3.5, 4, 4.5, 5, 5.5, 6.5, 7, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 26, 28, 32, 36, 40, 44, 52, 56, 64, 72, 80, 88, 104, 112, 128, 144, 160, 176, 208, 224};
    double qstep = qst[qp];
    for(int i=0; i<size; i++){
        dctout[i] = dctout[i]/qstep;
        out.data[i] = round(dctout[i]);
    }
    delete []dctin;
    delete []dctout;
}

void Intra::iqt(Block_8x8& in, Block_8x8& out, int qp)
{
    double* dctin = new double[size];
    double* dctout = new double[size];
    // inverse quantize
    static const double qst[52] = {0.625, 0.6875, 0.8125, 0.875, 1, 1.125, 1.25, 1.375, 1.625, 1.75, 2, 2.25, 2.5, 2.75, 3.25, 3.5, 4, 4.5, 5, 5.5, 6.5, 7, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 26, 28, 32, 36, 40, 44, 52, 56, 64, 72, 80, 88, 104, 112, 128, 144, 160, 176, 208, 224};
    double qstep = qst[qp];

    for(int i=0; i<size; i++){
        dctin[i] = in.data[i]*qstep;
    }
    Transform::idct2d(dctin, dctout, 8);
    for(int i=0; i<size; i++){
        out.data[i] = round(dctout[i]);
    }
    delete []dctin;
    delete []dctout;
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
