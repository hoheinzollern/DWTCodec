cmake_minimum_required(VERSION 2.8)

project(DWTCodec)

add_definitions(-Wall -g -fomit-frame-pointer)

add_executable(dwtcodec main.cpp Bitmap.cpp DWT.cpp Huffman.cpp bitfile.cpp)
