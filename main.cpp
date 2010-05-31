#include <iostream>
#include "Bitmap.h"
#include "DWT.h"

using namespace std;

int main(int argc, char **argv) {
	Bitmap *src = new Bitmap();
	src->readImage(string(argv[1]));
	DWT *dwt = new DWT(src);
	//dwt->compress(32);
	dwt->save(string(argv[2]));
	return 0;
}
