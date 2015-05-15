/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * JPEG Encoder
 */
#include<iostream>
#include<fstream>
using namespace std;

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
    int** block; /// every 8x8 block
    int blockTotal;

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
    /// set parameter
    width = w;
    height = h;
    quality = (q<1) ? 1 :    /// quality between 1 and 100
              (q>100) ? 100 : q;
    /// read image
    FILE *file;
    file = fopen(filename, "rb");
    int size = width*height*3/2;    /// YCbCr 4:2:0
    image = new unsigned char[size];
    fread(image, sizeof(char), size, file);
    fclose(file);

    /// partition 8x8 block
    partition();

}

void JPEGEncoder::partition()
{
    blockSize = 8;
    int blockWidthLuma, blockheightLuma;
    int blockWidthChroma, blockheightChroma;
    blockWidthLuma = (width+blockSize-1)/blockSize;
    blockheightLuma = (height/2+blockSize-1)/blockSize;
    blockWidthChroma = (width+blockSize-1)/blockSize;
    blockheightChroma = (height/2+blockSize-1)/blockSize;
    blockTotal = blockWidthLuma*blockheightLuma+2*blockWidthChroma*blockheightChroma;

    block = new int*[blockTotal];
    for(int i=0; i<blockTotal; i++){
        block[i] = new int[blockSize*blockSize];
    }
    for(int b=0; b<blockTotal; b++){
        for(int i=0; i<blockSize; i++){
            for(int j=0; j<blockSize; j++){
                block[b][i*blockSize+j] = 0;
            }
        }
    }
}

void JPEGEncoder::transform()
{

}

void JPEGEncoder::quantization()
{

}

void JPEGEncoder::entropy()
{

}


int main(int argc, char* argv[])
{
    JPEGEncoder je;
    je.encode("image\1_1536x1024.yuv", 1536, 1024, 100);
}
