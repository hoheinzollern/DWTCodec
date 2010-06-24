#include "DWT.h"
#include "lib/bitfile.h"
#include "Huffman.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <iostream>
#include <stdint.h>

#define PREVIEW 32
#define RANGE ((1 << bpp)-1)

const char DWT::magic[] = {'A', 'B'};

unsigned char DWT::nrm(int val)
{
	if (val > 255) return 255;
	if (val < 0) return 0;
	return val;
}

unsigned int DWT::range(float x)
{
	if (x<0) return 0;
	if (x>RANGE) return RANGE;
	return x;
}

DWT::DWT(unsigned int bpp) : bpp(bpp) {}

DWT::DWT(Bitmap *input, unsigned int bpp) : bpp(bpp)
{
	width = input->getPadWidth();
	height = input->getPadHeight();
	realWidth = input->width();
	realHeight = input->height();
	coeff = input->getPadded(width, height);
}

DWT::~DWT()
{
	delete [] coeff;
}

/**
  * Discrete Wavelet Transform in one dimension, forward
  */
void DWT::transform1d(float *src, unsigned int length, unsigned int step, float *tmp)
{
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
}


/**
  * Discrete Wavelet Transform in one dimension, backward
  */
void DWT::untransform1d(float *src, unsigned int length, unsigned int step, float *tmp)
{
	float W = 1/sqrt(2.0f);
	for (unsigned int len = PREVIEW; len < length; len *= 2) {
		for (unsigned int i = 0; i < len; i++) {
			float c = src[i*step];
			float w = src[(i+len)*step];
			tmp[i*2] = (c+w)*W;
			tmp[i*2+1] = (c-w)*W;
		}
		for (unsigned int i = 0; i < len*2; i++)
			src[i*step] = tmp[i];
	}
}

/**
  * Applies the DWT algorithm forward
  */
void DWT::transform()
{
	// tmp is allocated here and passed to the subroutine for performance reasons
	float *tmp = new float[max(width, height)];
	for (unsigned int i = 0; i < height; i++)
		transform1d(coeff + i * width, width, 1, tmp);

	for (unsigned int i = 0; i < width; i++)
		transform1d(coeff + i, height, width, tmp);
	delete[] tmp;
}

/**
  * Applies the DWT algorithm backward
  */
void DWT::untrasform()
{
	// tmp is allocated here and passed to the subroutine for performance reasons
	float *tmp = new float[max(width, height)];
	for (unsigned int i = 0; i < height; i++)
		untransform1d(coeff + i * width, width, 1, tmp);

	for (unsigned int i = 0; i < width; i++)
		untransform1d(coeff + i, height, width, tmp);
	delete[] tmp;
}

/**
  * Returns the bitmap representing the untrasformed image
  */
Bitmap *DWT::toBitmap()
{
	Bitmap *result = new Bitmap(realWidth, realHeight);
	for (unsigned int i = 0; i < realHeight; i++) {
		for (unsigned int j = 0; j < realWidth; j++) {
			result->set(i*realWidth + j, nrm(coeff[i*width + j]));
		}
	}
	return result;
}

/**
  * Saves a DWT file
  */
void DWT::save(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "wb");

	// Header
	fwrite(&magic, 1, 2, fHan);
#ifdef __BIG_ENDIAN__
	// Correct endianness for writing
	bpp = swap_endian32(bpp);
	realWidth = swap_endian32(realWidth);
	realHeight = swap_endian32(realHeight);
#endif
	fwrite(&bpp, sizeof(bpp), 1, fHan);
	fwrite(&realWidth, sizeof(int), 1, fHan);
	fwrite(&realHeight, sizeof(int), 1, fHan);
#ifdef __BIG_ENDIAN__
	// Revert endianness
	bpp = swap_endian32(bpp);
	realWidth = swap_endian32(realWidth);
	realHeight = swap_endian32(realHeight);
