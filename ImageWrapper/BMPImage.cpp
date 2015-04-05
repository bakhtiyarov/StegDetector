#include "BMPImage.hpp"
#include <fstream>

using namespace std;

struct BMPHEAD
{
	uint16_t Signature;         // Must be 0x4d42 == ”BM”              //0
	uint32_t FileLength;             // в байтах                                          //2
	uint32_t Zero;                       // Must be 0                                          //6
	uint32_t Ptr;                          // смещение к области данных         //10
	uint32_t Version;// длина оставшейся части заголовка=0x28    //14
	uint32_t Width;         // ширина изображения в пикселах             //18
	uint32_t Height;        // высота изображения в пикселах                         //22
	uint16_t   Planes;            // к-во битовых плоскостей             //26
	uint16_t   BitsPerPixel;  // к-во бит на папиксел         //28
	uint32_t Compression;          // сжатие: 0 или 1 или 2                    //30
	uint32_t SizeImage;              // размер блока данных в байтах     //34
	uint32_t XPelsPerMeter;      // в ширину: пикселов на метр         //38
	uint32_t YPelsPerMeter;       // в высчоту: пикселов на метр       //42
	uint32_t ClrUsed;                  // к-во цветов в палитре                  //46
	uint32_t ClrImportant; // к-во используемых цветов в палитре //50

	//uint8_t pal[256][4];
};


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
		in.seekg(head->Ptr);

		if (head->BitsPerPixel != 24 && head->BitsPerPixel != 32)
			throw UnsupportedImageFormatException("Image have unsupported bit depth");

		width = head->Width;
		height = head->Height;
		planes[0] = new uint8_t[width * height];
		planes[1] = new uint8_t[width * height];
		planes[2] = new uint8_t[width * height];

		for (size_t i = 0; i != head->Height; i++)
		{
			for (size_t j = 0; j != head->Width; j++)
			{
				in.read((char*)&planes[0][i*width + j], 1);
				in.read((char*)&planes[1][i*width + j], 1);
				in.read((char*)&planes[2][i*width + j], 1);

				if (head->BitsPerPixel == 32)
				{
					uint8_t garbage;
					in.read((char*)&garbage, 1);
				}
			}
			if (head->BitsPerPixel != 32)
			{
				uint8_t garbage[3];
				uint8_t tailLen= (width * head->BitsPerPixel) % 32;
				in.read((char*)garbage, tailLen);
			}
		}
	}
	else
	{
		std::string err = "Failed to open file ";
		err.append(filename);
		throw FileNameError(err.c_str());
	}
}


void BMPImage::save(const char* fileName)
{
	ofstream out(fileName);
	if (!out.is_open())
	{
		std::string err = "Failed to open file ";
		err.append(fileName);
		err.append(" for writing!");
		throw FileNameError(err.c_str());
	}

	head->Ptr = 55;
	out << *head;
	for (size_t i = 0; i != height; i++)
	{
		for (size_t j = 0; j != width; j++)
		{
			out.write((char*)&planes[0][i*width + j], 1);
			out.write((char*)&planes[1][i*width + j], 1);
			out.write((char*)&planes[2][i*width + j], 1);
			
			if (head->BitsPerPixel == 32)
			{
				char a = 0;
				out.write(&a, 1);
			}
		}
		if (head->BitsPerPixel != 32)
		{
			uint8_t garbage[3];
			std::fill_n(garbage, 3, 0);
			uint8_t tailLen = (width * head->BitsPerPixel) % 32;
			out.write((char*)garbage, tailLen);
		}
		
	}
}


BMPImage::~BMPImage()
{
	delete head;
	delete planes[0];
	delete planes[1];
	delete planes[2];
}


UnsupportedImageFormatException::UnsupportedImageFormatException(const std::string& str)
	:exception(), errStr(str)
{}

const char* UnsupportedImageFormatException::what()
{
	return errStr.c_str();
}
