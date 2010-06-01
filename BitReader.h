#ifndef BITREADER_H
#define BITREADER_H

#include <cstdio>

class BitReader {
	FILE *fHan;
	int s;
	unsigned char b;
public:
	BitReader(FILE *f): fHan(f), s(0), b(0) {}
	bool readBit() {
		if (s == 0) {
			fread(&b, sizeof(b), 1, fHan);
			s = 8;
		}
		bool result = b & 1;
		b <<= 1;
		s--;
		return result;
	}
	unsigned char readByte() {
		unsigned char result = b >> (8-s);
		fread(&b, sizeof(b), 1, fHan);
		result |= b << (8+s);
		return result;
	}
};

#endif
