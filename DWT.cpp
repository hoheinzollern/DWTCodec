#include "DWT.h"
#include "bitfile.h"
#include "Huffman.h"

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
	fwrite(&realWidth, sizeof(int), 1, fHan);
	fwrite(&realHeight, sizeof(int), 1, fHan);

	int stopHeight = height - (height - realHeight)/2;
	int stopWidth = width - (width - realWidth)/2;
	int stopPrHeight = stopHeight * PREVIEW / height;
	int stopPrWidth = stopWidth * PREVIEW / width;

	int maxAbsVal = 0;
	for (int i = 0; i < stopHeight; i++) {
		for (int j = 0; j < stopWidth; j++){
			if (i >= PREVIEW || j >= PREVIEW) {
				if (abs(coeff[i*width+j]) > maxAbsVal) maxAbsVal = coeff[i*width+j];
			}
		}
	}
	cout << "maxValue: " << maxAbsVal << endl;
	fwrite(&maxAbsVal, sizeof(int), 1, fHan);

	float C = RANGE/(float)(maxAbsVal*2);
	float W = 1;
	for (int i = PREVIEW; i < height; i <<= 1) W *= 1/sqrt(2);
	for (int i = PREVIEW; i < width; i <<= 1) W *= 1/sqrt(2);

	unsigned int *occ = new unsigned int[1 << 12];
	memset(occ, 0, (1 << 12)*sizeof(int));
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++){
			if (i >= PREVIEW || j >= PREVIEW) {
				if (coeff[i*width + j] != 0)
					occ[range((coeff[i*width + j]+maxAbsVal)*C)]++;
			}
		}
	}
	Huffman *huffman = new Huffman();
	huffman->buildTree(occ, 1<<12);
	delete[] occ;

	for (int i = 0; i < stopPrHeight && i < PREVIEW; i++) {
		for (int j = 0; j < stopPrWidth && j < PREVIEW; j++) {
			unsigned char l = coeff[i*width + j] * W;
			fwrite(&l, 1, 1, fHan);
		}
	}

	int z = 0;
	bit_file_t *bf = MakeBitFile(fHan, BF_APPEND);
	huffman->setFile(bf);
	huffman->writeTree();
	for (int i = 0; i < stopHeight; i++) {
		for (int j = 0; j < stopWidth; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				if (coeff[i * width + j]==0) z++;
				if (coeff[i * width + j]!=0 || (i==stopHeight-1 && j==stopWidth-1)) {
					if (z != 0) {
						if (z <= 8) {
							unsigned int n = z-1;
							BitFilePutBit(1, bf);
							BitFilePutBit(0, bf);
							BitFilePutBitsInt(bf, &n, 3, sizeof(n));
						} else {
							while (z > 0) {
								unsigned int n = z > 256 ? 255 : z-1;
								BitFilePutBit(1, bf);
								BitFilePutBit(1, bf);
								BitFilePutBitsInt(bf, &n, 8, sizeof(n));
								z -= 256;
							}
						}
						z = 0;
					}
					if (i!=stopHeight-1 || j!=stopWidth-1) {
						unsigned int l = range((coeff[i*width + j]+maxAbsVal)*C);
						BitFilePutBit(0, bf);
						huffman->writeSymbol(l);
					}
				}
			}
		}
	}
	BitFileFlushOutput(bf, 0);
	delete huffman;

	fclose(fHan);
}

void DWT::loadDWT(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "rb");
	fread(&realWidth, sizeof(int), 1, fHan);
	fread(&realHeight, sizeof(int), 1, fHan);
	width = 1;
	while (width < realWidth) width <<= 1;
	height = 1;
	while (height < realHeight) height <<= 1;

	int stopHeight = height - (height - realHeight)/2;
	int stopWidth = width - (width - realWidth)/2;
	int stopPrHeight = stopHeight * PREVIEW / height;
	int stopPrWidth = stopWidth * PREVIEW / width;

	cout	<< "width: " << width << endl
			<< "height: " << height << endl
			<< "realWidth: " << realWidth << endl
			<< "realHeight: " << realHeight << endl;
	coeff = new float[width * height];

	int maxAbsVal;
	fread(&maxAbsVal, sizeof(int), 1, fHan);
	cout << "maxValue: " << maxAbsVal << endl;

	float C = RANGE/(float)(maxAbsVal*2);
	float W = 1;
	for (int i = PREVIEW; i < height; i <<= 1) W *= 1/sqrt(2);
	for (int i = PREVIEW; i < width; i <<= 1) W *= 1/sqrt(2);

	for (int i = 0; i < stopPrHeight && i < PREVIEW; i++) {
		for (int j = 0; j < stopPrWidth && j < PREVIEW; j++) {
			unsigned char l;
			fread(&l, 1, 1, fHan);
			coeff[i*width + j] = l/W;
		}
	}

	unsigned int z = 0;
	bit_file_t *bf = MakeBitFile(fHan, BF_READ);
	Huffman *huffman = new Huffman();
	huffman->setFile(bf);
	huffman->readTree();
	for (int i = 0; i < stopHeight; i++) {
		for (int j = 0; j < stopWidth; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				int l = 0;
				if (z > 0) {
					coeff[i * width + j] = 0;
					z--;
				} else {
					bool seq0 = BitFileGetBit(bf);
					if (seq0) {
						coeff[i * width + j] = 0;
						bool cod8 = BitFileGetBit(bf);
						if (cod8) {
							BitFileGetBitsInt(bf, &z, 8, sizeof(z));
						} else {
							BitFileGetBitsInt(bf, &z, 3, sizeof(z));
						}
					} else {
						l = huffman->readSymbol();
						coeff[i*width + j] = l/C-maxAbsVal;
					}
				}
			}
		}
	}
	delete huffman;

	fclose(fHan);
}
