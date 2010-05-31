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
public:
	DWT(Bitmap *input);
	void compress(int threshold);
	void save(const string &fileName);
};

#endif
