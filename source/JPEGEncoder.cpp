/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 */
#include<iostream>
using namespace std;

class JPEGEncoder{
public:
    void encode(const char image, int width, int height, int quality);
    
    void partition();
    void transform();
    void quantization();
    void entropy();
private:
    int width;
    int height;
    int quality;
    unsigned char 
};


int main(int argc, char* argv[]) 
{
    cout << "Hello world!" << endl;
}
