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
using namespace std;

#define PI 3.14159265358979

class JPEGEncoder{
public:
    JPEGEncoder();
    ~JPEGEncoder();

    void encode(const char* filename, int w, int h, int q);

    void initial(const char* filename, int w, int h, int q);
    void partition();
    void transform();
    void quantization();
    void zigzag();
    void entropy();

    void write();
private:
    string name;
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
    void dct1d(double* in, double* out, const int count);
    void dct2d(double* in, double* out, const int count);
    int numberOfBits(int number);
    void pushBits(int length, int number);
};

JPEGEncoder::JPEGEncoder()
{

}

JPEGEncoder::~JPEGEncoder()
{
    delete []image;
    delete []block;
    bitstream.clear();
}

void JPEGEncoder::encode(const char* filename, int w, int h, int q)
{
    initial(filename, w, h, q);
    partition(); /// partition into 8x8 block
    transform(); /// DCT transform
    quantization();
    zigzag();
    entropy();

    write();
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

    /*width = 8;
    height = 8;
    quality = 50;
    int size = width*height*3/2; /// YCbCr 4:2:0
    image = new unsigned char[size];
    for(int i=0; i<size; i++){
        image[i] = 128;
    }
    unsigned char test[] = {
      52, 55, 61, 66, 70, 61, 64, 73,
      63, 59, 55, 90, 109, 85, 69, 72,
      62, 59, 68, 113, 144, 104, 66, 73,
      63, 58, 71, 122, 154, 106, 70, 69,
      67, 61, 68, 104, 126, 88, 68, 70,
      79, 65, 60, 70, 77, 68, 58, 75,
      85, 71, 64, 59, 55, 61, 65, 83,
      87, 79, 69, 68, 65, 76, 78, 94
    };
    for(int i=0; i<64; i++){
        image[i] = test[i];
    }*/

    /// YCbCr 4:2:0
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
}

