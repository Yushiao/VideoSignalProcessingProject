/**
 * EE6650 Video Signal Processing
 * Term Project : Image Coding Contest
 * PSNR Calculator
 */
#include<iostream>
#include<fstream>
#include<cmath>
#include<string>
#include<sstream>
using namespace std;

class PSNRCalc{
public:
    PSNRCalc();
    ~PSNRCalc();

    double PSNRYUV(const char* filename0, const char* filename1, int w, int h);
    double MSE(unsigned char* image0, unsigned char* image1, int width, int height);
    double PSNR(double MSE);
private:
    int width;
    int height;
    unsigned char* image0;
    unsigned char* image1;

    double PSNRY;
    double PSNRU;
    double PSNRV;
};

PSNRCalc::PSNRCalc()
{

}

PSNRCalc::~PSNRCalc()
{
    delete []image0;
    delete []image1;
}

double PSNRCalc::PSNRYUV(const char* filename0, const char* filename1, int w, int h)
{
    /// set parameter
    width = w;
    height = h;

    /// read image
    FILE *file;
    file = fopen(filename0, "rb");
    if(!file){
        cout << filename0 << " open failed" << endl;
    }
    int size = width*height*3/2; /// YCbCr 4:2:0
    image0 = new unsigned char[size];
    fread(image0, sizeof(char), size, file);
    fclose(file);

    file = fopen(filename1, "rb");
    if(!file){
        cout << filename1 << " open failed" << endl;
    }
    image1 = new unsigned char[size];
    fread(image1, sizeof(char), size, file);
    fclose(file);

    int offset = 0;
    double MSEY = MSE(image0, image1, width, height);
    offset += width*height;
    double MSEU = MSE(image0+offset, image1+offset, width/2, height/2);
    offset += width/2*height/2;
    double MSEV = MSE(image0+offset, image1+offset, width/2, height/2);

    PSNRY = PSNR(MSEY);
    PSNRU = PSNR(MSEU);
    PSNRV = PSNR(MSEV);

    return (6*PSNRY+PSNRU+PSNRV)/8;
}

double PSNRCalc::MSE(unsigned char* image0, unsigned char* image1, int width, int height)
{
    double result = 0.0;
    unsigned char* iter0 = image0;
    unsigned char* iter1 = image1;
    int size = width*height;
    for(int i=0; i<size; i++){
        result += pow(*iter0 - *iter1,2);
        iter0++;
        iter1++;
    }
    return result/size;
}

double PSNRCalc::PSNR(double MSE)
{
    return 10*log10(255*255/MSE);
}

int main(int argc, char* argv[])
{
    PSNRCalc* psnr;
    string command;
    string file0;
    string file1;
    string temp;
    stringstream ss;
    int width, height;

    std::ofstream out("out_psnr.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

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
        file0 = command.substr(pos, len);
        cout << "File0: " << file0 << endl;

        pos = pos+len+1;
        len = command.find(' ', pos) - pos;
        file1 = command.substr(pos, len);
        cout << "File1: " << file1 << endl;

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

        psnr = new PSNRCalc();
        cout << "PSNR: " << psnr->PSNRYUV(file0.c_str(), file1.c_str(), width, height) << endl;
        delete psnr;
    }
    std::cout.rdbuf(coutbuf); //reset to standard output again
    ftxt.close();
    return 0;
}
