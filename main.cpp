#include "Bitmap.h"
#include "DWT.h"
#include "lib/getopt_pp.h"

#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;
using namespace GetOpt;

int show_help()
{
	cout	<< "DWTCodec: simple image codec using Discrete Wavelet Transform, Huffman+RLE encoding"
			<< endl << endl
			<< "Usage: dwtcodec [-t <thresold>] [-b <bpp>] {-c|-d} input output" << endl
			<< "   -t --thresold     Set a thresold for coefficients (defaults to 5.0)" << endl
			<< "   -b --bpp          Bits per uncompressed pixels (defaults to 12)" << endl
			<< "   -c --compress     Compress Bitmap input into DWT output" << endl
			<< "   -d --decompress   Decompress DWT input into Bitmap output" << endl
			<< "   -h --help         Show this help message" << endl;
	return 0;
}

int main(int argc, char **argv)
{
	GetOpt_pp opt(argc, argv);
	bool compress = false, decompress = false;
	float thresold = 5.0f;
	unsigned int bpp = 12;
	opt >> Option('t', "thresold", thresold);
	opt >> Option('b', "bpp", bpp);
	opt >> OptionPresent('c', "compress", compress);
	opt >> OptionPresent('d', "decompress", decompress);

	if (argc < 3 || opt >> OptionPresent('h', "help") || (!compress && !decompress))
		return show_help();
	string input(argv[argc-2]), output(argv[argc-1]);

	if (bpp > 16 || bpp < 8) {
		cerr << "Invalid quantization value" << endl;
		return 1;
	}
	if (thresold < 0.0f || thresold > 64.0f) {
		cerr << "Invalid thresold, must be >= 0 and <= 64" << endl;
		return 1;
	}

	if (compress) {
		Bitmap *src = new Bitmap();
		src->readImage(input);
		DWT *dwt = new DWT(src, bpp);
		dwt->transform();
		dwt->compress(thresold);
		dwt->save(output);
		delete src;
		delete dwt;
	} else if (decompress) {
		DWT *dwt = new DWT(bpp);
		dwt->load(input);
		dwt->untrasform();
		Bitmap *out = dwt->toBitmap();
		out->writeImage(output);
		delete dwt;
		delete out;
	}
	return 0;
}
