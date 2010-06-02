cmake_minimum_required(VERSION 2.8)

project(DWTCodec)

find_library(LZMA REQUIRED)

add_definitions(-Wall -g -fomit-frame-pointer)

add_executable(dwtcodec main.cpp Bitmap.cpp DWT.cpp BitWriter.cpp BitReader.cpp)
