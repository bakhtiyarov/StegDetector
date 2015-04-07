#include "LSB.hpp"
#include <cassert>
#include <boost/program_options.hpp>
#include <iostream>
#include <algorithm>
#include <ctime>

namespace opts = boost::program_options;

using namespace std;







double calculateFactor(size_t width1, size_t height1, size_t width2, size_t height2, uint8_t k)
{
	size_t available_sz = width1*height1*k;
	size_t need_sz = width2*height2 * 8;
	return need_sz / available_sz;
}




void setLSB(uint8_t& c, const vector<bool>& data)
{
	assert(data.size() <= 8);

	uint8_t k = data.size();
	
	for (size_t i = 0; i != k; i++)
	{
		c &= 0xFE << (k - i - 1);
		c |= data[i] << (k - i - 1);
	}
}

void setMSB(uint8_t& c, const vector<bool>& data)
{
	uint8_t k = data.size();
	for (size_t i = 0; i != k; i++)
	{
		c &= 0x7F >> (k - i - 1);
		c |= data[k - i] << (8 - k + i);
	}
}


std::vector<bool> getLSB(char c, uint8_t k)
{
	std::vector<bool> result;
	for (uint8_t i = 0; i != k; i++)
		result.push_back(c &(1 << (k - i - 1)));

	return result;
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



void createLSBImage(BMPImage& cover, BMPImage& secret, const KeyTuple& key, LsbMode /* = LsbMode::Simple */)
{
	uint64_t curSecretBit = 0;

	//save dimensions of secret image into first bytes; TODO: may be should save all header of secret image?
	{
		uint32_t w = secret.width;
		uint32_t h = secret.height;
		uint8_t* wp = (uint8_t*)&w;
		uint8_t* hp = (uint8_t*)&h;

		secret.planes[0][0] = *wp++;
		secret.planes[1][0] = *wp++;
		secret.planes[2][0] = *wp++;
		secret.planes[0][1] = *wp;
		secret.planes[1][1] = *hp++;
		secret.planes[2][1] = *hp++;
		secret.planes[0][2] = *hp++;
		secret.planes[1][2] = *hp;
	}

	//TODO: change curSecretBit to f(x,y) and make this loop parallel
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

			setLSB(curPixel.r, next1);
			setLSB(curPixel.g, next2);
			setLSB(curPixel.b, next3);

			cover.setPixel(x, y, curPixel);
		}
	}
}


//get next byte encoded in LSB of plane; @rw and @rpixNum contain states between calls of this function;
static vector<bool> getNextByte(vector<bool> rw, size_t& rpixNum, uint8_t* plane, const KeyTuple& key)
{
	while (rw.size() < 8)
	{
		auto t = getLSB(plane[rpixNum], key.k);
		rw.insert(rw.end(), t.cbegin(), t.cend());
		rpixNum++;
	}
	vector<bool> nb(rw.cbegin(), rw.cbegin() + 8);
	rw.erase(rw.cbegin(), rw.cbegin() + 8);
	return nb;
}


//Extract dimensions of secret image from first LSB
static std::pair<uint32_t,uint32_t> getDimensions(const BMPImage& src, const KeyTuple& key)
{
	uint32_t width, height;
	uint8_t* p = (uint8_t*)&width;
	uint8_t* ph = (uint8_t*)&height;
	vector<bool> rw, gw, bw;
	size_t rpixNum, gpixNum, bpixNum; 
	rpixNum = gpixNum = bpixNum = 0;

	auto nb = getNextByte(rw, rpixNum, src.planes[0], key);
	setLSB(*p++, nb);
	nb = getNextByte(gw, gpixNum, src.planes[1], key);
	setLSB(*p++, nb);
	nb = getNextByte(bw, bpixNum, src.planes[2], key);
	setLSB(*p++, nb);
	nb = getNextByte(rw, rpixNum, src.planes[0], key);
	setLSB(*p++, nb);

	nb = getNextByte(gw, gpixNum, src.planes[1], key);
	setLSB(*ph++, nb);
	nb = getNextByte(bw, bpixNum, src.planes[2], key);
	setLSB(*ph++, nb);
	nb = getNextByte(rw, rpixNum, src.planes[0], key);
	setLSB(*ph++, nb);
	nb = getNextByte(gw, gpixNum, src.planes[1], key);
	setLSB(*ph++, nb);

	return std::pair<uint32_t, uint32_t>(width, height);
}


void extractLSBImage(const BMPImage& src, BMPImage& result, const KeyTuple& key, LsbMode /* = LSBMode::Simple*/)
{
	uint64_t curSecretBit = 0;

	uint32_t width, height;
	auto sz = getDimensions(src, key);
	width = sz.first;
	height = sz.second;


	result.planes[0] = new uint8_t[width * height];
	result.planes[1] = new uint8_t[width * height];
	result.planes[2] = new uint8_t[width * height];

	result.width = width;
	result.height = height;

	if (!result.head)
	{
		result.head = new BMPHEAD;
		result.head->FileLength = width*height * 3 + 54;
		result.head->Ptr = 54;
		result.head->SizeImage = width*height * 3;
		result.head->Width = width;
		result.head->Height = height;
	}


	if (!result.arr)
		result.arr = new uint8_t[result.head->FileLength];
	
	uint8_t* ptr = result.arr + result.head->Ptr;

	vector<bool> data[3];
	data[0].reserve(width * height * 8);
	data[1].reserve(width * height * 8);
	data[2].reserve(width * height * 8);


//#pragma omp parallel for schedule(static,1)
	for (int p = 0; p < 3; p++)
	{
		for (size_t i = 0; i < src.height; i++)
		{
			for (size_t j = 0; j < src.width; j++)	//TODO: process padding to 32-bit boundaries
			{
				auto d = getLSB(src.planes[p][i*src.width + j], key.k);
				data[p].insert(data[p].end(), d.cbegin(), d.cend());
			}
		}
	}

//#pragma omp parallel for schedule(static,1)
	for (int p = 0; p < 3; p++)
	{
//#pragma omp parallel for schedule(static,1)
		for (int64_t i = 0; i < height; i++)
		{
//#pragma omp parallel for schedule(static,1)
			for (int64_t j = 0; j < width; j++)
			{
				size_t pixIndex = i*width + j;
				vector<bool> nextByte(data[p].cbegin() + pixIndex * 8, data[p].cbegin() + (pixIndex + 1) * 8);
				setLSB(result.planes[p][pixIndex], nextByte);
			}
		}
	}
}



int main(int argc, char** argv)
{
	time_t start_time = time(0);
	std::string contName, secretName, outName, extractName;
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
			("output,o", opts::value<string>(&outName), "File name for result")
			("extract,e", opts::value<string>(&extractName), "Filename to extract secret information using key");

		opts::variables_map vm;
		opts::store(opts::parse_command_line(argc, argv, desc), vm);
		opts::notify(vm);

		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}
		if (vm.count("extract"))
		{
			if (!vm.count("LSB-bits"))
			{
				std::cout << "Error: You should pass count of LSB bits!" << desc;
				return -1;
			}
			if (!vm.count("output"))
			{
				std::cout << "Error: You should pass output file!" << desc;
				return -1;
			}

			BMPImage src(extractName.c_str());
			BMPImage res;

			extractLSBImage(src, res, key);
			res.save(outName.c_str());

		}
		else//hide information mode
		{
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
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}



	time_t end_time = time(0);

	std::cout << "Working time: " << end_time - start_time << std::endl;

	return 0;
}
