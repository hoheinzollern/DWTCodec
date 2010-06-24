#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <string>
#include <cstdio>

#define DIM_HEAD_BMP 1078

using namespace std;

/**
  * Structures used to represent bitmap headers borrowed from Wikipedia:
  * http://en.wikipedia.org/wiki/BMP_file_format
  */
struct bmpfile_magic {
  unsigned char magic[2];
};

struct bmpfile_header {
  uint32_t filesz;
  uint16_t creator1;
  uint16_t creator2;
  uint32_t bmp_offset;
};

typedef struct {
  uint32_t header_sz;
  uint32_t width;
  uint32_t height;
  uint16_t nplanes;
  uint16_t bitspp;
  uint32_t compress_type;
  uint32_t bmp_bytesz;
  uint32_t hres;
  uint32_t vres;
  uint32_t ncolors;
  uint32_t nimpcolors;
} bmp_dib_v3_header_t;

uint16_t swap_endian16(uint16_t src);
uint32_t swap_endian32(uint32_t src);

/**
  * Bitmap class, used to read, create and save 8bpp grayscale images
  */
class Bitmap {
private:
	static unsigned char palette[256*4];
	static bool palette_initialized;

	bmpfile_header header;
	bmp_dib_v3_header_t v3_header;
	unsigned char *payload;

	int padding();
public:
	Bitmap();
	Bitmap(int width, int height);
	Bitmap(int width, int height, unsigned char *payload);
	~Bitmap();

	void readImage(FILE *);
	void readImage(const string &fileName);

	void writeImage(FILE *);
	void writeImage(const string &fileName);

	int width();
	int height();
	void set(int pos, unsigned char val);

	float *getPadded(int, int);

	int getPadWidth();
	int getPadHeight();
};

#endif
