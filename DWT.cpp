#include "DWT.h"
#include "BitReader.h"
#include "BitWriter.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <iostream>
#include <stdint.h>

#define PREVIEW 64
#define RANGE (0xFFF)

unsigned char nrm(int val)
{
	if (val > 255) return 255;
	if (val < 0) return 0;
	return val;
}

uint range(float x)
{
	if (x<0) return 0;
	if (x>RANGE) return RANGE;
	return x;
}

DWT::DWT(Bitmap *input)
{
	bitmap = input;
	width = input->getPadWidth();
	height = input->getPadHeight();
	realWidth = input->width();
	realHeight = input->height();
	coeff = input->getPadded(width, height);
}

DWT::DWT(): bitmap(0)
{
}

void DWT::transform1d(float *src, int length, int step)
{
	float *tmp = new float[length];
	float W = 1/sqrt(2.0f);
	for (int len = length/2; len >= PREVIEW; len /= 2) {
		for (int i = 0; i < len; i++) {
			float c = src[i*2*step];
			float w = src[(i*2+1)*step];
			tmp[i] = (c+w)*W;
			tmp[i+len] = (c-w)*W;
		}
		for (int i = 0; i < len*2; i++)
			src[i*step] = tmp[i];
	}
	delete[] tmp;
}

void DWT::untransform1d(float *src, int length, int step)
{
	float *tmp = new float[length];
	float W = 1/sqrt(2.0f);
	for (int len = PREVIEW; len < length; len *= 2) {
		for (int i = 0; i < len; i++) {
			float c = src[i*step];
			float w = src[(i+len)*step];
			tmp[i*2] = (c+w)*W;
			tmp[i*2+1] = (c-w)*W;
		}
		for (int i = 0; i < len*2; i++)
			src[i*step] = tmp[i];
	}
	delete[] tmp;
}

void DWT::transform()
{
	for (int i = 0; i < height; i++)
		transform1d(coeff + i * width, width, 1);

	for (int i = 0; i < width; i++)
		transform1d(coeff + i, height, width);
}

void DWT::untrasform()
{
	for (int i = 0; i < height; i++)
		untransform1d(coeff + i * width, width, 1);

	for (int i = 0; i < width; i++)
		untransform1d(coeff + i, height, width);
}

void DWT::compress(int threshold)
{
	int c = 0;
	for (int i = 0; i < height * width; i++) {
		if (coeff[i] < threshold && coeff[i] > -threshold) {
			coeff[i] = 0;
			c++;
		}
	}
	cout << c << " coefficients over " << width * height
			<< " set to 0 (" << float(c) / (width * height) * 100 << "%)" << endl;
}

void DWT::saveBMP(const string &fileName)
{
	if (!bitmap)
		bitmap = new Bitmap(realWidth, realHeight);
	unsigned char *payload = bitmap->payload();
	for (int i = 0; i < realHeight; i++) {
		for (int j = 0; j < realWidth; j++) {
			payload[i*realWidth + j] = nrm(coeff[i*width + j]);
		}
	}
	bitmap->writeImage(fileName);
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
				if (coeff[i*width+j] > maxVal) maxVal = coeff[i*width+j];
				if (-coeff[i*width+j] > maxVal) maxVal = -coeff[i*width+j];
				if (coeff[i*width+j] < minVal) minVal = coeff[i*width+j];
				if (-coeff[i*width+j] < minVal) minVal = -coeff[i*width+j];
			}
		}
	}
	cout << "minValue: " << minVal << endl << "maxValue: " << maxVal << endl;
	fwrite(&minVal, sizeof(int), 1, fHan);
	fwrite(&maxVal, sizeof(int), 1, fHan);

	float C = RANGE/(float)(maxVal-minVal);
	float W = 1;
	for (int i = PREVIEW; i < height; i <<= 1) W *= 1/sqrt(2);
	for (int i = PREVIEW; i < width; i <<= 1) W *= 1/sqrt(2);

	int stopHeight = height - (height - realHeight)/2;
	int stopWidth = width - (width - realWidth)/2;
	int stopPrHeight = stopHeight * PREVIEW / height;
	int stopPrWidth = stopWidth * PREVIEW / width;

	for (int i = 0; i < stopPrHeight && i < PREVIEW; i++) {
		for (int j = 0; j < stopPrWidth && j < PREVIEW; j++) {
			unsigned char l = coeff[i*width + j] * W;
			fwrite(&l, 1, 1, fHan);
		}
	}

	bool status = 0;
	unsigned int tmp = 0;
	for (int i = 0; i < stopHeight; i++) {
		for (int j = 0; j < stopWidth; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				unsigned int l = range((coeff[i*width + j]-minVal)*C);
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

void DWT::loadDWT(const string &fileName)
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
	coeff = new float[width * height];

	int minVal, maxVal;
	fread(&minVal, sizeof(int), 1, fHan);
	fread(&maxVal, sizeof(int), 1, fHan);

	float C = RANGE/(float)(maxVal-minVal);
	float W = 1;
	for (int i = PREVIEW; i < height; i <<= 1) W *= 1/sqrt(2);
	for (int i = PREVIEW; i < width; i <<= 1) W *= 1/sqrt(2);

	int stopHeight = height - (height - realHeight)/2;
	int stopWidth = width - (width - realWidth)/2;
	int stopPrHeight = stopHeight * PREVIEW / height;
	int stopPrWidth = stopWidth * PREVIEW / width;

	for (int i = 0; i < stopPrHeight && i < PREVIEW; i++) {
		for (int j = 0; j < stopPrWidth && j < PREVIEW; j++) {
			unsigned char l;
			fread(&l, 1, 1, fHan);
			coeff[i*width + j] = l/W;
		}
	}

	bool status = 0;
	unsigned int tmp = 0;
	for (int i = 0; i < stopHeight; i++) {
		for (int j = 0; j < stopWidth; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				unsigned int l;
				if (!status) {
					fread(&tmp, 3, 1, fHan);
					l = (tmp & 0xFFF000) >> 12;
				} else {
					l = (tmp & 0x000FFF);
				}
				status = !status;
				coeff[i*width + j] = l/C+minVal;
			}
		}
	}

	fclose(fHan);
}
