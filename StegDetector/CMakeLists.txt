cmake_minimum_required(VERSION 2.6)
project(StegDetector)

find_package(Boost REQUIRED program_options)
include_directories(${Boost_INCLUDE_DIR} ../ImageWrapper)


set(SOURCE_EXE LSBDetection.cpp)

link_directories(${IMAGEWRAPPER_PATH} ${Boost_LIBRARY_DIRS})
add_executable(StegDetector ${SOURCE_EXE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(IMAGEWRAPPER_PATH "../ImageWrapper")

target_link_libraries(StegDetector ImageWrapper)
