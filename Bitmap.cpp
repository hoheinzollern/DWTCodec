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

#include "Bitmap.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

uint16_t swap_endian16(uint16_t src)
{
	uint16_t dst;
	char *srcP = (char*)&src, *dstP = (char*)&dst;
	
	dstP[1] = srcP[0];
	dstP[0] = srcP[1];
	return dst;
}

uint32_t swap_endian32(uint32_t src)
{
	uint32_t dst;
	char *srcP = (char*)&src, *dstP = (char*)&dst;
	
	dstP[3] = srcP[0];
	dstP[2] = srcP[1];
	dstP[1] = srcP[2];
	dstP[0] = srcP[3];
	return dst;
}


bool Bitmap::palette_initialized = false;
unsigned char Bitmap::palette[256*4] = {};

Bitmap::Bitmap()
{
}

Bitmap::Bitmap(int width, int height)
{
	header.bmp_offset = DIM_HEAD_BMP;
	memset(&v3_header, 0, sizeof(v3_header));
	v3_header.header_sz = sizeof(v3_header);
	v3_header.width = width;
	v3_header.height = height;
	v3_header.nplanes = 1;
	v3_header.bitspp = 8;
	v3_header.ncolors = 256;
	v3_header.nimpcolors = 256;
	payload = new unsigned char[width * height];
	if (!palette_initialized) {
		for (int i = 0; i < 256; i++) {
			palette[i*4] = i;
			palette[i*4+1] = i;
			palette[i*4+2] = i;
			palette[i*4+3] = 0;
		}
		palette_initialized = true;
	}
}

Bitmap::~Bitmap()
{
	delete []payload;
}

int Bitmap::width()
{
	return v3_header.width;
}

int Bitmap::height()
{
	return v3_header.height;
}

void Bitmap::set(int pos, unsigned char val)
{
	payload[pos] = val;
}

int Bitmap::padding()
{
	return ((4 - width() % 4) % 4);
}

int Bitmap::getPadWidth()
{
	int padWidth = 1;
	while (padWidth < width()) padWidth <<= 1;
	return padWidth;
}

int Bitmap::getPadHeight()
{
	int padHeight = 1;
	while (padHeight < height()) padHeight <<= 1;
	return padHeight;
}

float *Bitmap::getPadded(int padWidth, int padHeight)
{
	float *result = new float[padWidth * padHeight];
	for (int i = 0; i < height(); i++) {
		for (int j = 0; j < width(); j++)
			result[i * padWidth + j] = payload[i * width() + j];
		for (int j = width(); j < padWidth; j++)
			result[i * padWidth + j] = 0.0f;
	}
	for (int i = height(); i < padHeight; i++)
		for (int j = 0; j < padWidth; j++)
			result[i * padWidth + j] = 0.0f;
	return result;
}

void Bitmap::readImage(FILE *fHan)
{
	bmpfile_magic magic;
	fread(&magic, 1, 2, fHan);
	if (magic.magic[0]!=0x42 || magic.magic[1]!=0x4D) {
		cerr << "Formato not recognized" << endl;
		exit(1);
	}

	// Reading header
	fread(&(header.filesz), sizeof(uint32_t), 1, fHan);
	fread(&(header.creator1), sizeof(uint16_t), 1, fHan);
	fread(&(header.creator2), sizeof(uint16_t), 1, fHan);
	fread(&(header.bmp_offset), sizeof(uint32_t), 1, fHan);
#ifdef __BIG_ENDIAN__
	header.filesz = swap_endian32(header.filesz);
	header.creator1 = swap_endian16(header.creator1);
	header.creator2 = swap_endian16(header.creator2);
	header.bmp_offset = swap_endian32(header.bmp_offset);
#endif
	
	// Reading v3_header
	fread(&(v3_header.header_sz), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.width), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.height), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.nplanes), sizeof(uint16_t), 1, fHan);
	fread(&(v3_header.bitspp), sizeof(uint16_t), 1, fHan);
	fread(&(v3_header.compress_type), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.bmp_bytesz), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.hres), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.vres), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.ncolors), sizeof(uint32_t), 1, fHan);
	fread(&(v3_header.nimpcolors), sizeof(uint32_t), 1, fHan);
#ifdef __BIG_ENDIAN__
	v3_header.header_sz = swap_endian32(v3_header.header_sz);
	v3_header.width = swap_endian32(v3_header.width);
	v3_header.height = swap_endian32(v3_header.height);
	v3_header.nplanes = swap_endian16(v3_header.nplanes);
	v3_header.bitspp = swap_endian16(v3_header.bitspp);
	v3_header.compress_type = swap_endian32(v3_header.compress_type);
	v3_header.bmp_bytesz = swap_endian32(v3_header.bmp_bytesz);
	v3_header.hres = swap_endian32(v3_header.hres);
	v3_header.vres = swap_endian32(v3_header.vres);
	v3_header.ncolors = swap_endian32(v3_header.ncolors);
	v3_header.nimpcolors = swap_endian32(v3_header.nimpcolors);
