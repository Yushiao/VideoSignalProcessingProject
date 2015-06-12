#ifndef ENTROPY_H_INCLUDED
#define ENTROPY_H_INCLUDED

#include<iostream>
#include"Block.h"
#include"Common.h"
using namespace std;

class Entropy{
public:
    static void entropyEncode(BlockSet& imageYq, BlockSet& imageUq, BlockSet& imageVq, vector<bool>& bits);
    static void entropyEncodeLuma(Block_8x8& in, int* lastDC, vector<bool>& bits);
    static void entropyEncodeChroma(Block_8x8& in, int* lastDC, vector<bool>& bits);

    static void entropyDecode(vector<bool>& bits, BlockSet& imageYq, BlockSet& imageUq, BlockSet& imageVq);
    static void entropyDecodeLuma(vector<bool>& bits, BlockSet& imageYq);
    static void entropyDecodeChroma(vector<bool>& bits, BlockSet& imageUV);

    static void runLengthCoding(Block_8x8& in, int* run, int* number);
    static int findLastNonZero(Block_8x8& in);
    static int numberWidth(int number);
    static void pushBits(int length, int number, vector<bool>& bits);

    static int bitstreamDecode(vector<bool>& bits, int type);
    static int bitstreamNumber(vector<bool>& bits, int length);

    static const int count = 8;
    static const int size = 64;
};

void Entropy::entropyEncode(BlockSet& imageYq, BlockSet& imageUq, BlockSet& imageVq, vector<bool>& bits)
{
    int* last = new int;
    *last = 0;
    for(int i=0; i<imageYq.total; i++){
        entropyEncodeLuma(imageYq.block[i], last, bits);
    }
    for(int i=0; i<imageUq.total; i++){
        entropyEncodeChroma(imageUq.block[i], last, bits);
    }
    for(int i=0; i<imageVq.total; i++){
        entropyEncodeChroma(imageVq.block[i], last, bits);
    }
    delete last;

    /*Block_8x8 temp;
    temp.setnumber(0);
    temp.data[1]=1;
    cout << findLastNonZero(temp) << endl;*/
}

