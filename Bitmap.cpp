#include "Bitmap.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>

bool Bitmap::palette_initialized = false;
unsigned char Bitmap::palette[256*4] = {};

unsigned char Bitmap::nrm(int val)
{
	if (val > 255) return 255;
	if (val < 0) return 0;
	return val;
}

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
	_payload = new unsigned char[width * height];
	if (!palette_initialized) {
		for (int i = 0; i < 256; i++) {
			palette[i*4] = nrm(i);
			palette[i*4+1] = nrm(i);
			palette[i*4+2] = nrm(i);
			palette[i*4+3] = 0;
		}
		palette_initialized = true;
	}
}

int Bitmap::width()
{
	return v3_header.width;
}

int Bitmap::height()
{
	return v3_header.height;
}

unsigned char *Bitmap::payload()
{
	return _payload;
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
			result[i * padWidth + j] = _payload[i * width() + j];
		for (int j = width(); j < padWidth; j++)
			result[i * padWidth + j] = 0.0f;
	}
	for (int i = height(); i < padHeight; i++)
		for (int j = 0; j < padWidth; j++)
			result[i * padWidth + j] = 0.0f;
	return result;
}

void Bitmap::readImage(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "rb");
	if (fHan == 0) {
		cerr << "Errore nella lettura del file" << endl;
		exit(1);
	}
	bmpfile_magic magic;
	fread(&magic, sizeof(bmpfile_magic), 1, fHan);
	if (magic.magic[0]!=0x42 || magic.magic[1]!=0x4D) {
		cerr << "Formato non riconosciuto" << endl;
		exit(1);
	}
	fread(&header, sizeof(bmpfile_header), 1, fHan);
	fread(&v3_header, sizeof(bmp_dib_v3_header_t), 1, fHan);
	fread(palette, sizeof(palette), 1, fHan);

	cout << v3_header.bitspp << " bpp" <<endl;
	cout << v3_header.ncolors << " ncolors" <<endl;
	cout << v3_header.nimpcolors << " nimpcolors" <<endl;

	_payload = new unsigned char[width() * height()];
	fread(_payload, 1, width() * height(), fHan);
	fclose(fHan);
}

void Bitmap::writeImage(const string &fileName)
{
	FILE *fHan = fopen(fileName.data(), "wb");
	if(fHan == 0) {
		cerr << "Errore nell'apertura' del file" << endl;
		exit(1);
	}
	bmpfile_magic magic;
	magic.magic[0]=0x42;
	magic.magic[1]=0x4D;
	fwrite(&magic, sizeof(bmpfile_magic), 1, fHan);
	fwrite(&header, sizeof(bmpfile_header), 1, fHan);
	fwrite(&v3_header, sizeof(bmp_dib_v3_header_t), 1, fHan);
	fwrite(palette, sizeof(palette), 1, fHan);

	fwrite(_payload, 1, width() * height(), fHan);
	fclose(fHan);
}
