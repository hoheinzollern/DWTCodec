#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <string>

using namespace std;

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

class Bitmap {
private:
	bmpfile_header header;
	bmp_dib_v3_header_t v3_header;
	unsigned char fake[2048];
	unsigned char *_payload;
public:
	Bitmap();
	Bitmap(int width, int height);
	Bitmap(int width, int height, unsigned char *payload);
	void readImage(const string &fileName);
	void writeImage(const string &fileName);
	int width();
	int height();
	unsigned char *payload();
	unsigned char *getPadded();
	int getPadWidth();
	int getPadHeight();
};

#endif
