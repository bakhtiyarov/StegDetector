#pragma once

/* Algorithm to detect LSB steganography in BMP file using image statistics; 
see Ankit Gupta, Rahul Garg - "Detecting LSB Steganography in Images" for algorithm details;
*/


#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>



struct RgbPixel
{
	uint8_t r, g, b;
};

struct BMPImage
{
	size_t width, height;
	std::string filename;
	uint8_t* planes[3];//R,G,B;

	size_t getSize()const
	{
		return width * height * 3;
	}

	RgbPixel getPixel(size_t x, size_t y)const;
	void setPixel(size_t x, size_t y, RgbPixel p);
};


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



template<typename FwdIt> size_t sum(FwdIt First, FwdIt Last)
{
	size_t result = 0;
	while (First != Last)
	{
		result += *First;
		First++;
	}
	return result;
}



std::vector<bool> GetLeastBits(uint8_t c, size_t k);



std::vector<bool> GetMostBits(uint8_t c, size_t k);



bool scoreFunction(uint8_t p, uint8_t top, uint8_t left, size_t k);


//TODO: create multithread version of this function!
bitMap calculateBitmap(const BMPImage& img, size_t k);



KeyPair evaluateXW(const std::vector<uint32_t>& Fx);


void findKey(const BMPImage& img);