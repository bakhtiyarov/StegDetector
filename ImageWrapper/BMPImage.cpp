#include "BMPImage.hpp"
#include <fstream>

using namespace std;


BMPHEAD::BMPHEAD()
{
	Signature = 0x4d42;
	Zero = 0;
	Ptr = 54;
	Version = 0x28;
	Planes = 1;
	BitsPerPixel = 24;
	Compression = 0;
	XPelsPerMeter = YPelsPerMeter = 0;//???
	ClrUsed = ClrImportant = 0;
}

std::istream& operator>>(std::istream& in, BMPHEAD& h)
{
	in.read((char*)&h.Signature, sizeof(h.Signature));
	in.read((char*)&h.FileLength, sizeof(h.FileLength) * 6);
	in.read((char*)&h.Planes, sizeof(h.Planes));
	in.read((char*)&h.BitsPerPixel, sizeof(h.BitsPerPixel));
	in.read((char*)&h.Compression, sizeof(h.Compression) * 6);

	return in;
}

std::ostream& operator<<(std::ostream& out, BMPHEAD& h)
{
	out.write((char*)&h.Signature, sizeof(h.Signature));
	out.write((char*)&h.FileLength, sizeof(h.FileLength) * 6);
	out.write((char*)&h.Planes, sizeof(h.Planes));
	out.write((char*)&h.BitsPerPixel, sizeof(h.BitsPerPixel));
	out.write((char*)&h.Compression, sizeof(h.Compression) * 6);

	return out;
}


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





BMPImage::BMPImage(const char* filename)
{
	ifstream in;
	in.open(filename, std::ifstream::binary);
	if (in.is_open() && !in.fail())
	{
		head = new BMPHEAD;
		in >> *head;
		in.seekg(0);

		arr = new uint8_t[head->FileLength];
		in.read((char*)arr, head->FileLength);

		if (head->BitsPerPixel != 24 && head->BitsPerPixel != 32)
			throw UnsupportedImageFormatException("Image have unsupported bit depth");

		width = head->Width;
		height = head->Height;
		planes[0] = new uint8_t[width * height];
		planes[1] = new uint8_t[width * height];
		planes[2] = new uint8_t[width * height];

	
		if (head->BitsPerPixel == 24)
		{
			size_t rowlen = head->Width * 3;
			while (rowlen % 4)
				rowlen++;

			uint8_t* ptr = arr + head->Ptr;
			for (size_t i = 0; i != height; i++)
			{
				for (size_t j = 0; j != width; j++)
				{
					planes[0][i*width + j] = ptr[ i*rowlen + 3 * j ];
					planes[1][i*width + j] = ptr[i*rowlen + 3 * j + 1];
					planes[2][i*width + j] = ptr[i*rowlen + 3 * j + 2];
				}
			}
		}
		else if (head->BitsPerPixel == 32)
		{
			uint8_t* ptr = arr + head->Ptr;
			for (size_t i = 0; i != width*height; i++)
			{
				planes[0][i] = ptr[4 * i];
				planes[1][i] = ptr[4 * i + 1];
				planes[2][i] = ptr[4 * i + 1];
			}
		}
		if (!head->SizeImage)
		{
			head->SizeImage = head->FileLength - head->Ptr;
		}
	}
	else
	{
		std::string err = "Failed to open file ";
		err.append(filename);
		throw FileNameError(err.c_str());
	}
}

BMPImage::BMPImage()
	:width(0), height(0), head(nullptr), arr(nullptr)
{
	planes[0] = planes[1] = planes[2] = nullptr;
}


void BMPImage::save(const char* fileName)
{
	if (!head)
	{
		head = new BMPHEAD;
		head->Height = height;
		head->Width = width;
		head->SizeImage = width*height * 3;
		head->FileLength = head->SizeImage + 54;
		arr = new uint8_t[head->FileLength];
	}
	size_t rowlen;
	if (head->BitsPerPixel == 24)
	{
		rowlen = width*3;
		while (rowlen % 4)
			rowlen++;

		if (rowlen != width*3)
		{
			delete arr;
			arr = nullptr;
			head->FileLength = height*rowlen + head->Ptr;
			head->SizeImage = height*rowlen;
		}
	}
	if (head->BitsPerPixel == 32)
	{
		rowlen = width*4;
	}
	if (!arr)
		arr = new uint8_t[head->FileLength];

	uint8_t* ptr = arr;
	memcpy(ptr, &head->Signature, sizeof(head->Signature));
	ptr += sizeof(head->Signature);
	memcpy(ptr, &head->FileLength, sizeof(head->FileLength) * 6);
	ptr += sizeof(head->FileLength) * 6;
	memcpy(ptr, &head->Planes, sizeof(head->Planes) * 2);
	ptr += sizeof(head->Planes) * 2;
	memcpy(ptr, &head->Compression, sizeof(head->Compression) * 6);


	ofstream out(fileName, std::ofstream::out | std::ofstream::binary);
	if (!out.is_open())
	{
		std::string err = "Failed to open file ";
		err.append(fileName);
		err.append(" for writing!");
		throw FileNameError(err.c_str());
	}

	if (head->BitsPerPixel == 24)
	{
		ptr = arr + head->Ptr;
		for (size_t i = 0; i != height; i++)
		{
			for (size_t j = 0; j != width; j++)
			{
				ptr[i*rowlen + 3 * j] = planes[0][i*width + j];
				ptr[i*rowlen + 3 * j + 1] = planes[1][i*width + j];
				ptr[i*rowlen + 3 * j + 2] = planes[2][i*width + j];
			}
		}
	}
	else if (head->BitsPerPixel == 32)
	{
		uint8_t* ptr = arr + head->Ptr;
		for (size_t i = 0; i != width*height; i++)
		{
			ptr[4 * i] = planes[0][i];
			ptr[4 * i + 1] = planes[1][i];
			ptr[4 * i + 1] = planes[2][i];
		}
	}

	out.write((char*)arr, head->FileLength);
}


BMPImage::~BMPImage()
{
	delete head;
	delete[] planes[0];
	delete[] planes[1];
	delete[] planes[2];
	delete[] arr;
}


UnsupportedImageFormatException::UnsupportedImageFormatException(const std::string& str)
	:exception(), errStr(str)
{}

const char* UnsupportedImageFormatException::what()
{
	return errStr.c_str();
}
