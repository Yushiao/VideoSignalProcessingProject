/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * JPEG Encoder
 */
#include<iostream>
#include<fstream>
#include<vector>
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

    partition(); /// partition into 8x8 block
    transform(); /// DCT transform
    quantization();
    entropy();
}

void JPEGEncoder::partition()
{
    blockSize = 8;
    /// YCbCr 4:2:0
    int blockWidthLuma, blockheightLuma;
    int blockWidthChroma, blockheightChroma;
    blockWidthLuma = (width+blockSize-1)/blockSize;
    blockheightLuma = (height+blockSize-1)/blockSize;
    blockWidthChroma = (width/2+blockSize-1)/blockSize;
    blockheightChroma = (height/2+blockSize-1)/blockSize;
    blockTotal = blockWidthLuma*blockheightLuma+2*blockWidthChroma*blockheightChroma;

    block = new int[blockTotal*blockSize*blockSize];
    /// TODO set data to block from image
    for(int b=0; b<blockTotal; b++){
        for(int i=0; i<blockSize; i++){
            for(int j=0; j<blockSize; j++){
                block[b*blockSize*blockSize+i*blockSize+j] = 0;
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
