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
	in >> h.Signature >> h.FileLength >> h.Zero >> h.Ptr >> h.Version >> h.Width >> h.Height >> h.Planes >> h.BitsPerPixel >>
		h.Compression >> h.SizeImage >> h.XPelsPerMeter >> h.YPelsPerMeter >> h.ClrUsed >> h.ClrImportant;

	return in;
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
	ifstream in(filename);
	if (in.is_open())
	{
		head = new BMPHEAD;
		in >> *head;
		in.seekg(head->Ptr);

		if (head->Planes != 3)
			throw UnsupportedImageFormatException("Image have unupported count of bit planes; may be this is grayscale image");
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
				in >> planes[0][i*width + j];
				in >> planes[1][i*width + j];
				in >> planes[2][i*width + j];

				if (head->BitsPerPixel == 32)
				{
					uint8_t garbage;
					in >> garbage;
				}
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


BMPImage::~BMPImage()
{
	delete head;
	delete planes[0];
	delete planes[1];
	delete planes[2];
}


UnsupportedImageFormatException::UnsupportedImageFormatException(const std::string& str)
	:exception(str.c_str())
{}