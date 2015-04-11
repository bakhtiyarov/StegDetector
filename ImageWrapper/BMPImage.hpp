#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>

struct RgbPixel
{
	uint8_t r, g, b;
};

struct BMPHEAD
{
	uint16_t Signature;         // Must be 0x4d42 == ”BM”
	uint32_t FileLength;
	uint32_t Zero;
	uint32_t Ptr;
	uint32_t Version;			//Must be 0x28
	uint32_t Width;
	uint32_t Height;
	uint16_t   Planes;
	uint16_t   BitsPerPixel;
	uint32_t Compression;
	uint32_t SizeImage;
	uint32_t XPelsPerMeter;
	uint32_t YPelsPerMeter;
	uint32_t ClrUsed;
	uint32_t ClrImportant;

	BMPHEAD();
};

class UnsupportedImageFormatException : public std::exception
{
	std::string errStr;
public:
	UnsupportedImageFormatException(const std::string& str);
	virtual const char* what();
};


class FileNameError : public std::exception
{
	std::string errStr;
public:
	FileNameError(const char* err) : std::exception(), errStr(err){}
	virtual const char* what()
	{return errStr.c_str();}
};

struct BMPImage
{
	uint32_t width, height;
	//std::string filename;
	uint8_t* planes[3];//R,G,B;

	size_t getSize()const
	{
		return width * height * 3;
	}

	RgbPixel getPixel(size_t x, size_t y)const;
	void setPixel(size_t x, size_t y, RgbPixel p);

	explicit BMPImage(const char* filename);
	BMPImage();
	virtual ~BMPImage();

	void save(const char* fileName);

	BMPHEAD* head;
	uint8_t* arr;
};
