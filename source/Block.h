#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include<iostream>
using namespace std;

class Block_8x8{
public:
    Block_8x8();
    Block_8x8(const Block_8x8& b);
    ~Block_8x8(){
        delete []data;
    }
    void print();
    void setnumber(int num);
    void setneighbor(int u, int l, int r);
    void setneighbor(const Block_8x8& b);
    Block_8x8 &operator=(const Block_8x8& b); // assignment
    Block_8x8 &operator+=(const Block_8x8& b); // Block += b
    Block_8x8 &operator-=(const Block_8x8& b); // Block -= b
    Block_8x8 operator+(const Block_8x8& b); // Block + b
    Block_8x8 operator-(const Block_8x8& b); // Block - b

    static const int size = 64;
    int* data;
    int upper;
    int left;
    int right;
    int mode;
};

Block_8x8::Block_8x8()
{
    data = new int[size];
}

Block_8x8::Block_8x8(const Block_8x8& b)
{
    data = new int[size];
    for(int i=0; i<size; i++){
        data[i] = b.data[i];
    }
}

Block_8x8 &Block_8x8::operator=(const Block_8x8& b)
{
    for(int i=0; i<size; i++){
        data[i]=b.data[i];
    }
    return *this;
}

Block_8x8 &Block_8x8::operator+=(const Block_8x8& b)
{
    for(int i=0; i<size; i++){
        data[i]+=b.data[i];
    }
    return *this;
}

Block_8x8 &Block_8x8::operator-=(const Block_8x8& b)
{
    for(int i=0; i<size; i++){
        data[i]-=b.data[i];
    }
    return *this;
}

Block_8x8 Block_8x8::operator+(const Block_8x8& b)
{
    Block_8x8 r;
    for(int i=0; i<size; i++){
        r.data[i] = data[i] + b.data[i];
    }
    return r;
}

Block_8x8 Block_8x8::operator-(const Block_8x8& b)
{
    Block_8x8 r;
    for(int i=0; i<size; i++){
        r.data[i] = data[i] - b.data[i];
    }
    return r;
}

void Block_8x8::setnumber(int num)
{
    for(int i=0; i<size; i++){
        data[i] = num;
    }
}

void Block_8x8::setneighbor(int u, int l, int r)
{
    upper = u;
    left = l;
    right = r;
}

void Block_8x8::setneighbor(const Block_8x8& b)
{
    upper = b.upper;
    left = b.left;
    right = b.right;
}

void Block_8x8::print()
{
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            cout << data[i*8+j] << ' ';
        }
        cout << endl;
    }
}

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
