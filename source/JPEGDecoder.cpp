/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * JPEG Decoder
 */
#include<iostream>
#include<fstream>
#include<cmath>
#include<vector>
#include<string>
#include<algorithm>

#include"../En/Block.h"
#include"../En/Common.h"
#include"../En/Transform.h"
#include"../En/Quantization.h"
#include"../En/Intra.h"
#include"../En/Entropy.h"

using namespace std;

class JPEGDecoder{
public:
    JPEGDecoder();
    ~JPEGDecoder();

    void decode(const char* filename);

private:
    string name;
    int width;
    int height;
    int quality;
    vector<bool> bitstream; /// entropy coding

    static const int blockSize=8;
    int blockTotal;
    // reconstructed block
    BlockSet Y;
    BlockSet U;
    BlockSet V;

    // quantization table
    Block_8x8 lumaQTable;
    Block_8x8 chromaQTable;

    unsigned char* image;

    /// sub function
    void initial(const char* filename);
    void entropy();
    void rasterscan();
    void quantization();
    void transform();
    void partition();
    void write();
    int bitstreamDecode(int type);
    int bitstreamNumber(int length);
    void zigzag(int* in, int* out, const int count);
    void levelShiftBack(Block_8x8& in, Block_8x8& out);

    void itq(Block_8x8& in, Block_8x8& out);
};

JPEGDecoder::JPEGDecoder()
{
    width = 0;
    height = 0;
    quality = 0;
}

JPEGDecoder::~JPEGDecoder()
{
    delete []image;
    bitstream.clear();
}

void JPEGDecoder::decode(const char* filename)
{
    initial(filename);
    Entropy::entropyDecode(bitstream, Y, U, V);
    rasterscan();
    Intra::intra_iprocess(Y, Y, quality);
    Intra::intra_iprocess(U, U, quality);
    Intra::intra_iprocess(V, V, quality);
    partition(); /// restore image
    write();
}

void JPEGDecoder::initial(const char* filename)
{
    name = filename;
    name = name.substr(name.rfind('\\')+1);
    name = name.substr(0, name.rfind('.'));

    int header = 5;
    fstream fbin(filename, fstream::in | fstream::binary);
    if(!fbin.is_open()){
        cout << "open failed" << endl;
    }
    /// read all binary file to buffer array
    fbin.seekg(0, fbin.end);
    int length = fbin.tellg();
    fbin.seekg(0, fbin.beg);

    char *buffer = new char[length];
    fbin.read(buffer, length);
    fbin.close();

    /// initial parameter from header
    unsigned char temp0;
    unsigned char temp1;
    temp0 = buffer[0];
    temp1 = buffer[1];
    width = temp0*256 + temp1;
    temp0 = buffer[2];
    temp1 = buffer[3];
    height = temp0*256 + temp1;
    temp0 = buffer[4];
    quality = temp0;

    /// change char array to bit array
    for(int i=header; i<length; i++){
        for(int j=0; j<8; j++){
            bitstream.push_back(buffer[i] & (1<<(8-1-j)));
        }
    }
    delete []buffer;

    reverse(bitstream.begin(), bitstream.end());

    /// initial block image
    int size = width*height*3/2; /// YCbCr 4:2:0
    image = new unsigned char[size];

    int blockWidthLuma, blockHeightLuma;
    int blockWidthChroma, blockHeightChroma;
    blockWidthLuma = Common::divCeil(width, blockSize);
    blockHeightLuma = Common::divCeil(height, blockSize);
    blockWidthChroma = Common::divCeil(width/2, blockSize);
    blockHeightChroma = Common::divCeil(height/2, blockSize);
    blockTotal = blockWidthLuma*blockHeightLuma+2*blockWidthChroma*blockHeightChroma;

    Y.initial(blockWidthLuma*blockHeightLuma);
    U.initial(blockWidthChroma*blockHeightChroma);
    V.initial(blockWidthChroma*blockHeightChroma);

    Quantization::createTable(quality, lumaQTable, chromaQTable);

    for(int m=0; m<blockHeightLuma; m++){ /// loop all luma blocks
        for(int n=0; n<blockWidthLuma; n++){
            Y.block[m*blockWidthLuma+n].upper = (m==0) ? -1 : (m-1)*blockWidthLuma+n;
            Y.block[m*blockWidthLuma+n].left = (n==0) ? -1 : m*blockWidthLuma+n-1;
            Y.block[m*blockWidthLuma+n].right = (n==blockWidthLuma-1) ? -1 : m*blockWidthLuma+n+1;
        }
    }
    for(int m=0; m<blockHeightChroma; m++){ /// loop all chroma blocks
        for(int n=0; n<blockWidthChroma; n++){
            U.block[m*blockWidthChroma+n].upper = (m==0) ? -1 : (m-1)*blockWidthChroma+n;
            U.block[m*blockWidthChroma+n].left = (n==0) ? -1 : m*blockWidthChroma+n-1;
            U.block[m*blockWidthChroma+n].right = (n==blockWidthChroma-1) ? -1 : m*blockWidthChroma+n+1;
        }
    }
    for(int m=0; m<blockHeightChroma; m++){ /// loop all chroma blocks
        for(int n=0; n<blockWidthChroma; n++){
            V.block[m*blockWidthChroma+n].upper = (m==0) ? -1 : (m-1)*blockWidthChroma+n;
            V.block[m*blockWidthChroma+n].left = (n==0) ? -1 : m*blockWidthChroma+n-1;
            V.block[m*blockWidthChroma+n].right = (n==blockWidthChroma-1) ? -1 : m*blockWidthChroma+n+1;
        }
    }
}

