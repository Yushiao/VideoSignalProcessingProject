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

#include"Block.h"
#include"Common.h"
#include"Transform.h"
#include"Quantization.h"

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
    entropy();
    rasterscan();
    quantization();
    transform(); /// IDCT transform
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
}

void JPEGDecoder::entropy()
{
    int blockElement = blockSize * blockSize;
    int* blockTemp = new int[blockElement];
    int* blockTempIter;
    int blockTempSize;

    int blockNow=0;

    int index;
    int length;
    int num;
    int lastDC = 0;
    int runCount;
    /// Y
    while(blockNow < Y.total){
        blockTempIter = blockTemp;
        blockTempSize = 0;
        /// DC
        index = bitstreamDecode(0);
        length = index;
        num = bitstreamNumber(length) + lastDC; /// DPCM
        lastDC = num;
        *blockTempIter = num;
        blockTempIter++;
        blockTempSize++;
        /// AC
        while(1){
            index = bitstreamDecode(2);
            if(index==0) break;
            runCount = index / 11;
            length = index % 11;
            num = bitstreamNumber(length);
            /// fill runCount ZERO
            for(int i=0; i<runCount; i++){
                *blockTempIter = 0;
                blockTempIter++;
                blockTempSize++;
            }
            *blockTempIter = num;
            blockTempIter++;
            blockTempSize++;
        }
        /// fill ZERO behind EOB
        for(int i=0; i<blockElement-blockTempSize; i++){
            *blockTempIter = 0;
            blockTempIter++;
        }

        for(int i=0; i<blockElement; i++){
            Y.block[blockNow].data[i] = blockTemp[i];
        }

        blockNow++;
    }
    /// U
    blockNow = 0;
    while(blockNow < U.total){
        blockTempIter = blockTemp;
        blockTempSize = 0;
        /// DC
        index = bitstreamDecode(1);
        length = index;
        num = bitstreamNumber(length) + lastDC; /// DPCM
        lastDC = num;
        *blockTempIter = num;
        blockTempIter++;
        blockTempSize++;
        /// AC
        while(1){
            index = bitstreamDecode(3);
            if(index==0) break;
            runCount = index / 11;
            length = index % 11;
            num = bitstreamNumber(length);
            /// fill runCount ZERO
            for(int i=0; i<runCount; i++){
                *blockTempIter = 0;
                blockTempIter++;
                blockTempSize++;
            }
            *blockTempIter = num;
            blockTempIter++;
            blockTempSize++;
        }
        /// fill ZERO behind EOB
        for(int i=0; i<blockElement-blockTempSize; i++){
            *blockTempIter = 0;
            blockTempIter++;
        }

        for(int i=0; i<blockElement; i++){
            U.block[blockNow].data[i] = blockTemp[i];
        }

        blockNow++;
    }
    /// V
    blockNow = 0;
    while(blockNow < V.total){
        blockTempIter = blockTemp;
        blockTempSize = 0;
        /// DC
        index = bitstreamDecode(1);
        length = index;
        num = bitstreamNumber(length) + lastDC; /// DPCM
        lastDC = num;
        *blockTempIter = num;
        blockTempIter++;
        blockTempSize++;
        /// AC
        while(1){
            index = bitstreamDecode(3);
            if(index==0) break;
            runCount = index / 11;
            length = index % 11;
            num = bitstreamNumber(length);
            /// fill runCount ZERO
            for(int i=0; i<runCount; i++){
                *blockTempIter = 0;
                blockTempIter++;
                blockTempSize++;
            }
            *blockTempIter = num;
            blockTempIter++;
            blockTempSize++;
        }
        /// fill ZERO behind EOB
        for(int i=0; i<blockElement-blockTempSize; i++){
            *blockTempIter = 0;
            blockTempIter++;
        }

        for(int i=0; i<blockElement; i++){
            V.block[blockNow].data[i] = blockTemp[i];
        }

        blockNow++;
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
    zigzag(in, out, blockSize);

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

void JPEGDecoder::quantization()
{
    for(int i=0; i<Y.total; i++){
        Quantization::iquantization(Y.block[i], Y.block[i], lumaQTable);
    }
    for(int i=0; i<U.total; i++){
        Quantization::iquantization(U.block[i], U.block[i], chromaQTable);
    }
    for(int i=0; i<V.total; i++){
        Quantization::iquantization(V.block[i], V.block[i], chromaQTable);
    }
}

void JPEGDecoder::transform()
{
    /// IDCT transform to all blocks
    for(int i=0; i<Y.total; i++){
        Transform::itransform(Y.block[i], Y.block[i]);
    }
    for(int i=0; i<U.total; i++){
        Transform::itransform(U.block[i], U.block[i]);
    }
    for(int i=0; i<V.total; i++){
        Transform::itransform(V.block[i], V.block[i]);
    }
}

void JPEGDecoder::partition()
{
    /// level shift 2^(Bits-1)
    for(int i=0; i<Y.total; i++){
        levelShiftBack(Y.block[i], Y.block[i]);
    }
    for(int i=0; i<U.total; i++){
        levelShiftBack(U.block[i], U.block[i]);
    }
    for(int i=0; i<V.total; i++){
        levelShiftBack(V.block[i], V.block[i]);
    }

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

int JPEGDecoder::bitstreamDecode(int type)
{
    /// constant
    static const int lumaDCCodeLengthTable[] = {2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9};
    static const int lumaDCCodeWordTable[] = {0, 2, 3, 4, 5, 6, 14, 30, 62, 126, 254, 510};
    static const int chromaDCCodeLengthTable[] = {2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    static const int chromaDCCodeWordTable[] = {0, 1, 2, 6, 14, 30, 62, 126, 254, 510, 1022, 2046};
    static const int lumaACCodeLengthTable[] = {4, 2, 2, 3, 4, 5, 7, 8, 10, 16, 16, 0, 4, 5, 7, 9, 11, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 12, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 15, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int lumaACCodeWordTable[] = {10, 0, 1, 4, 11, 26, 120, 248, 1014, 65410, 65411, 0, 12, 27, 121, 502, 2038, 65412, 65413, 65414, 65415, 65416, 0, 28, 249, 1015, 4084, 65417, 65418, 65419, 65420, 65421, 65422, 0, 58, 503, 4085, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 0, 59, 1016, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 0, 122, 2039, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 0, 123, 4086, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 0, 250, 4087, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 0, 504, 32704, 65462, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 0, 505, 65470, 65471, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 0, 506, 65479, 65480, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 0, 1017, 65488, 65489, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 0, 1018, 65497, 65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 0, 2040, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 0, 65515, 65516, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 2041, 65525, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};
    static const int chromaACCodeLengthTable[] = {2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 12, 0, 4, 6, 8, 9, 11, 12, 16, 16, 16, 16, 0, 5, 8, 10, 12, 15, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 16, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int chromaACCodeWordTable[] = {0, 1, 4, 10, 24, 25, 56, 120, 500, 1014, 4084, 0, 11, 57, 246, 501, 2038, 4085, 65416, 65417, 65418, 65419, 0, 26, 247, 1015, 4086, 32706, 65420, 65421, 65422, 65423, 65424, 0, 27, 248, 1016, 4087, 65425, 65426, 65427, 65428, 65429, 65430, 0, 58, 502, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 0, 59, 1017, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 0, 121, 2039, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 0, 122, 2040, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 0, 249, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 65470, 65471, 0, 503, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 65479, 65480, 0, 504, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 65488, 65489, 0, 505, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 0, 506, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 0, 2041, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 65515, 65516, 0, 16352, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 65525, 1018, 32707, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};
    static const int DCTableLength = 12;
    static const int ACTableLength = 176;

    /// type : 0: luma DC, 1: chroma DC, 2: luma AC, 3: chroma AC
    int result = -1;
    int codeWord = 0;
    int codeWordLength = 0;
    while(result==-1){
        /// update code word and code word length, then pop the back element
        codeWord = codeWord << 1;
        codeWord = codeWord | bitstream.back();
        codeWordLength++;
        bitstream.pop_back();
        if(codeWordLength<2) continue;
        if(codeWordLength>16) cout << "ERROR! can't decode" << endl;
        switch(type){
        case 0:
            for(int i=0; i<DCTableLength; i++){
                if(lumaDCCodeLengthTable[i]==codeWordLength && lumaDCCodeWordTable[i]==codeWord){
                    result = i;
                    break;
                }
            }
            break;
        case 1:
            for(int i=0; i<DCTableLength; i++){
                if(chromaDCCodeLengthTable[i]==codeWordLength && chromaDCCodeWordTable[i]==codeWord){
                    result = i;
                    break;
                }
            }
            break;
        case 2:
            for(int i=0; i<ACTableLength; i++){
                if(lumaACCodeLengthTable[i]==codeWordLength && lumaACCodeWordTable[i]==codeWord){
                    result = i;
                    break;
                }
            }
            break;
        case 3:
            for(int i=0; i<ACTableLength; i++){
                if(chromaACCodeLengthTable[i]==codeWordLength && chromaACCodeWordTable[i]==codeWord){
                    result = i;
                    break;
                }
            }
            break;
        }
    }
    return result;
}

int JPEGDecoder::bitstreamNumber(int length)
{
    bool negative = !bitstream.back();
    int number = 0;
    for(int i=0; i<length; i++){
        number = number << 1;
        number = number | (negative^bitstream.back());
        bitstream.pop_back();
    }
    if(negative) number = -number;
    return number;
}

void JPEGDecoder::zigzag(int* in, int* out, const int count)
{
    int* iter = out;
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
}

void JPEGDecoder::levelShiftBack(Block_8x8& in, Block_8x8& out)
{
    for(int i=0; i<64; i++){
        out.data[i] = in.data[i] + 128;
        if(out.data[i]<0) out.data[i] = 0;
        if(out.data[i]>255) out.data[i] = 255;
    }
}

int main(int argc, char* argv[])
{
    JPEGDecoder* jd;
    for(int i=1; i<argc; i++){
        cout << argv[i] << endl;
        jd = new JPEGDecoder();
        jd->decode(argv[i]);
        delete jd;
    }
    return 0;
    /*JPEGDecoder* jd;
    jd = new JPEGDecoder();
    jd->decode("5_1000x1504_50.bin");
    delete jd;*/
}
