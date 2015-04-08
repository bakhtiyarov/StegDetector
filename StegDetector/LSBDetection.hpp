#pragma once

/* Algorithm to detect LSB steganography in BMP file using image statistics; 
see Ankit Gupta, Rahul Garg - "Detecting LSB Steganography in Images" for algorithm details;
*/


#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include "BMPImage.hpp"



using KeyPair = std::pair < uint64_t, uint64_t > ;
struct KeyTuple
{
	uint64_t x, y, w, h, k;
};

using bitMap = std::vector < std::vector<bool> > ;

enum class LsbMode
{
	Simple,
	Shuffle
};



template<typename FwdIt> int64_t sum(FwdIt First, FwdIt Last)
{
	int64_t result = 0;
	while (First != Last)
	{
		result += *First;
		First++;
	}
	return result;
}



uint8_t GetLeastBits(uint8_t c, size_t k);



uint8_t GetMostBits(uint8_t c, size_t k);



bool scoreFunction(uint8_t p, uint8_t top, uint8_t left, size_t k);


//TODO: create multithread version of this function!
bitMap calculateBitmap(const BMPImage& img, size_t k);



KeyPair evaluateXW(const std::vector<uint32_t>& Fx);


void findKey(const BMPImage& img);