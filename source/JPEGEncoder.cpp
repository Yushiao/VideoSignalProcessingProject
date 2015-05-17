/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * JPEG Encoder
 */
#include<iostream>
#include<fstream>
#include<cmath>
#include<vector>
using namespace std;

#define DEBUG

class JPEGEncoder{
public:
    JPEGEncoder();
    ~JPEGEncoder();

    void encode(const char* filename, int w, int h, int q);

    void partition();
    void transform();
    void quantization();
    void zigzag();
    void entropy();
private:
    int width;
    int height;
    int quality;
    unsigned char* image;

    static const int blockSize=8;
    int blockTotal;
    int blockLumaTotal;
    int blockChromaTotal;
    int* block; /// every 8x8 block

    vector<bool> bitstream; /// entropy coding

    /// sub function
    int divCeil(int dividend, int divisor);
    int divRound(int dividend, int divisor);
};

JPEGEncoder::JPEGEncoder()
{

}

JPEGEncoder::~JPEGEncoder()
{
    delete []image;
    delete []block;
}

void JPEGEncoder::encode(const char* filename, int w, int h, int q)
{
    //#undef DEBUG
    #ifndef DEBUG
    /// set parameter
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
    #else
    width = 8;
    height = 8;
    quality = 90;
    int size = width*height*3/2; /// YCbCr 4:2:0
    image = new unsigned char[size];
    for(int i=0; i<size; i++){ /// loop 8x8 block
        image[i] = 128;
    }
    image[0]=57+128;
    image[19]=3+128;
    image[24]=2+128;
    image[58]=255;
    #endif

    partition(); /// partition into 8x8 block
    transform(); /// DCT transform
    quantization();
    zigzag();
    entropy();
}

void JPEGEncoder::partition()
{
    /// YCbCr 4:2:0
    int* blockIter;
    int blockWidthLuma, blockHeightLuma;
    int blockWidthChroma, blockHeightChroma;
    blockWidthLuma = divCeil(width, blockSize);
    blockHeightLuma = divCeil(height, blockSize);
    blockWidthChroma = divCeil(width/2, blockSize);
    blockHeightChroma = divCeil(height/2, blockSize);
    blockLumaTotal = blockWidthLuma*blockHeightLuma;
    blockChromaTotal = 2*blockWidthChroma*blockHeightChroma;
    blockTotal = blockWidthLuma*blockHeightLuma+2*blockWidthChroma*blockHeightChroma;

    block = new int[blockTotal*blockSize*blockSize];
    blockIter = block;
    /// set data to block from image
    for(int m=0; m<blockHeightLuma; m++){ /// loop all luma blocks
        for(int n=0; n<blockWidthLuma; n++){
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    int ii = m*blockSize+i;
                    int ij = n*blockSize+j;
                    *blockIter = (ii<height && ij<width) ? image[ii*width+ij] : 255;
                    blockIter++;
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
                    *blockIter = (ii<height/2 && ij<width/2) ? image[height*width+ii*width/2+ij] : 255;
                    blockIter++;
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
                    *blockIter = (ii<height/2 && ij<width/2) ? image[height*width*5/4+ii*width/2+ij] : 255;
                    blockIter++;
                }
            }
        }
    }
    /// level shift 2^(Bits-1)
    blockIter = block;
    for(int i=0; i<blockTotal*blockSize*blockSize; i++){
        *blockIter = *blockIter-128;
        blockIter++;
    }
}

void JPEGEncoder::transform()
{
    /// TODO DCT transform to all blocks
    double* result = new double[blockSize*blockSize];
    for(int b=0; b<blockTotal; b++){

    }

    delete []result;
}

