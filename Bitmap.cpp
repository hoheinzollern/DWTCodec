#include "Bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

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

	_payload = new unsigned char[width() * height()];
	unsigned char padding[4];
	int padding_size = width() % 4;
	for (int i = 0; i < width(); i++) {
		fread(_payload + i * width(), 1, width(), fHan);
		if (padding_size != 0)
			fread(padding, padding_size, 1, fHan);
	}
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

	unsigned char padding[4];
	int padding_size = width() % 4;
	for (int i = 0; i < width(); i++) {
		fwrite(_payload + i * width(), 1, width(), fHan);
		if (padding_size != 0)
			fwrite(padding, padding_size, 1, fHan);
	}
	fclose(fHan);
}
