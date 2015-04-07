cmake_minimum_required(VERSION 2.6)
project(HideInformation)

find_package(Boost REQUIRED program_options)
include_directories(${Boost_INCLUDE_DIR} ../ImageWrapper)

find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(SOURCE_EXE LSB.cpp)

link_directories(${IMAGEWRAPPER_PATH} ${Boost_LIBRARY_DIRS})
add_executable(HideInformation ${SOURCE_EXE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(IMAGEWRAPPER_PATH "../ImageWrapper")

target_link_libraries(HideInformation ImageWrapper)
