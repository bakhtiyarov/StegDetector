#pragma once

#include <vector>
#include <cstdint>
#include <string>

struct RgbPixel
{
	uint8_t r, g, b;
};

struct BMPHEAD;

class UnsupportedImageFormatException : public std::exception
{
public:
	UnsupportedImageFormatException(const std::string& str);
};


class FileNameError : public std::exception
{
public:
	FileNameError(const char* err) : std::exception(err){}
};

struct BMPImage
{
	size_t width, height;
	//std::string filename;
	uint8_t* planes[3];//R,G,B;

	size_t getSize()const
	{
		return width * height * 3;
	}

	RgbPixel getPixel(size_t x, size_t y)const;
	void setPixel(size_t x, size_t y, RgbPixel p);

	explicit BMPImage(const char* filename);
	virtual ~BMPImage();

	void save(const char* fileName);

private:
	BMPHEAD* head;
};