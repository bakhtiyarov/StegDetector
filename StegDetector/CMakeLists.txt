cmake_minimum_required(VERSION 2.6)
project(StegDetector)

find_package(Boost REQUIRED program_options)
include_directories(${Boost_INCLUDE_DIR} ../ImageWrapper)

find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(SOURCE_EXE LSBDetection.cpp)

link_directories(${IMAGEWRAPPER_PATH} ${Boost_LIBRARY_DIRS})
add_executable(StegDetector ${SOURCE_EXE})

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++11")
endif()

set(IMAGEWRAPPER_PATH "../ImageWrapper")

target_link_libraries(StegDetector ImageWrapper ${Boost_LIBRARIES})
