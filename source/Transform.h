#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

#include<iostream>
#include<cmath>
#include"Block.h"
using namespace std;

class Transform{
public:
    static void dct1d(double* in, double* out, const int count);
    static void dct2d(double* in, double* out, const int count);
    static void idct1d(double* in, double* out, const int count);
    static void idct2d(double* in, double* out, const int count);

    static void transform(int* in, int* out, const int count);
    static void transform(Block_8x8& in, Block_8x8& out);
    static void itransform(int* in, int* out, const int count);
    static void itransform(Block_8x8& in, Block_8x8& out);
};

void Transform::transform(int* in, int* out, const int count)
{
    double* dctin = new double[count*count];
    double* dctout = new double[count*count];
    for(int i=0; i<count*count; i++){
        dctin[i] = in[i];
    }
    dct2d(dctin, dctout, count);
    for(int i=0; i<count*count; i++){
        out[i] = round(dctout[i]);
    }
}

void Transform::transform(Block_8x8& in, Block_8x8& out)
{
    Transform::transform(in.data, out.data, 8);
}

void Transform::itransform(int* in, int* out, const int count)
{
    double* dctin = new double[count*count];
    double* dctout = new double[count*count];
    for(int i=0; i<count*count; i++){
        dctin[i] = in[i];
    }
    idct2d(dctin, dctout, count);
    for(int i=0; i<count*count; i++){
        out[i] = round(dctout[i]);
    }
}

void Transform::itransform(Block_8x8& in, Block_8x8& out)
{
    Transform::itransform(in.data, out.data, 8);
}

void Transform::dct1d(double* in, double* out, const int count)
{
	for(int u=0; u<count; u++){
		double z = 0.0;
		for(int x=0; x<count; x++){
			z += in[x] * cos(M_PI*u*(2*x+1)/(2*count));
		}
		if(u == 0) z *= sqrt(1.0/count);
		else z *= sqrt(2.0/count);
        out[u] = z;
	}
}

void Transform::dct2d(double* in, double* out, const int count)
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

void Transform::idct1d(double* in, double* out, const int count){
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

void Transform::idct2d(double* in, double* out, const int count){
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


#endif // TRANSFORM_H_INCLUDED
