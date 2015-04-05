#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>

struct RgbPixel
{
	uint8_t r, g, b;
};

struct BMPHEAD;

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
