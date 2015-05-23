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
using namespace std;

union bitType{
    int num;
    char bit[4];
};

class JPEGDecoder{
public:
    JPEGDecoder();
    ~JPEGDecoder();

    void decode(const char* filename);

    void initial(const char* filename);
    void entropy();
    void rasterscan();
    void quantization();
    void transform();
    void partition();
    void write();
private:
    string name;
    int width;
    int height;
    int quality;
    vector<bool> bitstream; /// entropy coding

    static const int blockSize=8;
    int blockTotal;
    int blockLumaTotal;
    int blockChromaTotal;
    int* block; /// every 8x8 block

    unsigned char* image;

    /// sub function
    int divCeil(int dividend, int divisor);
    int divRound(int dividend, int divisor);
    int bitstreamDecode(int type);
    int bitstreamNumber(int length);
    void zigzag(int* in, int* out, const int count);
    void idct1d(double* in, double* out, const int count);
    void idct2d(double* in, double* out, const int count);
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
    delete []block;
    bitstream.clear();
}

void JPEGDecoder::decode(const char* filename)
{
    initial(filename);
    entropy();
    rasterscan();
    quantization();
    transform(); /// IDCT transform
    partition(); /// de-block, restore image
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
    union bitType b;
    b.num = 0;
    b.bit[0] = buffer[1];
    b.bit[1] = buffer[0];
    width = b.num;
    b.num = 0;
    b.bit[0] = buffer[3];
    b.bit[1] = buffer[2];
    height = b.num;
    b.num = 0;
    b.bit[0] = buffer[4];
    quality = b.num;

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
    blockWidthLuma = divCeil(width, blockSize);
    blockHeightLuma = divCeil(height, blockSize);
    blockWidthChroma = divCeil(width/2, blockSize);
    blockHeightChroma = divCeil(height/2, blockSize);
    blockLumaTotal = blockWidthLuma*blockHeightLuma;
    blockChromaTotal = 2*blockWidthChroma*blockHeightChroma;
    blockTotal = blockWidthLuma*blockHeightLuma+2*blockWidthChroma*blockHeightChroma;

    block = new int[blockTotal*blockSize*blockSize];
}

void JPEGDecoder::entropy()
{
    int blockElement = blockSize * blockSize;
    int* blockTemp = new int[blockElement];
    int* blockTempIter;
    int blockTempSize;

    int blockNow=0;
    int blockIndex;

    int index;
    int length;
    int num;
    int lastDC = 0;
    int runCount;
    /// luma
    while(blockNow < blockLumaTotal){
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

        blockIndex = blockNow * blockElement;
        for(int i=0; i<blockElement; i++){
            block[blockIndex+i] = blockTemp[i];
        }

        blockNow++;
    }
    /// chroma
    while(blockNow < blockTotal){
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

        blockIndex = blockNow * blockElement;
        for(int i=0; i<blockElement; i++){
            block[blockIndex+i] = blockTemp[i];
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
    for(int b=0; b<blockTotal; b++){
        int blockIndex = b*blockElement;
        for(int i=0; i<blockElement; i++){
            in[out[i]] = block[blockIndex+i];
        }

        for(int i=0; i<blockElement; i++){
            block[blockIndex+i] = in[i];
        }
    }
    delete []in;
    delete []out;
}

void JPEGDecoder::quantization()
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
            *blockIter = (*blockIter) * lumaTableQuality[i];
            blockIter++;
        }
    }
    for(int b=0; b<blockChromaTotal; b++){
        for(int i=0; i<blockSize*blockSize; i++){
            *blockIter = (*blockIter) * chromaTableQuality[i];
            blockIter++;
        }
    }
    delete []lumaTableQuality;
    delete []chromaTableQuality;
}

void JPEGDecoder::transform()
{
    /// IDCT transform to all blocks
    double* in = new double[blockSize*blockSize];
    double* out = new double[blockSize*blockSize];
    for(int b=0; b<blockTotal; b++){
        int blockIndex = b*blockSize*blockSize;
        for(int i=0; i<blockSize*blockSize; i++){
            in[i] = block[blockIndex+i];
        }
        idct2d(in, out, blockSize);
        for(int i=0; i<blockSize*blockSize; i++){
            block[blockIndex+i] = round(out[i]);
        }
    }
    delete []in;
    delete []out;
}

void JPEGDecoder::partition()
{
    int *blockIter;
    /// level shift 2^(Bits-1)
    blockIter = block;
    for(int i=0; i<blockTotal*blockSize*blockSize; i++){
        *blockIter = *blockIter+128;
        if(*blockIter < 0) *blockIter = 0;
        if(*blockIter > 255) *blockIter = 255;
        blockIter++;
    }

    /// YCbCr 4:2:0
    int blockWidthLuma, blockHeightLuma;
    int blockWidthChroma, blockHeightChroma;
    blockWidthLuma = divCeil(width, blockSize);
    blockHeightLuma = divCeil(height, blockSize);
    blockWidthChroma = divCeil(width/2, blockSize);
    blockHeightChroma = divCeil(height/2, blockSize);

    /// set data to image from block
    blockIter = block;
    for(int m=0; m<blockHeightLuma; m++){ /// loop all luma blocks
        for(int n=0; n<blockWidthLuma; n++){
            for(int i=0; i<blockSize; i++){ /// loop 8x8 block
                for(int j=0; j<blockSize; j++){
                    int ii = m*blockSize+i;
                    int ij = n*blockSize+j;
                    if(ii<height && ij<width){
                        image[ii*width+ij] = *blockIter;
                    }
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
                    if(ii<height/2 && ij<width/2){
                        image[height*width+ii*width/2+ij] = *blockIter;
                    }
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
                    if(ii<height/2 && ij<width/2){
                        image[height*width*5/4+ii*width/2+ij] = *blockIter;
                    }
                    blockIter++;
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

int JPEGDecoder::divCeil(int dividend, int divisor)
{
    /// dividend and divisor must be positive
    return ((dividend-1) / divisor) + 1;
}

int JPEGDecoder::divRound(int dividend, int divisor)
{
    /// divisor must be positive
    if(dividend < 0){
        return -1*(-1*dividend + (divisor/2))/divisor;
    }else{
        return (dividend + (divisor/2))/divisor;
    }
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

void JPEGDecoder::idct1d(double* in, double* out, const int count){
    double* temp = new double[count];
    for(int i=0; i<count; i++){
        if(i==0) temp[i] = in[i] * sqrt(1.0/count);
        else temp[i] = in[i] * sqrt(2.0/count);
    }
    for(int n=0; n<count; n++){
        double o = 0.0;
        for(int k=0; k<count; k++){
            o += temp[k]*cos(M_PI*(2*n+1)*k/(2*count));
        }
        out[n] = o;
    }
    delete []temp;
}

void JPEGDecoder::idct2d(double* in, double* out, const int count){
    double* in1d = new double[count];
    double* out1d = new double[count];
	double* temp = new double[count*count];

	/* transform rows */
	for(int j=0; j<count; j++){
		for(int i=0; i<count; i++){
			in1d[i] = in[j*count+i];
		}
		idct1d(in1d, out1d, count);
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
		idct1d(in1d, out1d, count);
		for(int i=0; i<count; i++){
            out[i*count+j] = out1d[i];
		}
	}

	delete []in1d;
	delete []out1d;
	delete []temp;
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
    //jd->decode("1_1536x1024_90.bin");
}
