#include "LSBDetection.hpp"

using namespace std;

vector<bool> GetLeastBits(uint8_t c, size_t k)
{
	std::vector<bool> result;
	result.resize(k);
	for (uint8_t i = 0; i != k; i++)
		result[i] = c & (1 << i);
	return result;
}

vector<bool> GetMostBits(uint8_t c, size_t k)
{
	std::vector<bool> result;
	result.resize(k);
	for (uint8_t i = 0; i != k; i++)
		result[i] = c & (0x80 >> i);
	return result;
}


bool scoreFunction(uint8_t p, uint8_t top, uint8_t left, size_t k)
{
	return (GetLeastBits(p,k) == GetLeastBits(left,k)) &
		(GetLeastBits(p,k) == GetLeastBits(top,k)) &
		(GetMostBits(p,8-k) == GetMostBits(left,8-k)) &
		(GetMostBits(p,8-k) == GetMostBits(top,8-k));
}

bitMap calculateBitmap(const BMPImage& img, size_t k)
{
	bitMap result;
	result.resize(img.width);
	for (auto x = result.begin(); x != result.end(); x++)
		x->resize(img.height);

	for (size_t i = 1; i != img.width; i++)
	{
		for (size_t j = 1; j != img.height; j++)
		{
			RgbPixel pixel = img.getPixel(i, j);
			RgbPixel left = img.getPixel(i - 1, j);
			RgbPixel top = img.getPixel(i, j - 1);

			bool res = scoreFunction(pixel.r, top.r, left.r, k) &
				scoreFunction(pixel.g, top.g, left.g, k) &
				scoreFunction(pixel.b, top.b, left.b, k);
			result[i][j] = res;
		}
	}

	return result;
}


KeyPair evaluateXW(const std::vector<uint32_t>& Fx)
{
	uint32_t curMax = 0;
	size_t imax, jmax;

	for (size_t i = 0; i != Fx.size(); i++)
	{
		for (size_t j = i + 1; j != Fx.size(); j++)
		{
			uint32_t res = sum(Fx.cbegin() + i, Fx.cbegin() + j + 1);
			if (res > curMax)
			{
				curMax = res;
				imax = i;
				jmax = j;
			}
		}
	}

	KeyPair r;
	r.first = imax;
	r.second = jmax;
	return r;
}


void findKey(const BMPImage& img)
{
	std::vector<KeyTuple> keys;
	for (size_t k = 1; k != 5; k++)
	{
		bitMap S = calculateBitmap(img, k);

		std::vector<uint32_t> Fx, Fy;
		Fx.resize(img.height);
		Fy.resize(img.width);
		std::fill(Fx.begin(), Fx.end(), 0);
		std::fill(Fy.begin(), Fy.end(), 0);

		for (size_t i = 0; i != img.width; i++)
		{
			for (size_t j = 0; j != img.height; j++)
			{
				Fx[i] += S[i][j];
				Fy[j] += S[i][j];
			}
		}


		uint32_t xmean, ymean;
		xmean = ymean = 0;

		for (size_t i = 0; i != Fx.size(); i++)
			xmean += Fx[i];
		xmean /= Fx.size();

		for (size_t i = 0; i != Fy.size(); i++)
			ymean += Fy[i];
		ymean /= Fy.size();


		for (auto x = Fx.begin(); x != Fx.end(); x++)
		{
			x -= xmean;
		}

		for (auto y = Fy.begin(); y != Fy.end(); y++)
		{
			y -= ymean;
		}


		KeyPair xw = evaluateXW(Fx);
		KeyPair yh = evaluateXW(Fy);
		KeyTuple curKey;
		curKey.x = xw.first;
		curKey.y = yh.first;
		curKey.w = xw.second;
		curKey.h = yh.second;
		curKey.k = k;

		keys.push_back(curKey);
	}



}