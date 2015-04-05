#include "LSB.hpp"
#include <cassert>
#include <boost/program_options.hpp>
#include <iostream>
#include <algorithm>

namespace opts = boost::program_options;

using namespace std;







double calculateFactor(size_t width1, size_t height1, size_t width2, size_t height2, uint8_t k)
{
	size_t available_sz = width1*height1*k;
	size_t need_sz = width2*height2 * 8;
	return need_sz / available_sz;
}




void setLSB(char& c, const vector<bool>& data)
{
	assert(data.size() < 8);

	uint8_t k = data.size();
	
	for (size_t i = 0; i != k; i++)
	{
		c &= 0xFE << (k - i - 1);
		c |= data[i] << (k - i - 1);
	}
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

	if (indexOfFirstBit + count > 8)
	{
		for (int i = indexOfFirstBit; i != 8; i++)
			result.push_back(data[0] & (0x80 >> i));

		const vector<bool> tail = getBits(data + 1, 0, count - 8 + indexOfFirstBit);
		std::copy(tail.cbegin(), tail.cend(), std::back_inserter(result));
	}
	else
	{
		for (int i = indexOfFirstBit; i != std::min<size_t>(8, count + indexOfFirstBit); i++)
			result.push_back(data[0] & (0x80 >> i));
	}
	
	return result;
}



void createLSBImage(BMPImage& cover, const BMPImage& secret, const KeyTuple& key, LsbMode /* = LsbMode::Simple */)
{
	uint64_t curSecretBit = 0;

	for (size_t y = key.y; y != key.h; y++)
	{
		if (curSecretBit * 3 / 8 >= secret.getSize())
			break;
		for (size_t x = key.x; x != key.w; x++)
		{
			if (curSecretBit * 3 / 8 >= secret.getSize())
				break;

			vector<bool> next1, next2, next3;
			next1 = getBits(secret.planes[0], curSecretBit, key.k);
			next2 = getBits(secret.planes[1], curSecretBit, key.k);
			next3 = getBits(secret.planes[2], curSecretBit, key.k);
			curSecretBit += key.k;

			RgbPixel curPixel = cover.getPixel(x, y);

			setLSB((char&)curPixel.r, next1);
			setLSB((char&)curPixel.g, next2);
			setLSB((char&)curPixel.b, next3);

			cover.setPixel(x, y, curPixel);
		}
	}
}



int main(int argc, char** argv)
{
	std::string contName, secretName, outName;
	KeyTuple key;


	try
	{
		opts::options_description desc("Allowed options");
		desc.add_options()
			("help", "show this help message")
			("secret,s", opts::value<string>(&secretName), "Image which will be hiden in the container")
			("cont,c", opts::value<string>(&contName), "Image which will be used as container for hiding")
			("left,x", opts::value<size_t>(&key.x)->default_value(0), "The low bound of hiding area by X axis")
			("right,w", opts::value<size_t>(&key.w)->default_value(0), "The high bound of hiding area by X axis")
			("top,y", opts::value<size_t>(&key.y)->default_value(0), "The low bound of hiding area by Y axis")
			("bottom,h", opts::value<size_t>(&key.h)->default_value(0), "The high bound of hiding area by Y axis")
			("LSB-bits,k", opts::value<size_t>(&key.k)->default_value(2), "Count of least significant bits used for message hiding")
			("output,o", opts::value<string>(&outName), "File name for result");

		opts::variables_map vm;
		opts::store(opts::parse_command_line(argc, argv, desc), vm);
		opts::notify(vm);

		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}
		if (!vm.count("secret"))
		{
			cout << "You should pass name of secret image!" << std::endl << desc << std::endl;
			return -1;
		}
		if (!vm.count("cont"))
		{
			cout << "You should pass name of container image!" << std::endl << desc << std::endl;
			return -1;
		}
		if (!vm.count("output"))
		{
			cout << "You should pass name of output image!" << std::endl << desc << std::endl;
			return -1;
		}


		BMPImage secret(secretName.c_str());
		BMPImage cont(contName.c_str());

		if (!key.h)
			key.h = cont.height;
		if (!key.w)
			key.w = cont.width;

		createLSBImage(cont, secret, key);
		cont.save(outName.c_str());
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}



	return 0;
}
