#ifndef DWT_H
#define DWT_H

#include "Bitmap.h"
#include <string>
using namespace std;

class DWT {
private:
	Bitmap *input;
	Bitmap *output;
	float *W;
	int width;
	int height;
	int realWidth;
	int realHeight;
public:
	DWT();
	DWT(Bitmap *input);
	void compress(int threshold);
	void save(const string &fileName);
	void saveDWT(const string &fileName);
	void readDWT(const string &fileName);
};

#endif
