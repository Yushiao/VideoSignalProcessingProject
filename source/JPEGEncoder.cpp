/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * JPEG Encoder
 */
#include<iostream>
#include<fstream>
#include<cmath>
#include<vector>
#include<string>
#include<sstream>

#include"Block.h"
#include"Common.h"
#include"Intra.h"
#include"Transform.h"
#include"Quantization.h"
#include"Entropy.h"
using namespace std;

class JPEGEncoder{
public:
    JPEGEncoder();
    ~JPEGEncoder();

    void encode(const char* filename, int w, int h, int q);
private:
    string name;
    int width;
    int height;
    int quality;
    unsigned char* image;

    static const int blockSize=8;
    int blockTotal;

    // original and reconstructed block
    BlockSet Y;
    BlockSet U;
    BlockSet V;

    // best choice block after quantization
    BlockSet Yq;
    BlockSet Uq;
    BlockSet Vq;

    // quantization table
    Block_8x8 lumaQTable;
    Block_8x8 chromaQTable;

    vector<bool> bitstream; /// entropy coded bits

    /// sub function
    void initial(const char* filename, int w, int h, int q);
    void partition();

    void zigzag();
    void entropy();

    void write();
    void levelShift(Block_8x8& in, Block_8x8& out);
};

JPEGEncoder::JPEGEncoder()
{

}

JPEGEncoder::~JPEGEncoder()
{
    delete []image;
    bitstream.clear();
}

void JPEGEncoder::encode(const char* filename, int w, int h, int q)
{
    initial(filename, w, h, q);
    partition(); /// partition into 8x8 block
    Intra::intra_process( Y, Yq, quality);
    Intra::intra_process( U, Uq, quality);
    Intra::intra_process( V, Vq, quality);

    zigzag();
    entropy();
    cout << bitstream.size() << endl;
//    write();
}

void JPEGEncoder::initial(const char* filename, int w, int h, int q)
{
    /// set parameter
    name = filename;
    name = name.substr(name.rfind('\\')+1);
    name = name.substr(0, name.rfind('.'));
    width = w;
    height = h;
    quality = (q<1) ? 1 : /// qp between 1 and 51
              (q>51) ? 51 : q;
    /// read image
    FILE *file;
    file = fopen(filename, "rb");
    int size = width*height*3/2; /// YCbCr 4:2:0
    image = new unsigned char[size];
    fread(image, sizeof(char), size, file);
    fclose(file);

    Quantization::createTable(quality, lumaQTable, chromaQTable);
}

void JPEGEncoder::partition()
{
    /// YCbCr 4:2:0
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
    Yq.initial(blockWidthLuma*blockHeightLuma);
    Uq.initial(blockWidthChroma*blockHeightChroma);
    Vq.initial(blockWidthChroma*blockHeightChroma);

    /// set data to block from image
    for(int m=0; m<blockHeightLuma; m++){ /// loop all luma blocks
        for(int n=0; n<blockWidthLuma; n++){
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    int ii = m*blockSize+i;
                    int ij = n*blockSize+j;
                    Y.block[m*blockWidthLuma+n].data[i*blockSize+j] = (ii<height && ij<width) ? image[ii*width+ij] : 255;
                    Y.block[m*blockWidthLuma+n].upper = (m==0) ? -1 : (m-1)*blockWidthLuma+n;
                    Y.block[m*blockWidthLuma+n].left = (n==0) ? -1 : m*blockWidthLuma+n-1;
                    Y.block[m*blockWidthLuma+n].right = (n==blockWidthLuma-1) ? -1 : m*blockWidthLuma+n+1;
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
                    U.block[m*blockWidthChroma+n].data[i*blockSize+j] = (ii<height/2 && ij<width/2) ? image[height*width+ii*width/2+ij] : 255;
                    U.block[m*blockWidthChroma+n].upper = (m==0) ? -1 : (m-1)*blockWidthChroma+n;
                    U.block[m*blockWidthChroma+n].left = (n==0) ? -1 : m*blockWidthChroma+n-1;
                    U.block[m*blockWidthChroma+n].right = (n==blockWidthChroma-1) ? -1 : m*blockWidthChroma+n+1;
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
                    V.block[m*blockWidthChroma+n].data[i*blockSize+j] = (ii<height/2 && ij<width/2) ? image[height*width*5/4+ii*width/2+ij] : 255;
                    V.block[m*blockWidthChroma+n].upper = (m==0) ? -1 : (m-1)*blockWidthChroma+n;
                    V.block[m*blockWidthChroma+n].left = (n==0) ? -1 : m*blockWidthChroma+n-1;
                    V.block[m*blockWidthChroma+n].right = (n==blockWidthChroma-1) ? -1 : m*blockWidthChroma+n+1;
                }
            }
        }
    }
}

