#include <iostream>
#include "Bitmap.h"
#include "DWT.h"
#include <cstring>

using namespace std;

int main(int argc, char **argv) {
	if (strcmp(argv[1], "-c") == 0) {
		Bitmap *src = new Bitmap();
		src->readImage(string(argv[2]));
		DWT *dwt = new DWT(src);
		dwt->transform();
		dwt->compress(5);
		dwt->saveDWT(string(argv[3]));
	} else if (strcmp(argv[1], "-d") == 0) {
		DWT *dwt = new DWT();
		dwt->loadDWT(string(argv[2]));
		dwt->untrasform();
		dwt->saveBMP(string(argv[3]));
	}
	return 0;
}