void JPEGEncoder::partition()
{
    int* blockIter = block;
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
    /// DCT transform to all blocks
    // parameter
/*	const double w0 = sqrt(1.0/blockSize);
	const double w1 = sqrt(2.0/blockSize);

    double* result = new double[blockSize*blockSize];
	double dct_matrix[blockSize][blockSize]={0}; //dct transform martix : dimention =(blockSize*blockSize)

	for(int i=0; i<blockSize; i++){
		for(int j=0; j<blockSize; j++){
			dct_matrix[i][j] = cos((2*j+1)*i*PI/blockSize/2);
		}
	}

//	for(int i=0; i<2*blockSize; i++){
//        printf("block[%d]: %d\n",i,block[i]);
//	}

	for(int i=0; i<blockTotal*blockSize*blockSize; i++){
		for(int j=0; j<blockSize; j++){
			//i!=8n ex:0,8,16,...
			if(i%blockSize){
				result[i] += block[i/blockSize+j]*w1*dct_matrix[i%blockSize][j];
			}
			//i==8n ex:0,8,16,...
			else{
				result[i] += block[i/blockSize+j]*w0*dct_matrix[i%blockSize][j];
			}
		}
	}

	for(int i=0; i<blockTotal*blockSize*blockSize; i++){
        block[i] = round(result[i]);
//        printf("result[%d]: %d\n",i,block[i]);
//        printf("result[%d]: %f\n",i,result[i]);
	}

    delete []result;*/

    double* in = new double[blockSize*blockSize];
    double* out = new double[blockSize*blockSize];
    for(int b=0; b<blockTotal; b++){
        int blockIndex = b*blockSize*blockSize;
        for(int i=0; i<blockSize*blockSize; i++){
            in[i] = block[blockIndex+i];
        }
        dct2d(in, out, blockSize);
        for(int i=0; i<blockSize*blockSize; i++){
            block[blockIndex+i] = round(out[i]);
        }
    }
    delete []in;
    delete []out;
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
    int* blockIter = block;
    for(int b=0; b<blockLumaTotal; b++){
        for(int i=0; i<blockSize*blockSize; i++){
            *blockIter = divRound(*blockIter, lumaTableQuality[i]);
            blockIter++;
        }
    }
    for(int b=0; b<blockChromaTotal; b++){
        for(int i=0; i<blockSize*blockSize; i++){
            *blockIter = divRound(*blockIter, chromaTableQuality[i]);
            blockIter++;
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
    static const int lumaDCCodeLengthTable[] = {2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9};
    static const int lumaDCCodeWordTable[] = {0, 2, 3, 4, 5, 6, 14, 30, 62, 126, 254, 510};
    static const int chromaDCCodeLengthTable[] = {2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    static const int chromaDCCodeWordTable[] = {0, 1, 2, 6, 14, 30, 62, 126, 254, 510, 1022, 2046};
    static const int lumaACCodeLengthTable[] = {4, 2, 2, 3, 4, 5, 7, 8, 10, 16, 16, 0, 4, 5, 7, 9, 11, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 12, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 15, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int lumaACCodeWordTable[] = {10, 0, 1, 4, 11, 26, 120, 248, 1014, 65410, 65411, 0, 12, 27, 121, 502, 2038, 65412, 65413, 65414, 65415, 65416, 0, 28, 249, 1015, 4084, 65417, 65418, 65419, 65420, 65421, 65422, 0, 58, 503, 4085, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 0, 59, 1016, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 0, 122, 2039, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 0, 123, 4086, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 0, 250, 4087, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 0, 504, 32704, 65462, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 0, 505, 65470, 65471, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 0, 506, 65479, 65480, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 0, 1017, 65488, 65489, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 0, 1018, 65497, 65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 0, 2040, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 0, 65515, 65516, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 2041, 65525, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};
    static const int chromaACCodeLengthTable[] = {2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 12, 0, 4, 6, 8, 9, 11, 12, 16, 16, 16, 16, 0, 5, 8, 10, 12, 15, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 16, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int chromaACCodeWordTable[] = {0, 1, 4, 10, 24, 25, 56, 120, 500, 1014, 4084, 0, 11, 57, 246, 501, 2038, 4085, 65416, 65417, 65418, 65419, 0, 26, 247, 1015, 4086, 32706, 65420, 65421, 65422, 65423, 65424, 0, 27, 248, 1016, 4087, 65425, 65426, 65427, 65428, 65429, 65430, 0, 58, 502, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 0, 59, 1017, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 0, 121, 2039, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 0, 122, 2040, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 0, 249, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 65470, 65471, 0, 503, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 65479, 65480, 0, 504, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 65488, 65489, 0, 505, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 0, 506, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 0, 2041, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 65515, 65516, 0, 16352, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 65525, 1018, 32707, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};

    int blockElement = blockSize * blockSize;
    int* run = new int[blockElement+1];
    int* number = new int[blockElement+1];
    int* runIter;
    int* numberIter;
    int blockIndex;
    int blockEnd;
    int runCount;
    for(int b=0; b<blockTotal; b++){
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
        /// encode block to bitstream
        runIter = run;
        numberIter = number;
        /// for DC value, use DPCM
        *numberIter = (b==0) ? *numberIter : *numberIter-block[blockIndex-blockElement];
        /// encode DC to bitstream
        int category = *numberIter;
        category = (category<0) ? -category : category;
        category = numberOfBits(category);
        if(category>11) cout << "ERROR DC value too large" << endl;
        if(b < blockLumaTotal){ /// luma
            pushBits(lumaDCCodeLengthTable[category], lumaDCCodeWordTable[category]);
        }else{ /// chroma
            pushBits(chromaDCCodeLengthTable[category], chromaDCCodeWordTable[category]);
        }
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

            if(b < blockLumaTotal){ /// luma
                pushBits(lumaACCodeLengthTable[(*runIter)*11 + category], lumaACCodeWordTable[(*runIter)*11 + category]);
            }else{ /// chroma
                pushBits(chromaACCodeLengthTable[(*runIter)*11 + category], chromaACCodeWordTable[(*runIter)*11 + category]);
            }
            pushBits(category, *numberIter);
            runIter++;
            numberIter++;
        }
        /// EOB
        if(b < blockLumaTotal){ /// luma
            pushBits(lumaACCodeLengthTable[0], lumaACCodeWordTable[0]);
        }else{ /// chroma
            pushBits(chromaACCodeLengthTable[0], chromaACCodeWordTable[0]);
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
    int header = 6; /// for width(2 char), height(2 char), quality(1 char) and zeros(1 char)
    length = length + header;
    char* buffer = new char[length];

    buffer[0] = (width>>8) & 0xFF;
    buffer[1] = width & 0xFF;
    buffer[2] = (height>>8) & 0xFF;
    buffer[3] = height & 0xFF;
    buffer[4] = quality;
    buffer[5] = zeros;

    /// every 8-bit store in one char, use or operation to set char
    for(int i=header; i<length; i++){
        char bit = 0;
        for(int j=0; j<8; j++){
            if(bitstream[i*8+j]==1){
                bit = bit | (1 << (8-1-j));
            }
        }
        buffer[i] = bit;
    }

    FILE *file;
    file = fopen(out.c_str(), "wb");
    fwrite(buffer, sizeof(char), length, file);
    fclose(file);
    delete []buffer;
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

void JPEGEncoder::dct1d(double* in, double* out, const int count)
{
	for(int u=0; u<count; u++){
		double z = 0.0;
		for(int x=0; x<count; x++){
			z += in[x] * cos(PI*u*(2*x+1)/(2*count));
		}
		if(u == 0) z *= sqrt(1.0/count);
		else z *= sqrt(2.0/count);
        out[u] = z;
	}
}

void JPEGEncoder::dct2d(double* in, double* out, const int count)
{
    double* in1d = new double[count];
    double* out1d = new double[count];
	double* temp = new double[count*count];

	/* transform rows */
	for(int j=0; j<count; j++){
		for(int i=0; i<count; i++){
			in1d[i] = in[j*count+i];
		}
		dct1d(in1d, out1d, count);
		for(int i=0; i<count; i++){
            temp[j*count+i] = out1d[i];
		}
	}

	/* transform columns */
	for(int j=0; j<count; j++)
	{
		for(int i=0; i<count; i++){
			in1d[i] = temp[i*count+j];
		}
		dct1d(in1d, out1d, count);
		for(int i=0; i<count; i++){
            out[i*count+j] = out1d[i];
		}
	}

	delete []in1d;
	delete []out1d;
	delete []temp;
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
    JPEGEncoder je;
    je.encode("image\\1_1536x1024.yuv", 1536, 1024, 90);
}