void JPEGEncoder::write()
{
    stringstream ss;
    ss << quality;
    string out = name + '_' + ss.str() + ".bin";

    /// fill end with ZERO
    int zeros = 8-bitstream.size()%8;
    for(int i=0; i<zeros; i++){
        bitstream.push_back(false);
    }
    int length = bitstream.size()/8;
    int header = 5; /// for width(2 char), height(2 char), quality(1 char)
    length = length + header;
    char* buffer = new char[length];

    buffer[0] = (width>>8) & 0xFF;
    buffer[1] = width & 0xFF;
    buffer[2] = (height>>8) & 0xFF;
    buffer[3] = height & 0xFF;
    buffer[4] = quality;

    /// every 8-bit store in one char, use or operation to set char
    for(int i=0; i<length-header; i++){
        char bit = 0;
        for(int j=0; j<8; j++){
            if(bitstream[i*8+j]==1){
                bit = bit | (1 << (8-1-j));
            }
        }
        buffer[header+i] = bit;
    }

    FILE *file;
    file = fopen(out.c_str(), "wb");
    fwrite(buffer, sizeof(char), length, file);
    fclose(file);
    delete []buffer;
}

void JPEGEncoder::levelShift(Block_8x8& in, Block_8x8& out)
{
    for(int i=0; i<64; i++){
        out.data[i] = in.data[i] - 128;
    }
}

void JPEGEncoder::zigzag()
{
    for(int i=0; i<Y.total; i++){
        Common::zigzag(Yq.block[i], Yq.block[i]);
    }
    for(int i=0; i<U.total; i++){
        Common::zigzag(Uq.block[i], Uq.block[i]);
    }
    for(int i=0; i<V.total; i++){
        Common::zigzag(Vq.block[i], Vq.block[i]);
    }
}

void JPEGEncoder::entropy()
{
    Entropy::entropyEncode(Yq, Uq, Vq, bitstream);
}

int main(int argc, char* argv[])
{
/*    JPEGEncoder* je;
    string command;
    string file;
    string temp;
    stringstream ss;
    int width, height, quality;

    if(argc==1 || argc>2){
        cout << "wrong input arguments" << endl;
        return 0;
    }
    cout << argv[1] << endl;
    fstream ftxt(argv[1], fstream::in);
    if(!ftxt.is_open()){
        cout << argv[1] << " open failed" << endl;
        return 0;
    }
    while(getline(ftxt, command)){
        cout << "Command: " << command << endl;
        size_t pos = 0;
        size_t len = command.find(' ');
        file = command.substr(pos, len);
        cout << "Filename: " << file << endl;

        pos = pos+len+1;
        len = command.find(' ', pos) - pos;
        temp = command.substr(pos, len);
        ss << temp;
        ss >> width;
        cout << "Width: " << width << endl;
        ss.str("");
        ss.clear();

        pos = pos+len+1;
        len = command.find(' ', pos) - pos;
        temp = command.substr(pos, len);
        ss << temp;
        ss >> height;
        cout << "Height: " << height << endl;
        ss.str("");
        ss.clear();

        pos = pos+len+1;
        len = command.find(' ', pos) - pos;
        temp = command.substr(pos, len);
        ss << temp;
        ss >> quality;
        cout << "Quality: " << quality << endl;
        ss.str("");
        ss.clear();

        je = new JPEGEncoder();
        je->encode(file.c_str(), width, height, quality);
        delete je;
    }
    ftxt.close();*/
    JPEGEncoder* je;
    je = new JPEGEncoder();
    //je->encode("test_32x32.yuv", 32, 32, 30);
    je->encode("2_1024x768.yuv", 1024, 768, 30);

    delete je;
}
