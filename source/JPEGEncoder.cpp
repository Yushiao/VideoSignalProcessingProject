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
#include"Transform.h"
#include"Quantization.h"
#include"Intra.h"

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

    vector<bool> bitstream; /// entropy coding

    /// sub function
    void initial(const char* filename, int w, int h, int q);
    void partition();

    void zigzag();
    void entropy();

    void write();
    void levelShift(Block_8x8& in, Block_8x8& out);
    void zigzag(int* in, int* out, const int count);
    void zigzag(Block_8x8& in, Block_8x8& out);
    void entropyLuma(Block_8x8& in, int* lastDC);
    void entropyChroma(Block_8x8& in, int* lastDC);
    int numberOfBits(int number);
    void pushBits(int length, int number);
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
    Intra::intra_process( Y, Yq, blockSize, lumaQTable);
    cout << "Yq block[0]:\n";
    Yq.block[0].print();

//    for(int i=0; i<Y.total; i++){
//        levelShift(Y.block[i], Yq.block[i]);
//        Transform::transform(Yq.block[i], Yq.block[i]);
//        Quantization::quantization(Yq.block[i], Yq.block[i], lumaQTable);
//    }
//    for(int i=0; i<U.total; i++){
//        levelShift(U.block[i], Uq.block[i]);
//        Transform::transform(Uq.block[i], Uq.block[i]);
//        Quantization::quantization(Uq.block[i], Uq.block[i], chromaQTable);
//    }
//    for(int i=0; i<V.total; i++){
//        levelShift(V.block[i], Vq.block[i]);
//        Transform::transform(Vq.block[i], Vq.block[i]);
//        Quantization::quantization(Vq.block[i], Vq.block[i], chromaQTable);
//    }
//
//    zigzag();
//    entropy();
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
    quality = (q<1) ? 1 : /// quality between 1 and 100
              (q>100) ? 100 : q;
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
        zigzag(Yq.block[i], Yq.block[i]);
    }
    for(int i=0; i<U.total; i++){
        zigzag(Uq.block[i], Uq.block[i]);
    }
    for(int i=0; i<V.total; i++){
        zigzag(Vq.block[i], Vq.block[i]);
    }
}

void JPEGEncoder::zigzag(int* in, int* out, const int count)
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

void JPEGEncoder::zigzag(Block_8x8& in, Block_8x8& out)
{
    JPEGEncoder::zigzag(in.data, out.data, 8);
}

void JPEGEncoder::entropy()
{
    int* last = new int;
    *last = 0;
    for(int i=0; i<Y.total; i++){
        entropyLuma(Yq.block[i], last);
    }
    for(int i=0; i<U.total; i++){
        entropyChroma(Uq.block[i], last);
    }
    for(int i=0; i<V.total; i++){
        entropyChroma(Vq.block[i], last);
    }
}