void Entropy::entropyEncodeLuma(Block_8x8& in, int* lastDC, vector<bool>& bits)
{
    static const int lumaDCCodeLengthTable[] = {2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9};
    static const int lumaDCCodeWordTable[] = {0, 2, 3, 4, 5, 6, 14, 30, 62, 126, 254, 510};
    static const int lumaACCodeLengthTable[] = {4, 2, 2, 3, 4, 5, 7, 8, 10, 16, 16, 0, 4, 5, 7, 9, 11, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 12, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 12, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 15, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int lumaACCodeWordTable[] = {10, 0, 1, 4, 11, 26, 120, 248, 1014, 65410, 65411, 0, 12, 27, 121, 502, 2038, 65412, 65413, 65414, 65415, 65416, 0, 28, 249, 1015, 4084, 65417, 65418, 65419, 65420, 65421, 65422, 0, 58, 503, 4085, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 0, 59, 1016, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 0, 122, 2039, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 0, 123, 4086, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 0, 250, 4087, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 0, 504, 32704, 65462, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 0, 505, 65470, 65471, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 0, 506, 65479, 65480, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 0, 1017, 65488, 65489, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 0, 1018, 65497, 65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 0, 2040, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 0, 65515, 65516, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 2041, 65525, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};

    int* run = new int[size+1];
    int* number = new int[size+1];
    int* runIter;
    int* numberIter;

    runLengthCoding(in, run, number);
    /// encode block to bitstream
    /// intra mode
    pushBits(3, in.mode, bits);

    runIter = run;
    numberIter = number;
    /// for DC value, use DPCM
    *numberIter = *numberIter-*lastDC;
    *lastDC = in.data[0];
    /// encode DC to bitstream
    int category = *numberIter;
    category = (category<0) ? -category : category;
    category = numberWidth(category);
    if(category>11) cout << "ERROR DC value too large" << endl;
    pushBits(lumaDCCodeLengthTable[category], lumaDCCodeWordTable[category], bits);
    pushBits(category, *numberIter, bits);
    runIter++;
    numberIter++;
    /// for AC value
    while(*runIter!=0 || *numberIter!=0){
        /// encode AC to bitstream
        category = *numberIter;
        category = (category<0) ? -category : category;
        category = numberWidth(category);
        if(category>=11) cout << "ERROR AC value too large" << endl;
        if(category==0 && (*runIter!=15)) cout << "ERROR run value" << endl;
        pushBits(lumaACCodeLengthTable[(*runIter)*11 + category], lumaACCodeWordTable[(*runIter)*11 + category], bits);
        pushBits(category, *numberIter, bits);
        runIter++;
        numberIter++;
    }
    /// EOB
    pushBits(lumaACCodeLengthTable[0], lumaACCodeWordTable[0], bits);

    delete []run;
    delete []number;
}

void Entropy::entropyEncodeChroma(Block_8x8& in, int* lastDC, vector<bool>& bits)
{
    static const int chromaDCCodeLengthTable[] = {2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    static const int chromaDCCodeWordTable[] = {0, 1, 2, 6, 14, 30, 62, 126, 254, 510, 1022, 2046};
    static const int chromaACCodeLengthTable[] = {2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 12, 0, 4, 6, 8, 9, 11, 12, 16, 16, 16, 16, 0, 5, 8, 10, 12, 15, 16, 16, 16, 16, 16, 0, 5, 8, 10, 12, 16, 16, 16, 16, 16, 16, 0, 6, 9, 16, 16, 16, 16, 16, 16, 16, 16, 0, 6, 10, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 7, 11, 16, 16, 16, 16, 16, 16, 16, 16, 0, 8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static const int chromaACCodeWordTable[] = {0, 1, 4, 10, 24, 25, 56, 120, 500, 1014, 4084, 0, 11, 57, 246, 501, 2038, 4085, 65416, 65417, 65418, 65419, 0, 26, 247, 1015, 4086, 32706, 65420, 65421, 65422, 65423, 65424, 0, 27, 248, 1016, 4087, 65425, 65426, 65427, 65428, 65429, 65430, 0, 58, 502, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 0, 59, 1017, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 0, 121, 2039, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 0, 122, 2040, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 0, 249, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 65470, 65471, 0, 503, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 65479, 65480, 0, 504, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 65488, 65489, 0, 505, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 0, 506, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 0, 2041, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 65515, 65516, 0, 16352, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 65525, 1018, 32707, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534};

    int* run = new int[size+1];
    int* number = new int[size+1];
    int* runIter;
    int* numberIter;

    runLengthCoding(in, run, number);
    /// encode block to bitstream
    /// intra mode
    pushBits(3, in.mode, bits);
    runIter = run;
    numberIter = number;
    /// for DC value, use DPCM
    *numberIter = *numberIter-*lastDC;
    *lastDC = in.data[0];
    /// encode DC to bitstream
    int category = *numberIter;
    category = (category<0) ? -category : category;
    category = numberWidth(category);
    if(category>11)
        cout << "ERROR DC value too large" << endl;
    pushBits(chromaDCCodeLengthTable[category], chromaDCCodeWordTable[category], bits);
    pushBits(category, *numberIter, bits);
    runIter++;
    numberIter++;
    /// for AC value
    while(*runIter!=0 || *numberIter!=0){
        /// encode AC to bitstream
        category = *numberIter;
        category = (category<0) ? -category : category;
        category = numberWidth(category);
        if(category>=11) cout << "ERROR AC value too large: " << *numberIter << endl;
        if(category==0 && (*runIter!=15)) cout << "ERROR run value" << endl;

        pushBits(chromaACCodeLengthTable[(*runIter)*11 + category], chromaACCodeWordTable[(*runIter)*11 + category], bits);
        pushBits(category, *numberIter, bits);
        runIter++;
        numberIter++;
    }
    /// EOB
    pushBits(chromaACCodeLengthTable[0], chromaACCodeWordTable[0], bits);

    delete []run;
    delete []number;
}

void Entropy::entropyDecode(vector<bool>& bits, BlockSet& imageYq, BlockSet& imageUq, BlockSet& imageVq)
{
    entropyDecodeLuma(bits, imageYq);
    entropyDecodeChroma(bits, imageUq);
    entropyDecodeChroma(bits, imageVq);
}

void Entropy::entropyDecodeLuma(vector<bool>& bits, BlockSet& imageYq)
{
    int* blockTemp = new int[size];
    int* blockTempIter;
    int blockTempSize;

    int blockNow=0;

    int index;
    int length;
    int num;
    int lastDC = 0;
    int runCount;
    /// Y
    while(blockNow < imageYq.total){
        /// intra mode
        int mode = 0;
        for(int i=0; i<3; i++){
            mode = mode << 1;
            mode = mode | bits.back();
            bits.pop_back();
        }
        imageYq.block[blockNow].mode = mode;

        blockTempIter = blockTemp;
        blockTempSize = 0;
        /// DC
        index = bitstreamDecode(bits, 0);
        length = index;
        num = bitstreamNumber(bits, length) + lastDC; /// DPCM
        lastDC = num;
        *blockTempIter = num;
        blockTempIter++;
        blockTempSize++;
        /// AC
        while(1){
            index = bitstreamDecode(bits, 2);
            if(index==0) break;
            runCount = index / 11;
            length = index % 11;
            num = bitstreamNumber(bits, length);
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
        for(int i=0; i<size-blockTempSize; i++){
            *blockTempIter = 0;
            blockTempIter++;
        }

        for(int i=0; i<size; i++){
            imageYq.block[blockNow].data[i] = blockTemp[i];
        }

        blockNow++;
    }
    delete []blockTemp;
}

void Entropy::entropyDecodeChroma(vector<bool>& bits, BlockSet& imageUV)
{
    int* blockTemp = new int[size];
    int* blockTempIter;
    int blockTempSize;

    int blockNow=0;

    int index;
    int length;
    int num;
    int lastDC = 0;
    int runCount;
    /// U or V
    blockNow = 0;
    while(blockNow < imageUV.total){
        /// intra mode
        int mode = 0;
        for(int i=0; i<3; i++){
            mode = mode << 1;
            mode = mode | bits.back();
            bits.pop_back();
        }
        imageUV.block[blockNow].mode = mode;

        blockTempIter = blockTemp;
        blockTempSize = 0;
        /// DC
        index = bitstreamDecode(bits, 1);
        length = index;
        num = bitstreamNumber(bits, length) + lastDC; /// DPCM
        lastDC = num;
        *blockTempIter = num;
        blockTempIter++;
        blockTempSize++;
        /// AC
        while(1){
            index = bitstreamDecode(bits, 3);
            if(index==0) break;
            runCount = index / 11;
            length = index % 11;
            num = bitstreamNumber(bits, length);
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
        for(int i=0; i<size-blockTempSize; i++){
            *blockTempIter = 0;
            blockTempIter++;
        }

        for(int i=0; i<size; i++){
            imageUV.block[blockNow].data[i] = blockTemp[i];
        }

        blockNow++;
    }
    delete []blockTemp;
}

void Entropy::runLengthCoding(Block_8x8& in, int* run, int* number)
{
    int* runIter = run;
    int* numberIter = number;
    int last = findLastNonZero(in);
    int runLength = 0;

    /// run length coding
    // DC value
    *runIter = 0;
    runIter++;
    *numberIter = in.data[0];
    numberIter++;
    // AC value
    for(int i=1; i<=last; i++){
        if(in.data[i]!=0){
            *runIter = runLength;
            runIter++;
            *numberIter = in.data[i];
            numberIter++;
            runLength=0;
            continue;
        }
        runLength++;
        /// 16 zeros (ZRL)
        if(runLength>=16){
            *runIter = 15;
            runIter++;
            *numberIter = 0;
            numberIter++;
            runLength = 0;
        }
    }
    /// end of block (EOB)
    *runIter = 0;
    *numberIter = 0;
}

int Entropy::findLastNonZero(Block_8x8& in)
{
    int loc = -1;
    for(int i=size-1; i>=0; i--){
        if(in.data[i]!=0){
            loc = i;
            break;
        }
    }
    return loc;
}

int Entropy::numberWidth(int number)
{
    int result = 0;
    while(number !=0){
        number = number >> 1;
        result++;
    }
    return result;
}

void Entropy::pushBits(int length, int number, vector<bool>& bits)
{
    if(length<0) cout << "ERROR! length is negative" << endl;
    int bit = 1 << length;
    bool b;
    if(number < 0) number = ~(-number);
    for(int i=0; i<length; i++){
        bit = bit >> 1;
        b = number&bit;
        bits.push_back(b);
    }
}

int Entropy::bitstreamDecode(vector<bool>& bits, int type)
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
        codeWord = codeWord | bits.back();
        codeWordLength++;
        bits.pop_back();
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

int Entropy::bitstreamNumber(vector<bool>& bits, int length)
{
    bool negative = !bits.back();
    int number = 0;
    for(int i=0; i<length; i++){
        number = number << 1;
        number = number | (negative^bits.back());
        bits.pop_back();
    }
    if(negative) number = -number;
    return number;
}

#endif // ENTROPY_H_INCLUDED
