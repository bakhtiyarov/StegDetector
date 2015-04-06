#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "BMPImage.hpp"


struct KeyTuple
{
	size_t x, y, w, h, k;
};


using KeyPair = std::pair < size_t, size_t > ;



enum class LsbMode
{
	Simple,
	Shuffle
};



double calculateFactor(size_t width1, size_t height1, size_t width2, size_t height2, uint8_t k);


void setLSB(uint8_t& c, const std::vector<bool>& data);
void setMSB(uint8_t& c, const std::vector<bool>& data);


//return @count bits beginning at (@data[@indexOfFirstBit/8] + @indexOfFirstBit%8)
std::vector<bool> getBits(uint8_t* data, uint64_t indexOfFirstBit, uint64_t count);


/* hide @secret image into @cover, using least @k significant bits and keys @x and @y;
*/
void createLSBImage(BMPImage& cover, BMPImage& secret, const KeyTuple& key, LsbMode = LsbMode::Simple);

void extractLSBImage(const BMPImage& src, BMPImage& result, const KeyTuple& key, LsbMode  = LsbMode::Simple);
