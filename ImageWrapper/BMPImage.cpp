#include "BMPImage.hpp"
#include <fstream>

using namespace std;

struct BMPHEAD
{
	uint16_t Signature;         // Must be 0x4d42 == �BM�              //0
	uint32_t FileLength;             // � ������                                          //2
	uint32_t Zero;                       // Must be 0                                          //6
	uint32_t Ptr;                          // �������� � ������� ������         //10
	uint32_t Version;// ����� ���������� ����� ���������=0x28    //14
	uint32_t Width;         // ������ ����������� � ��������             //18
	uint32_t Height;        // ������ ����������� � ��������                         //22
	uint16_t   Planes;            // �-�� ������� ����������             //26
	uint16_t   BitsPerPixel;  // �-�� ��� �� ��������         //28
	uint32_t Compression;          // ������: 0 ��� 1 ��� 2                    //30
	uint32_t SizeImage;              // ������ ����� ������ � ������     //34
	uint32_t XPelsPerMeter;      // � ������: �������� �� ����         //38
	uint32_t YPelsPerMeter;       // � �������: �������� �� ����       //42
	uint32_t ClrUsed;                  // �-�� ������ � �������                  //46
	uint32_t ClrImportant; // �-�� ������������ ������ � ������� //50

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

			size_t rowlen = (head->Width * 3) % 4;
			rowlen += head->Width * 3;

			uint8_t* ptr = arr + head->Ptr;
			for (size_t i = 0; i != height; i++)
			{
				for (size_t j = 0; j != width; j++)
				{
					planes[0][i*width + j] = ptr[ i*rowlen + j ];
					planes[1][i*width + j] = ptr[i*rowlen + j + 1];
					planes[2][i*width + j] = ptr[i*rowlen + j + 2];
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
		size_t rowlen = (head->Width * 3) % 4;
		rowlen += head->Width * 3;

		uint8_t* ptr = arr + head->Ptr;
		for (size_t i = 0; i != height; i++)
		{
			for (size_t j = 0; j != width; j++)
			{
				ptr[i*rowlen + j] = planes[0][i*width + j];
				ptr[i*rowlen + j + 1] = planes[1][i*width + j];
				ptr[i*rowlen + j + 2] = planes[2][i*width + j];
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
