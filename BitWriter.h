#ifndef BITWRITER_H
#define BITWRITER_H
#include <cstdio>

class BitWriter {
	FILE *fHan;
	int s;
	unsigned char b;
public:
	BitWriter(FILE *f): fHan(f), s(0), b(0) {}
	void writeBit(bool x);
	void writeByte(unsigned char x);
	void flush();
};

#endif
