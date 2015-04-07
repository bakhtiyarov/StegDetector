cmake_minimum_required(VERSION 2.8)
project(ImageWrapper)

set(SOURCE_LIB BMPImage.cpp ImageDiff.cpp)

add_library(ImageWrapper STATIC ${SOURCE_LIB})

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++11")
endif()
