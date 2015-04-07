#include "LSBDetection.hpp"
#include <boost/program_options.hpp>
#include <iostream>

using namespace std;
namespace opts = boost::program_options;


std::ostream& operator<<(std::ostream& out, const KeyTuple& t)
{
	out << "(x,y) = (" << t.x << "," << t.y << ")" << std::endl <<
		"(w,h) = (" << t.w << "," << t.h << ")" << std::endl <<
		"k = " << t.k << std::endl;

	return out;
}

inline uint8_t GetLeastBits(uint8_t c, size_t k)
{
	uint8_t result = 0;

	for (uint8_t i = 0; i != k; i++)
		result |= c & (1 << i);
	return result;
}

inline uint8_t GetMostBits(uint8_t c, size_t k)
{
	uint8_t result = 0;

	for (uint8_t i = 0; i != k; i++)
		result |= c & (0x80 >> i);
	return result;
}


bool scoreFunction(uint8_t p, uint8_t top, uint8_t left, size_t k)
{
	return (GetLeastBits(p,k) == GetLeastBits(left,k)) &
		(GetLeastBits(p,k) == GetLeastBits(top,k)) &
		(GetMostBits(p,8-k) == GetMostBits(left,8-k)) &
		(GetMostBits(p,8-k) == GetMostBits(top,8-k));
}

//TODO: I need parallel version of this function
bitMap calculateBitmap(const BMPImage& img, size_t k)
{
	bitMap result;
	result.resize(img.width);


	for (auto x = result.begin(); x != result.end(); x++)
		x->resize(img.height);

#pragma omp parallel for
	for (int64_t i = 1; i < img.width; i++)
	{
#pragma omp parallel for
		for (int64_t j = 1; j < img.height; j++)
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

#pragma omp parallel for
	for (int32_t i = 0; i < Fx.size()-1; i++)
	{
#pragma omp parallel for
		for (int32_t j = i + 1; j < Fx.size()-1; j++)
		{
			uint32_t res = 0;
			res = sum(Fx.data() + i, Fx.data() + j + 1);
			
#pragma omp critical
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
	std::vector<uint32_t> keyScores;
	for (size_t k = 1; k != 9; k++)
	{
		bitMap S = calculateBitmap(img, k);

		std::vector<uint32_t> Fx, Fy;
		Fx.resize(img.width);
		Fy.resize(img.height);
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
			if (*x >= xmean)
				*x -= xmean;
		}

		for (auto y = Fy.begin(); y != Fy.end(); y++)
		{
			if (*y >= ymean)
				*y -= ymean;
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
		uint32_t scoreX = sum(Fx.cbegin() + curKey.x, Fx.cbegin() + curKey.w + 1);
		uint32_t scoreY = sum(Fy.cbegin() + curKey.y, Fy.cbegin() + curKey.h + 1);

		uint32_t score = std::pow(2, k) * (scoreX + scoreY);
		keyScores.push_back(score);
	}


	for (size_t i = 0; i != keyScores.size(); i++)
	{
		std::cout << keys[i] << "and scores " << keyScores[i] / (keys[i].w * keys[i].h) << std::endl;
	}
}




int main(int argc, char** argv)
{
	std::string fName, outfName;

	try
	{
		opts::options_description desc("Allowed options");
		desc.add_options()
			("help", "show this help message")
			("input,i", opts::value<string>(&fName), "Image which will be checked")
			("output,o", opts::value<string>(&outfName), "In this image we save result of work");

		opts::variables_map vm;
		opts::store(opts::parse_command_line(argc, argv, desc), vm);
		opts::notify(vm);

		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}
		if (!vm.count("input"))
		{
			std::cout << "You should pass name of file to check!" << std::endl << desc << std::endl;
			return -1;
		}

		BMPImage img(fName.c_str());
		findKey(img);
	}
	catch (std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return -2;
	}

	return 0;
}