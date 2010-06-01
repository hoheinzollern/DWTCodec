#include "DWT.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

void transform(int *src, int length, int step)
{
	int *tmp = new int[length];
	for (int len = length/2; len > 0; len /= 2) {
		for (int i = 0; i < len; i++) {
			float c = (src[i*2*step] + src[(i*2+1)*step]);
			float w = (src[i*2*step] - src[(i*2+1)*step]);
			tmp[i] = c;
			tmp[i+len] = w;
		}
		for (int i = 0; i < len*2; i++)
			src[i*step] = tmp[i];
	}
	delete[] tmp;
}

void untransform(int *src, int length, int step)
{
	int *tmp = new int[length];
	for (int len = 1; len < length; len *= 2) {
		for (int i = 0; i < len; i++) {
			float c = (src[i*step] + src[(i+len)*step]) / 2;
			float w = (src[i*step] - src[(i+len)*step]) / 2;
			tmp[i*2] = c;
			tmp[i*2+1] = w;
		}
		for (int i = 0; i < len*2; i++)
			src[i*step] = tmp[i];
	}
	delete[] tmp;
}

unsigned char normalize(int val)
{
	if (val > 255) return 255;
	if (val < 0) return 0;
	return val;
}

DWT::DWT(Bitmap *input)
{
	this->input = input;
	width = input->getPadWidth();
	height = input->getPadHeight();
	unsigned char *src = input->getPadded();
	W = new int[width * height];
	for (int i = 0; i < width * height; i++)
		W[i] = src[i];
	delete[] src;

	for (int i = 0; i < height; i++)
		transform(W + i * width, width, 1);

	for (int i = 0; i < width; i++)
		transform(W + i, height, width);
}

void DWT::compress(int threshold)
{
	int c = 0;
	for (int i = 0; i < height * width; i++) {
		if (W[i] < threshold && W[i] > -threshold) {
			W[i] = 0;
			c++;
		}
	}
	cout << c << " coefficients over " << width * height
		<< " set to 0 (" << float(c)/(width*height)*100 << "%)" << endl;
}

void DWT::save(const string &fileName)
{
	for (int i = 0; i < height; i++)
		untransform(W + i * width, width, 1);

	for (int i = 0; i < width; i++)
		untransform(W + i, height, width);

	int outWidth = input->width(), outHeight = input->height();
	unsigned char *payload = input->payload();
	for (int i = 0; i < outHeight; i++) {
		for (int j = 0; j < outWidth; j++) {
			payload[i*outWidth + j] = normalize(W[i*width + j]);
		}
	}
	input->writeImage(fileName);
}
