#include "LSB.hpp"
#include <cassert>

using namespace std;


RgbPixel BMPImage::getPixel(size_t x, size_t y)const
{
	RgbPixel result;
	result.r = planes[0][y*width + x];
	result.g = planes[1][y*width + x];
	result.b = planes[2][y*width + x];

	return result;
}


void BMPImage::setPixel(size_t x, size_t y, RgbPixel p)
{
	planes[0][y*width + x] = p.r;
	planes[1][y*width + x] = p.g;
	planes[2][y*width + x] = p.b;
}




double calculateFactor(size_t width1, size_t height1, size_t width2, size_t height2, uint8_t k)
{
	size_t available_sz = width1*height1*k;
	size_t need_sz = width2*height2 * 8;
	return need_sz / available_sz;
}




void setLSB(char& c, const vector<bool>& data)
{
	assert(data.size() < 8);

	c &= 0xf0;
	uint8_t k = data.size();
	for (size_t i = 0; i != k; i++)
		c |= (data[i] << (k - i));
}





vector<bool> getBits(uint8_t* data, uint64_t indexOfFirstBit, uint64_t count)
{
	if (indexOfFirstBit > 7)
	{
		uint8_t* newData = data + indexOfFirstBit / 8;
		return getBits(newData, indexOfFirstBit % 8, count);
	}


	vector<bool> result;
	result.reserve(count);
	for (int i = indexOfFirstBit; i != 8; i++)
		result.push_back(data[0] & (0x80 >> i));

	if (count > 8)
	{
		const vector<bool> tail = getBits(data + 1, 0, count - 8 + indexOfFirstBit);
		std::copy(tail.cbegin(), tail.cend(), std::back_inserter(result));
	}


	return result;
}



void createLSBImage(BMPImage& cover, const BMPImage& secret, const KeyTuple& key, LsbMode /* = LsbMode::Simple */)
{
	uint64_t curSecretBit = 0;

	for (size_t y = key.y; y != key.h; y++)
	{
		if (curSecretBit / 8 >= secret.getSize())
			break;
		for (size_t x = key.x; x != key.w; x++)
		{
			if (curSecretBit / 8 >= secret.getSize())
				break;

			vector<bool> next1, next2, next3;
			next1 = getBits(secret.planes[0], curSecretBit, key.k);
			curSecretBit += key.k;
			next2 = getBits(secret.planes[1], curSecretBit, key.k);
			curSecretBit += key.k;
			next3 = getBits(secret.planes[2], curSecretBit, key.k);
			curSecretBit += k;

			RgbPixel curPixel = cover.getPixel(x, y);

			setLSB((char&)curPixel.r, next1);
			setLSB((char&)curPixel.g, next2);
			setLSB((char&)curPixel.b, next3);

			cover.setPixel(x, y, curPixel);
		}
	}
}