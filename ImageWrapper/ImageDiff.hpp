#pragma once

#include "BMPImage.hpp"
#include <cmath>


double BMPdiff(uint8_t* planes1[3], uint32_t width, uint32_t height, uint8_t* planes2[3]);