void JPEGDecoder::rasterscan()
{
    /// reorder block to raster scan form
    int blockElement = blockSize*blockSize;
    int* in = new int[blockElement];
    int* out = new int[blockElement];
    for(int i=0; i<blockElement; i++){
        in[i] = i;
    }
    Common::zigzag(in, out, blockSize);

    for(int i=0; i<Y.total; i++){
        for(int j=0; j<blockElement; j++){
            in[out[j]] = Y.block[i].data[j];
        }

        for(int j=0; j<blockElement; j++){
            Y.block[i].data[j] = in[j];
        }
    }
    for(int i=0; i<U.total; i++){
        for(int j=0; j<blockElement; j++){
            in[out[j]] = U.block[i].data[j];
        }

        for(int j=0; j<blockElement; j++){
            U.block[i].data[j] = in[j];
        }
    }
    for(int i=0; i<V.total; i++){
        for(int j=0; j<blockElement; j++){
            in[out[j]] = V.block[i].data[j];
        }

        for(int j=0; j<blockElement; j++){
            V.block[i].data[j] = in[j];
        }
    }

    delete []in;
    delete []out;
}

void JPEGDecoder::partition()
{
    /// YCbCr 4:2:0
    int blockWidthLuma, blockHeightLuma;
    int blockWidthChroma, blockHeightChroma;
    blockWidthLuma = Common::divCeil(width, blockSize);
    blockHeightLuma = Common::divCeil(height, blockSize);
    blockWidthChroma = Common::divCeil(width/2, blockSize);
    blockHeightChroma = Common::divCeil(height/2, blockSize);

    /// set data to image from block
    for(int m=0; m<blockHeightLuma; m++){ /// loop all luma blocks
        for(int n=0; n<blockWidthLuma; n++){
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    int ii = m*blockSize+i;
                    int ij = n*blockSize+j;
                    if(ii<height && ij<width){
                        image[ii*width+ij] = Y.block[m*blockWidthLuma+n].data[i*blockSize+j];
                    }
                }
            }
        }
    }
    for(int m=0; m<blockHeightChroma; m++){ /// loop all chroma blocks
        for(int n=0; n<blockWidthChroma; n++){
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    int ii = m*blockSize+i;
                    int ij = n*blockSize+j;
                    if(ii<height/2 && ij<width/2){
                        image[height*width+ii*width/2+ij] = U.block[m*blockWidthChroma+n].data[i*blockSize+j];
                    }
                }
            }
        }
    }
    for(int m=0; m<blockHeightChroma; m++){ /// loop all chroma blocks
        for(int n=0; n<blockWidthChroma; n++){
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    int ii = m*blockSize+i;
                    int ij = n*blockSize+j;
                    if(ii<height/2 && ij<width/2){
                        image[height*width*5/4+ii*width/2+ij] = V.block[m*blockWidthChroma+n].data[i*blockSize+j];
                    }
                }
            }
        }
    }
}

void JPEGDecoder::write()
{
    string out = name + "_decode.yuv";
    FILE *file;
    file = fopen(out.c_str(), "wb");
    int size = width*height*3/2; /// YCbCr 4:2:0
    fwrite(image, sizeof(char), size, file);
    fclose(file);
}

int main(int argc, char* argv[])
{
    /*JPEGDecoder* jd;
    for(int i=1; i<argc; i++){
        cout << argv[i] << endl;
        jd = new JPEGDecoder();
        jd->decode(argv[i]);
        delete jd;
    }
    return 0;*/
    JPEGDecoder* jd;
    jd = new JPEGDecoder();
    jd->decode("2_1024x768_30.bin");
    delete jd;
}