void JPEGEncoder::quantization()
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
    int* lumaTableQuality = new int[blockSize*blockSize];
    int* chromaTableQuality = new int[blockSize*blockSize];
    int q = (quality<50) ? (5000/quality) : (200-2*quality);
    for(int i=0; i<blockSize*blockSize; i++){
        int lumaTemp = divRound(lumaTable[i]*q, 100);
        int chromaTemp = divRound(chromaTable[i]*q, 100);
        if(lumaTemp<=0) lumaTemp=1;
        if(chromaTemp<=0) chromaTemp=1;
        lumaTableQuality[i] = lumaTemp;
        chromaTableQuality[i] = chromaTemp;
    }

    /// quantize all blocks
    for(int b=0; b<blockLumaTotal; b++){
        int blockIndex = b*blockSize*blockSize;
        for(int i=0; i<blockSize*blockSize; i++){
            block[blockIndex+i] = divRound(block[blockIndex+i], lumaTableQuality[i]);
        }
    }
    for(int b=0; b<blockChromaTotal; b++){
        int blockIndex = blockLumaTotal + b*blockSize*blockSize;
        for(int i=0; i<blockSize*blockSize; i++){
            block[blockIndex+i] = divRound(block[blockIndex+i], chromaTableQuality[i]);
        }
    }
    delete []lumaTableQuality;
    delete []chromaTableQuality;
}

void JPEGEncoder::zigzag()
{
    /// reorder block to zigzag form
    int* z = new int[blockSize*blockSize];
    int* ziter;
    for(int b=0; b<blockTotal; b++){
        int blockIndex = b*blockSize*blockSize;
        ziter = z;
        for(int s=0; s<2*blockSize-1; s++){
            if(s<blockSize){ /// upper-left block
                if(s%2==0){
                    for(int i=s; i>=0; i--){
                        *ziter = block[blockIndex+i*blockSize+s-i];
                        ziter++;
                    }
                }else{
                    for(int i=0; i<=s; i++){
                        *ziter = block[blockIndex+i*blockSize+s-i];
                        ziter++;
                    }
                }
            }else{ /// lower-right block
                if(s%2==0){
                    for(int i=blockSize-1; s-i<blockSize; i--){
                        *ziter = block[blockIndex+i*blockSize+s-i];
                        ziter++;
                    }
                }else{
                    for(int i=s-blockSize+1; i<blockSize; i++){
                        *ziter = block[blockIndex+i*blockSize+s-i];
                        ziter++;
                    }
                }
            }
        }
        for(int i=0; i<blockSize*blockSize; i++){
            block[blockIndex+i] = z[i];
        }
    }

    delete []z;
}

void JPEGEncoder::entropy()
{
    int blockElement = blockSize * blockSize;
    int* run = new int[blockElement];
    int* number = new int[blockElement];
    int* runIter;
    int* numberIter;
    int blockIndex;
    int blockEnd;
    int runCount;
    for(int b=0; b<1; b++){ //\fixme for test
        runIter = run;
        numberIter = number;
        blockIndex = b*blockElement;
        blockEnd = 0;
        runCount = 0;
        /// find the last element
        for(int i=blockElement-1; i>=0; i--){
            if(block[blockIndex+i]!=0){
                blockEnd = i;
                break;
            }
        }
        /// run length coding
        *runIter = 0;
        runIter++;
        *numberIter = block[blockIndex];
        numberIter++;
        for(int i=1; i<=blockEnd; i++){
            if(block[blockIndex+i]!=0){
                *runIter = runCount;
                runIter++;
                *numberIter = block[blockIndex+i];
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
        runIter = run;
        numberIter = number;
        /// encode block to bitstream
        /// for DC value, use DPCM
        *numberIter = (b==0) ? *numberIter : *numberIter-block[blockIndex-blockElement];
        /// TODO encode DC to bitstream

        runIter++;
        numberIter++;
        /// for AC value
        while(*runIter!=0 || *numberIter!=0){
            /// TODO encode AC to bitstream

            runIter++;
            numberIter++;
        }
    }
    cout << endl;
}

int JPEGEncoder::divCeil(int dividend, int divisor)
{
    /// dividend and divisor must be positive
    return ((dividend-1) / divisor) + 1;
}

int JPEGEncoder::divRound(int dividend, int divisor)
{
    /// divisor must be positive
    if(dividend < 0){
        return -1*(-1*dividend + (divisor/2))/divisor;
    }else{
        return (dividend + (divisor/2))/divisor;
    }
}

int main(int argc, char* argv[])
{
    JPEGEncoder je;
    je.encode("image\\1_1536x1024.yuv", 1536, 1024, 90);
}
