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
    for(int i=0; i<height; i++){ /// loop 8x8 block
        for(int j=0; j<width; j++){
            image[i*width+j] = i*width+j;
        }
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
    int blockWidthLuma, blockHeightLuma;
    int blockWidthChroma, blockHeightChroma;
    blockWidthLuma = (width+blockSize-1)/blockSize;
    blockHeightLuma = (height+blockSize-1)/blockSize;
    blockWidthChroma = (width/2+blockSize-1)/blockSize;
    blockHeightChroma = (height/2+blockSize-1)/blockSize;
    blockTotal = blockWidthLuma*blockHeightLuma+2*blockWidthChroma*blockHeightChroma;

    /// padding
    unsigned char* Y;
    unsigned char* Cb;
    unsigned char* Cr;
    Y = new unsigned char[blockWidthLuma*blockHeightLuma*blockSize*blockSize];
    Cb = new unsigned char[blockWidthChroma*blockHeightChroma*blockSize*blockSize];
    Cr = new unsigned char[blockWidthChroma*blockHeightChroma*blockSize*blockSize];
    for(int i=0; i<blockHeightLuma*blockSize; i++){
        for(int j=0; j<blockWidthLuma*blockSize; j++){
            int ii = (i<height) ? i : (height-1);
            int ij = (j<width) ? j : (width-1);
            Y[i*blockWidthLuma*blockSize+j] = image[ii*width+ij];
        }
    }

    block = new int[blockTotal*blockSize*blockSize];
    /// TODO set data to block from image
    for(int m=0; m<blockHeightLuma; m++){ /// loop all luma blocks
        for(int n=0; n<blockWidthLuma; n++){
            int blockIndex = (m*blockWidthLuma+n)*blockSize*blockSize;
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    block[blockIndex+i*blockSize+j] = image[(m*blockSize+i)*width+(n*blockSize+j)];
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
    je.encode("image\1_1536x1024.yuv", 1536, 1024, 100);
}