#endif

	// Reading palette
	fread(palette, 1, sizeof(palette), fHan);

	if (v3_header.bitspp != 8 || v3_header.ncolors != 256) {
		cerr << "Bad bitmap format: only grayscale bitmaps (8bit) are allowed" << endl;
		exit(1);
	}

	payload = new unsigned char[width() * height()];
	unsigned char pad[4];
	for (int i = 0; i < height(); i++) {
		fread(payload + i * width(), 1, width(), fHan);
		if (padding() > 0)
			fread(pad, padding(), 1, fHan);
	}
}

void Bitmap::readImage(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "rb");
	if (fHan == 0) {
		cerr << "Errore opening the file" << endl;
		exit(1);
	}
	readImage(fHan);
	fclose(fHan);
}

void Bitmap::writeImage(FILE *fHan)
{
	bmpfile_magic magic;
	magic.magic[0]=0x42;
	magic.magic[1]=0x4D;

	fwrite(&magic, 1, 2, fHan);

	// Writing header
#ifdef __BIG_ENDIAN__
	// Correct endianness for writing
	header.filesz = swap_endian32(header.filesz);
	header.creator1 = swap_endian16(header.creator1);
	header.creator2 = swap_endian16(header.creator2);
	header.bmp_offset = swap_endian32(header.bmp_offset);
#endif
	fwrite(&(header.filesz), sizeof(uint32_t), 1, fHan);
	fwrite(&(header.creator1), sizeof(uint16_t), 1, fHan);
	fwrite(&(header.creator2), sizeof(uint16_t), 1, fHan);
	fwrite(&(header.bmp_offset), sizeof(uint32_t), 1, fHan);
#ifdef __BIG_ENDIAN__
	// Revert endianness
	header.filesz = swap_endian32(header.filesz);
	header.creator1 = swap_endian16(header.creator1);
	header.creator2 = swap_endian16(header.creator2);
	header.bmp_offset = swap_endian32(header.bmp_offset);
#endif

	// Writing v3_header
#ifdef __BIG_ENDIAN__
	// Correct endianness for writing
	v3_header.header_sz = swap_endian32(v3_header.header_sz);
	v3_header.width = swap_endian32(v3_header.width);
	v3_header.height = swap_endian32(v3_header.height);
	v3_header.nplanes = swap_endian16(v3_header.nplanes);
	v3_header.bitspp = swap_endian16(v3_header.bitspp);
	v3_header.compress_type = swap_endian32(v3_header.compress_type);
	v3_header.bmp_bytesz = swap_endian32(v3_header.bmp_bytesz);
	v3_header.hres = swap_endian32(v3_header.hres);
	v3_header.vres = swap_endian32(v3_header.vres);
	v3_header.ncolors = swap_endian32(v3_header.ncolors);
	v3_header.nimpcolors = swap_endian32(v3_header.nimpcolors);
#endif
	fwrite(&(v3_header.header_sz), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.width), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.height), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.nplanes), sizeof(uint16_t), 1, fHan);
	fwrite(&(v3_header.bitspp), sizeof(uint16_t), 1, fHan);
	fwrite(&(v3_header.compress_type), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.bmp_bytesz), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.hres), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.vres), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.ncolors), sizeof(uint32_t), 1, fHan);
	fwrite(&(v3_header.nimpcolors), sizeof(uint32_t), 1, fHan);
#ifdef __BIG_ENDIAN__
	// Revert endianness
	v3_header.header_sz = swap_endian32(v3_header.header_sz);
	v3_header.width = swap_endian32(v3_header.width);
	v3_header.height = swap_endian32(v3_header.height);
	v3_header.nplanes = swap_endian16(v3_header.nplanes);
	v3_header.bitspp = swap_endian16(v3_header.bitspp);
	v3_header.compress_type = swap_endian32(v3_header.compress_type);
	v3_header.bmp_bytesz = swap_endian32(v3_header.bmp_bytesz);
	v3_header.hres = swap_endian32(v3_header.hres);
	v3_header.vres = swap_endian32(v3_header.vres);
	v3_header.ncolors = swap_endian32(v3_header.ncolors);
	v3_header.nimpcolors = swap_endian32(v3_header.nimpcolors);
#endif

	// Writing palette
	fwrite(palette, 1, sizeof(palette), fHan);

	unsigned char pad[4] = {0, 0, 0, 0};
	for (int i = 0; i < height(); i++) {
		fwrite(payload + i * width(), 1, width(), fHan);
		if (padding() > 0)
			fwrite(pad, padding(), 1, fHan);
	}
}

void Bitmap::writeImage(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "wb");
	if(fHan == 0) {
		cerr << "Errore opening the file" << endl;
		exit(1);
	}
	writeImage(fHan);
	fclose(fHan);
}
