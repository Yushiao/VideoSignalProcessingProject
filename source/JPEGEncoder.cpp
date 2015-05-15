/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * JPEG Encoder
 */
#include<iostream>
#include<fstream>
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
    void entropy();
private:
    int width;
    int height;
    int quality;
    unsigned char* image;

    int blockSize;
    int blockTotal;
    int* block; /// every 8x8 block

    vector<bool> bitstream; /// entropy coding
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
    #undef DEBUG
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
    width = 12;
    height = 10;
    quality = 100;
    int size = width*height*3/2; /// YCbCr 4:2:0
    image = new unsigned char[size];
    for(int i=0; i<size; i++){ /// loop 8x8 block
        image[i] = i%255;
    }

    #endif

    partition(); /// partition into 8x8 block
    transform(); /// DCT transform
    quantization();
    entropy();
}

void JPEGEncoder::partition()
{
    blockSize = 8;
    /// YCbCr 4:2:0
    int* blockIter;
    int blockWidthLuma, blockHeightLuma;
    int blockWidthChroma, blockHeightChroma;
    blockWidthLuma = (width+blockSize-1)/blockSize;
    blockHeightLuma = (height+blockSize-1)/blockSize;
    blockWidthChroma = (width/2+blockSize-1)/blockSize;
    blockHeightChroma = (height/2+blockSize-1)/blockSize;
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
                    ii = (ii<height) ? ii : (height-1); /// padding
                    ij = (ij<width) ? ij : (width-1);
                    *blockIter = image[ii*width+ij];
                    blockIter = blockIter+1;
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
                    ii = (ii<height/2) ? ii : (height/2-1); /// padding
                    ij = (ij<width/2) ? ij : (width/2-1);
                    *blockIter = image[height*width+ii*width/2+ij];
                    blockIter = blockIter+1;
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
                    ii = (ii<height/2) ? ii : (height/2-1); /// padding
                    ij = (ij<width/2) ? ij : (width/2-1);
                    *blockIter = image[height*width*5/4+ii*width/2+ij];
                    blockIter = blockIter+1;
                }
            }
        }
    }
}

void JPEGEncoder::transform()
{
    /// TODO DCT transform to block[]
}

void JPEGEncoder::quantization()
{
    /// TODO quantize all block[]
}

void JPEGEncoder::entropy()
{
    /// TODO encode block to bitstream
}


int main(int argc, char* argv[])
{
    JPEGEncoder je;
    je.encode("image\\1_1536x1024.yuv", 1536, 1024, 100);
}