void JPEGEncoder::entropyLuma(Block_8x8& in, int* lastDC)
{
    static const int lumaDCCodeLengthTable[] = {2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9};
    static const int lumaDCCodeWordTable[] = {0, 2, 3, 4, 5, 6, 14, 30, 62, 126, 254, 510};
    static const int lumaACCodeLengthTable[] = {4, 2, 2, 3, 4, 5, 7, 8, 10, 16, 16, 0, 4, 5, 7, 9, 11, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 12, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 15, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int lumaACCodeWordTable[] = {10, 0, 1, 4, 11, 26, 120, 248, 1014, 65410, 65411, 0, 12, 27, 121, 502, 2038, 65412, 65413, 65414, 65415, 65416, 0, 28, 249, 1015, 4084, 65417, 65418, 65419, 65420, 65421, 65422, 0, 58, 503, 4085, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 0, 59, 1016, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 0, 122, 2039, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 0, 123, 4086, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 0, 250, 4087, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 0, 504, 32704, 65462, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 0, 505, 65470, 65471, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 0, 506, 65479, 65480, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 0, 1017, 65488, 65489, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 0, 1018, 65497, 65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 0, 2040, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 0, 65515, 65516, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 2041, 65525, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};

    int blockElement = blockSize * blockSize;
    int* run = new int[blockElement+1];
    int* number = new int[blockElement+1];
    int* runIter;
    int* numberIter;
    int blockEnd = 0;
    int runCount = 0;

    runIter = run;
    numberIter = number;
    /// find the last element
    for(int i=blockElement-1; i>=0; i--){
        if(in.data[i]!=0){
            blockEnd = i;
            break;
        }
    }
    /// run length coding
    *runIter = 0;
    runIter++;
    *numberIter = in.data[0];
    numberIter++;
    for(int i=1; i<=blockEnd; i++){
        if(in.data[i]!=0){
            *runIter = runCount;
            runIter++;
            *numberIter = in.data[i];
            numberIter++;
            runCount=0;
            continue;
        }
        runCount++;
        /// 16 zeros (ZRL)
        if(runCount>=16){
            *runIter = 15;
            runIter++;
            *numberIter = 0;
            numberIter++;
            runCount = 0;
        }
    }
    /// end of block (EOB)
    *runIter = 0;
    *numberIter = 0;
    /// encode block to bitstream
    runIter = run;
    numberIter = number;
    /// for DC value, use DPCM
    *numberIter = *numberIter-*lastDC;
    *lastDC = in.data[0];
    /// encode DC to bitstream
    int category = *numberIter;
    category = (category<0) ? -category : category;
    category = numberOfBits(category);
    if(category>11) cout << "ERROR DC value too large" << endl;
    pushBits(lumaDCCodeLengthTable[category], lumaDCCodeWordTable[category]);
    pushBits(category, *numberIter);
    runIter++;
    numberIter++;
    /// for AC value
    while(*runIter!=0 || *numberIter!=0){
        /// encode AC to bitstream
        category = *numberIter;
        category = (category<0) ? -category : category;
        category = numberOfBits(category);
        if(category>=11) cout << "ERROR AC value too large" << endl;
        if(category==0 && (*runIter!=15)) cout << "ERROR run value" << endl;
        pushBits(lumaACCodeLengthTable[(*runIter)*11 + category], lumaACCodeWordTable[(*runIter)*11 + category]);
        pushBits(category, *numberIter);
        runIter++;
        numberIter++;
    }
    /// EOB
    pushBits(lumaACCodeLengthTable[0], lumaACCodeWordTable[0]);

    delete []run;
    delete []number;
}

void JPEGEncoder::entropyChroma(Block_8x8& in, int* lastDC)
{
    static const int chromaDCCodeLengthTable[] = {2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    static const int chromaDCCodeWordTable[] = {0, 1, 2, 6, 14, 30, 62, 126, 254, 510, 1022, 2046};
    static const int chromaACCodeLengthTable[] = {2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 12, 0, 4, 6, 8, 9, 11, 12, 16, 16, 16, 16, 0, 5, 8, 10, 12, 15, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 16, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int chromaACCodeWordTable[] = {0, 1, 4, 10, 24, 25, 56, 120, 500, 1014, 4084, 0, 11, 57, 246, 501, 2038, 4085, 65416, 65417, 65418, 65419, 0, 26, 247, 1015, 4086, 32706, 65420, 65421, 65422, 65423, 65424, 0, 27, 248, 1016, 4087, 65425, 65426, 65427, 65428, 65429, 65430, 0, 58, 502, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 0, 59, 1017, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 0, 121, 2039, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 0, 122, 2040, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 0, 249, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 65470, 65471, 0, 503, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 65479, 65480, 0, 504, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 65488, 65489, 0, 505, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 0, 506, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 0, 2041, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 65515, 65516, 0, 16352, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 65525, 1018, 32707, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};

    int blockElement = blockSize * blockSize;
    int* run = new int[blockElement+1];
    int* number = new int[blockElement+1];
    int* runIter;
    int* numberIter;
    int blockEnd;
    int runCount;
    runIter = run;
    numberIter = number;
    blockEnd = 0;
    runCount = 0;
    /// find the last element
    for(int i=blockElement-1; i>=0; i--){
        if(in.data[i]!=0){
            blockEnd = i;
            break;
        }
    }
    /// run length coding
    *runIter = 0;
    runIter++;
    *numberIter = in.data[0];
    numberIter++;
    for(int i=1; i<=blockEnd; i++){
        if(in.data[i]!=0){
            *runIter = runCount;
            runIter++;
            *numberIter = in.data[i];
            numberIter++;
            runCount=0;
            continue;
        }
        runCount++;
        /// 16 zeros (ZRL)
        if(runCount>=16){
            *runIter = 15;
            runIter++;
            *numberIter = 0;
            numberIter++;
            runCount = 0;
        }
    }
    /// end of block (EOB)
    *runIter = 0;
    *numberIter = 0;
    /// encode block to bitstream
    runIter = run;
    numberIter = number;
    /// for DC value, use DPCM
    *numberIter = *numberIter-*lastDC;
    *lastDC = in.data[0];
    /// encode DC to bitstream
    int category = *numberIter;
    category = (category<0) ? -category : category;
    category = numberOfBits(category);
    if(category>11) cout << "ERROR DC value too large" << endl;
    pushBits(chromaDCCodeLengthTable[category], chromaDCCodeWordTable[category]);
    pushBits(category, *numberIter);
    runIter++;
    numberIter++;
    /// for AC value
    while(*runIter!=0 || *numberIter!=0){
        /// encode AC to bitstream
        category = *numberIter;
        category = (category<0) ? -category : category;
        category = numberOfBits(category);
        if(category>=11) cout << "ERROR AC value too large" << endl;
        if(category==0 && (*runIter!=15)) cout << "ERROR run value" << endl;

        pushBits(chromaACCodeLengthTable[(*runIter)*11 + category], chromaACCodeWordTable[(*runIter)*11 + category]);
        pushBits(category, *numberIter);
        runIter++;
        numberIter++;
    }
    /// EOB
    pushBits(chromaACCodeLengthTable[0], chromaACCodeWordTable[0]);

    delete []run;
    delete []number;
}

int JPEGEncoder::numberOfBits(int number)
{
    int result = 0;
    while(number !=0){
        number = number >> 1;
        result++;
    }
    return result;
}

void JPEGEncoder::pushBits(int length, int number)
{
    if(length<0) cout << "ERROR! length is negative" << endl;
    int bit = 1 << length;
    bool b;
    if(number < 0) number = ~(-number);
    for(int i=0; i<length; i++){
        bit = bit >> 1;
        b = number&bit;
        bitstream.push_back(b);
    }
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
    je->encode("test_32x32.yuv", 32, 32, 90);
    delete je;
}
