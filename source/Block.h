#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include<iostream>
using namespace std;

class Block_8x8{
public:
    Block_8x8(){
        data = new int[64];
    }
    ~Block_8x8(){
        delete []data;
    }
    void print(){
        for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
                cout << data[i*8+j] << ' ';
            }
            cout << endl;
        }
    }
    int* data;
    int upper;
    int left;
};

class BlockSet{
public:
    ~BlockSet(){
        delete []block;
    };
    void initial(int t){
        total = t;
        block = new Block_8x8[total];
    }
    Block_8x8* block;
    int total;
};


#endif // BLOCK_H_INCLUDED
