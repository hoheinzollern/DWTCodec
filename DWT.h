#ifndef DWT_H
#define DWT_H

#include "Bitmap.h"
#include <string>
using namespace std;

class DWT {
private:
	Bitmap *bitmap;
	Bitmap *output;
	float *coeff;
	int width;
	int height;
	int realWidth;
	int realHeight;
	void transform1d(float *src, int length, int step);
	void untransform1d(float *src, int length, int step);
public:
	DWT();
	DWT(Bitmap *input);
	void transform();
	void untrasform();
	void compress(int threshold);
	void saveBMP(const string &fileName);
	void saveDWT(const string &fileName);
	void loadDWT(const string &fileName);
};

#endif
