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
};

JPEGEncoder::~JPEGEncoder()
{

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
}

void JPEGEncoder::partition()
{

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
    cout << "Hello world!" << endl;
}
