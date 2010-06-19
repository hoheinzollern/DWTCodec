cmake_minimum_required(VERSION 2.6)

project(DWTCodec)

add_definitions(-Wall)

add_executable(dwtcodec main.cpp Bitmap.cpp DWT.cpp Huffman.cpp
	lib/bitfile.cpp lib/getopt_pp.cpp)