#endif

	// Drops half of the padding when writing
	unsigned int stopHeight = height - (height - realHeight)/2;
	unsigned int stopWidth = width - (width - realWidth)/2;
	unsigned int stopPrHeight = stopHeight * PREVIEW / height;
	unsigned int stopPrWidth = stopWidth * PREVIEW / width;

	// Looking for the highest absolute value in the transformation, used for quantization
	float maxAbsValF = 0;
	for (unsigned int i = 0; i < stopHeight; i++) {
		for (unsigned int j = 0; j < stopWidth; j++){
			if (i >= PREVIEW || j >= PREVIEW) {
				if (abs(coeff[i*width+j]) > maxAbsValF) maxAbsValF = abs(coeff[i*width+j]);
			}
		}
	}
	int maxAbsVal = round(maxAbsValF);
	cout << "maxValue: " << maxAbsVal << endl;
	fwrite(&maxAbsVal, sizeof(int), 1, fHan);

	const float C = ((1 << (bpp-1))-1)/(float)(maxAbsVal);	// Range value
	const float M = (1 << (bpp-1));							// Added to get only positive values
	float W = 1, w = 1/sqrt(2);							// Factor of multiplication for preview
	for (unsigned int i = PREVIEW; i < height; i <<= 1) W *= w;
	for (unsigned int i = PREVIEW; i < width; i <<= 1) W *= w;

	// Huffman, searching for occurrences in the quantization
	unsigned int *occ = new unsigned int[1 << bpp];
	memset(occ, 0, (1 << bpp)*sizeof(int));
	for (unsigned int i = 0; i < stopHeight; i++) {
		for (unsigned int j = 0; j < stopWidth; j++){
			if (i >= PREVIEW || j >= PREVIEW) {
				float quant = coeff[i * width + j] * C;
				if (abs(quant) >= .5f)
					occ[range(round(quant + M))]++;
			}
		}
	}
	Huffman *huffman = new Huffman(bpp);
	huffman->buildTree(occ, 1<<bpp);
	delete[] occ;

	// Encoding of the preview
	for (unsigned int i = 0; i < stopPrHeight && i < PREVIEW; i++) {
		for (unsigned int j = 0; j < stopPrWidth && j < PREVIEW; j++) {
			unsigned char l = coeff[i*width + j] * W;
			fwrite(&l, 1, 1, fHan);
		}
	}

	// Encoding of the rest of the transform, using Huffman and RLE
	int zeros = 0;
	bit_file_t *bf = MakeBitFile(fHan, BF_APPEND);
	huffman->setFile(bf);
	huffman->writeTree();
	for (unsigned int i = 0; i < stopHeight; i++) {
		for (unsigned int j = 0; j < stopWidth; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				bool zero = abs(coeff[i*width + j]*C) < .5f;
				if (zero) zeros++;
				if (!zero || (i==stopHeight-1 && j==stopWidth-1)) {
					if (zeros != 0) {
						// RLE: a sequence of zeros has been found
						if (zeros <= 8) {
							unsigned int n = zeros-1;
							BitFilePutBit(1, bf);
							BitFilePutBit(0, bf);
							BitFilePutBitsInt(bf, &n, 3, sizeof(n));
						} else {
							while (zeros > 0) {
								unsigned int n = zeros > 256 ? 255 : zeros-1;
								BitFilePutBit(1, bf);
								BitFilePutBit(1, bf);
								BitFilePutBitsInt(bf, &n, 8, sizeof(n));
								zeros -= 256;
							}
						}
						zeros = 0;
					}
					if (i!=stopHeight-1 || j!=stopWidth-1) {
						// Huffman: write a quantized and then Huffman-encoded value
						unsigned int l = range(round(coeff[i*width + j]*C + M));
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

/**
  * Loads a DWT file
  */
void DWT::load(const string &fileName)
{
	// Open file and read magic number
	FILE *fHan = fopen(fileName.data(), "rb");
	char thisMagic[2];
	fread(thisMagic, 1, 2, fHan);
	if (magic[0] != thisMagic[0] || magic[1] != thisMagic[1]) {
		cerr << "Unrecognized file format for " << fileName << endl;
		exit(1);
	}

	// Read header
	fread(&bpp, sizeof(bpp), 1, fHan);
	fread(&realWidth, sizeof(int), 1, fHan);
	fread(&realHeight, sizeof(int), 1, fHan);
#ifdef __BIG_ENDIAN__
	bpp = swap_endian32(bpp);
	realWidth = swap_endian32(realWidth);
	realHeight = swap_endian32(realHeight);
#endif

	// Calculate padded width and height
	width = 1;
	while (width < realWidth) width <<= 1;
	height = 1;
	while (height < realHeight) height <<= 1;

	// Drops half of the padding when reading
	unsigned int stopHeight = height - (height - realHeight)/2;
	unsigned int stopWidth = width - (width - realWidth)/2;
	unsigned int stopPrHeight = stopHeight * PREVIEW / height;
	unsigned int stopPrWidth = stopWidth * PREVIEW / width;

	cout	<< "bpp: " << bpp << endl
			<< "width: " << width << endl
			<< "height: " << height << endl
			<< "realWidth: " << realWidth << endl
			<< "realHeight: " << realHeight << endl;

	coeff = new float[width * height];
	int maxAbsVal;
	fread(&maxAbsVal, sizeof(int), 1, fHan);
	cout << "maxValue: " << maxAbsVal << endl;

	const float C = ((1 << (bpp-1))-1)/(float)(maxAbsVal);	// Range value
	const float M = (1 << (bpp-1));							// Added to get only positive values
	float W = 1, w = 1/sqrt(2);							// Factor of multiplication for preview
	for (unsigned int i = PREVIEW; i < height; i <<= 1) W *= w;
	for (unsigned int i = PREVIEW; i < width; i <<= 1) W *= w;

	// Reading the preview
	for (unsigned int i = 0; i < stopPrHeight && i < PREVIEW; i++) {
		for (unsigned int j = 0; j < stopPrWidth && j < PREVIEW; j++) {
			unsigned char l;
			fread(&l, 1, 1, fHan);
			coeff[i*width + j] = l/W;
		}
	}

	// Reading the rest of the transform, decoding Huffman and RLE
	unsigned int zeros = 0;
	bit_file_t *bf = MakeBitFile(fHan, BF_READ);
	Huffman *huffman = new Huffman(bpp);
	huffman->setFile(bf);
	huffman->readTree();
	for (unsigned int i = 0; i < stopHeight; i++) {
		for (unsigned int j = 0; j < stopWidth; j++) {
			if (i >= PREVIEW || j >= PREVIEW) {
				int l = 0;
				if (zeros > 0) {
					coeff[i * width + j] = 0;
					zeros--;
				} else {
					bool seq0 = BitFileGetBit(bf);
					if (seq0) {
						// RLE: read the number of coefficents to set to 0 and set the first
						coeff[i * width + j] = 0;
						bool cod8 = BitFileGetBit(bf);
						if (cod8) {
							BitFileGetBitsInt(bf, &zeros, 8, sizeof(zeros));
						} else {
							BitFileGetBitsInt(bf, &zeros, 3, sizeof(zeros));
						}
					} else {
						// Huffman: read coefficent and dequantize it
						l = huffman->readSymbol();
						coeff[i*width + j] = (l-M)/C;
					}
				}
			}
		}
	}
	delete huffman;

	fclose(fHan);
}
