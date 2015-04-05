cmake_minimum_required(VERSION 2.6)
project(StegDetector)

find_package(Boost REQUIRED program_options)
include_directories(${BOOST_INCLUDE_DIRS})

include_directories(../ImageWrapper)

set(SOURCE_EXE LSBDetection.cpp)

add_executable(StegDetector ${SOURCE_EXE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(IMAGEWRAPPER_PATH "../ImageWrapper")

link_directories(${IMAGEWRAPPER_PATH})
target_link_libraries(StegDetector ImageWrapper ${Boost_LIBRARIES})
