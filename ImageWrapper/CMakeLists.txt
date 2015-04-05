cmake_minimum_required(VERSION 2.8)
project(ImageWrapper)

set(SOURCE_LIB BMPImage.cpp)

add_library(ImageWrapper STATIC ${SOURCE_LIB})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
