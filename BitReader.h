#ifndef BITREADER_H
#define BITREADER_H

#include <cstdio>

class BitReader {
	FILE *fHan;
	int s;
	unsigned char b;
public:
	BitReader(FILE *f): fHan(f), s(0), b(0) {}
	bool readBit();
	unsigned char readByte();
};

#endif
