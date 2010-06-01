#ifndef BITWRITER_H
#define BITWRITER_H
#include <cstdio>

class BitWriter {
	FILE *fHan;
	int s;
	unsigned char b;
public:
	BitWriter(FILE *f): fHan(f), s(0), b(0) {}
	void writeBit(bool x) {
		b |= (x ? 1 : 0) << s;
		s++;
		if (s == 8)
		{
			fwrite(&b, sizeof(b), 1, fHan);
			b = 0;
			s = 0;
		}
	}
	void writeByte(unsigned char x) {
		b |= x << (8+s);
		fwrite(&b, sizeof(b), 1, fHan);
		b = (x >> (8-s));
	}
	void flush() {
		if (s)
			fwrite(&b, sizeof(b), 1, fHan);
	}
};

#endif
