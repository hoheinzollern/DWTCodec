#include "BitWriter.h"

void BitWriter::writeBit(bool x) {
	b |= (x ? 1 : 0) << s;
	s++;
	if (s == 8)
	{
		fwrite(&b, sizeof(b), 1, fHan);
		b = 0;
		s = 0;
	}
}

void BitWriter::writeByte(unsigned char x) {
	b |= x << (8+s);
	fwrite(&b, sizeof(b), 1, fHan);
	b = (x >> (8-s));
}

void BitWriter::flush() {
	if (s)
		fwrite(&b, sizeof(b), 1, fHan);
}
