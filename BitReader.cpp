#include "BitReader.h"

bool BitReader::readBit() {
	if (s == 0) {
		fread(&b, sizeof(b), 1, fHan);
		s = 8;
	}
	bool result = b & 1;
	b <<= 1;
	s--;
	return result;
}

unsigned char BitReader::readByte() {
	unsigned char result = b >> (8-s);
	fread(&b, sizeof(b), 1, fHan);
	result |= b << (8+s);
	return result;
}
