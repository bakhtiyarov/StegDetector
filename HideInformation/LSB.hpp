#pragma once

#include <vector>
#include <cstdint>
#include <string>

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


void setLSB(char& c, const std::vector<bool>& data);


//return @count bits beginning at (@data[@indexOfFirstBit/8] + @indexOfFirstBit%8)
std::vector<bool> getBits(uint8_t* data, uint64_t indexOfFirstBit, uint64_t count);



/* hide @secret image into @cover, using least @k significant bits and keys @x and @y;
*/
void createLSBImage(BMPImage& cover, const BMPImage& secret, const KeyTuple& key, LsbMode = LsbMode::Simple);
