PROJECT(DWTCodec)

add_definitions(-Wall -g -fomit-frame-pointer)

ADD_EXECUTABLE(dwtcodec main.cpp Bitmap.cpp DWT.cpp)
