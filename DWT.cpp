#include "DWT.h"
#include "BitReader.h"
#include "BitWriter.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <iostream>
#include <cstdio>
#include <stdint.h>
#include <cmath>

#define PREVIEW 64
#define RANGE (0xFFF)

void transform(float *src, int length, int step)
{
	float *tmp = new float[length];
	for (int len = length/2; len >= PREVIEW; len /= 2) {
		for (int i = 0; i < len; i++) {
			float c = (src[i*2*step] + src[(i*2+1)*step]);
			float w = (src[i*2*step] - src[(i*2+1)*step]);
			tmp[i] = round(c);
			tmp[i+len] = w;
		}
		for (int i = 0; i < len*2; i++)
			src[i*step] = tmp[i];
	}
	delete[] tmp;
}

void untransform(float *src, int length, int step)
{
	float *tmp = new float[length];
	for (int len = PREVIEW; len < length; len *= 2) {
		for (int i = 0; i < len; i++) {
			float c = (src[i*step] + src[(i+len)*step])/2.0;
			float w = (src[i*step] - src[(i+len)*step])/2.0;
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
	realWidth = input->width();
	realHeight = input->height();
	unsigned char *src = input->getPadded();
	W = new float[width * height];
	for (int i = 0; i < width * height; i++)
		W[i] = src[i];
	delete[] src;

	for (int i = 0; i < height; i++)
		transform(W + i * width, width, 1);

	for (int i = 0; i < width; i++)
		transform(W + i, height, width);
}

DWT::DWT(): input(0)
{
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
			<< " set to 0 (" << float(c) / (width * height) * 100 << "%)" << endl;
}

void DWT::save(const string &fileName)
{
	for (int i = 0; i < height; i++)
		untransform(W + i * width, width, 1);

	for (int i = 0; i < width; i++)
		untransform(W + i, height, width);

	if (!input)
		input = new Bitmap(realWidth, realHeight);
	unsigned char *payload = input->payload();
	for (int i = 0; i < realHeight; i++) {
		for (int j = 0; j < realWidth; j++) {
			payload[i*realWidth + j] = normalize(W[i*width + j]);
		}
	}
	input->writeImage(fileName);
}

uint range(float x)
{
	if (x<0) return 0;
	if (x>RANGE) return RANGE;
	return x;
}

void DWT::saveDWT(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "wb");
	fwrite(&width, sizeof(int), 1, fHan);
	fwrite(&height, sizeof(int), 1, fHan);
	fwrite(&realWidth, sizeof(int), 1, fHan);
	fwrite(&realHeight, sizeof(int), 1, fHan);

	int maxVal = 0;
	int minVal = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++){
			if (i >= PREVIEW || j >= PREVIEW) {
				if (W[i*width+j] > maxVal) maxVal = W[i*width+j];
				if (-W[i*width+j] > maxVal) maxVal = -W[i*width+j];
				if (W[i*width+j] < minVal) minVal = W[i*width+j];
				if (-W[i*width+j] < minVal) minVal = -W[i*width+j];
			}
		}
	}
	fwrite(&minVal, sizeof(int), 1, fHan);
	fwrite(&maxVal, sizeof(int), 1, fHan);
	float C = RANGE/(float)(maxVal-minVal);
	for (int i = 0; i < height && i < PREVIEW; i++) {
		for (int j = 0; j < width && j < PREVIEW; j++) {
			unsigned int l = W[i*width + j];
			fwrite(&l, 4, 1, fHan);
		}
	}
	bool status = 0;
	unsigned int tmp = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				unsigned short int l = range((W[i*width + j]-minVal)*C);
				if (!status) {
					tmp = l << 12;
				} else {
					tmp |= l;
					fwrite(&tmp, 3, 1, fHan);
				}
				status = !status;
			}
		}
	}

	fclose(fHan);
}

void DWT::readDWT(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "rb");
	fread(&width, sizeof(int), 1, fHan);
	fread(&height, sizeof(int), 1, fHan);
	fread(&realWidth, sizeof(int), 1, fHan);
	fread(&realHeight, sizeof(int), 1, fHan);
	cout	<< "width: " << width << endl
			<< "height: " << height << endl
			<< "realWidth: " << realWidth << endl
			<< "realHeight: " << realHeight << endl;
	W = new float[width * height];

	int minVal, maxVal;
	fread(&minVal, sizeof(int), 1, fHan);
	fread(&maxVal, sizeof(int), 1, fHan);
	float C = RANGE/(float)(maxVal-minVal);
	for (int i = 0; i < height && i < PREVIEW; i++) {
		for (int j = 0; j < width && j < PREVIEW; j++) {
			unsigned int l;
			fread(&l, 4, 1, fHan);
			W[i*width + j] = l;
		}
	}
	bool status = 0;
	unsigned int tmp = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				unsigned int l;
				if (!status) {
					fread(&tmp, 3, 1, fHan);
					l = (tmp & 0xFFF000) >> 12;
				} else {
					l = (tmp & 0x000FFF);
				}
				status = !status;
				W[i*width + j] = l/C+minVal;
			}
		}
	}

	fclose(fHan);
}
