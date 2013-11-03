// Copyright (C) 2010 Alessandro Bruni
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
// 02110-1301, USA.

#ifndef DWT_H
#define DWT_H

#include "Bitmap.h"

#include <string>
using namespace std;

/**
  * Applies the Discrete Wavelet Transformation to compress bitmap images.
  * Uses arbitrary quantization to 8-16bits per coefficient and compresses
  * the output using Huffman + RLE (only on zeros).
  * File format:
  * | Header | Payload |
  *
  * Header: | AB | bpp | width | height | Huffman tree |
  * AB: magic number
  * bpp: bits per pixel
  * width: real width of the original image
  * height: real height of the original image
  * Huffman tree: the tree used to decode Huffman encoded coefficents
  *
  * Payload: 0 + Huffman encoded coefficent | 1 0 + RLE 3 bits | 1 1 + RLE 8 bits
  */
class DWT {
private:
	static const char magic[2];

	// Matrix of coefficients
	float *coeff;

	// Quantization coefficient
	unsigned int bpp;

	// Padded width and height
	unsigned int width;
	unsigned int height;

	// Real width and height of the image
	unsigned int realWidth;
	unsigned int realHeight;

	unsigned char nrm(int val);
	unsigned int range(float val);

	void transform1d(float *src, unsigned int length, unsigned int step, float *tmp);
	void untransform1d(float *src, unsigned int length, unsigned int step, float *tmp);
public:
	DWT(unsigned int bpp);
	DWT(Bitmap *input, unsigned int bpp);
	~DWT();

	void transform();
	void untrasform();
	Bitmap *toBitmap();
	void save(const string &fileName);
	void load(const string &fileName);
};

#endif
