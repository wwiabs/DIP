#include "image.h"
#include "bitmap_modified.h"

Image::Image()
/*:m_width(0), m_height(0), m_data(NULL)*/  {}

Image::Image(const char* filename)
{
	const char* ext = strrchr(filename, '.');
	if (ext == NULL)
		return;
	ext++;
	if (strcmp(ext, "bmp") == 0 || strcmp(ext, "BMP") == 0)//bmp
	{
		CBitmap bitmap;
		if (bitmap.Load(filename)){
			m_width = bitmap.GetWidth();
			m_height = bitmap.GetHeight();
			m_bpp = bitmap.GetBitCount();
			m_channel = m_bpp / 8;
			m_stepsize = m_width * m_channel;
			uint size = bitmap.GetSize() * m_channel;
			m_data = new uchar[size];
			memcpy(m_data, bitmap.GetBits(), size);
		}
	}
	else if (strcmp(ext, "png") == 0 || strcmp(ext, "PNG") == 0)//png
	{

	}
}

Image::~Image()
{
	if (m_data != NULL){
		delete[] m_data;
		m_data = NULL;
	}
}
