#pragma once

#ifndef IMAGE_H_
#define IMAGE_H_

#include "basic_shape.h"
#include "bitmap.h"
//#include <iostream>
//#include "lpng1634/png.h"
typedef unsigned char uchar;

//#ifdef _DEBUG
//#pragma comment(lib,"./lpng1634/projects/vstudio/x64/Debug Library/libpng16.lib")
//#pragma comment(lib,"./lpng1634/projects/vstudio/x64/Debug Library/zlib.lib")
//#else
//#pragma comment(lib,"./lpng1634/projects/vstudio/x64/Release Library/libpng16.lib")
//#pragma comment(lib,"./lpng1634/projects/vstudio/x64/Release Library/zlib.lib")
//#endif
//
//#define PNG_BYTES_TO_CHECK 4

template <class T> class Image_
{
public:
	Image_();
	Image_(const char* filename);
	Image_(unsigned _width, unsigned _height, uchar _channel = 1, bool init_value = false, T _value = 0);
	Image_(uchar* p, unsigned _width, unsigned _height, uchar _channel = 1);
	Image_(const Image_& img, bool copy = true);    //初始化
	template<class T2> Image_(const Image_<T2>& img);   //类型转换
	void release();
	~Image_();
	Image_& operator = (const Image_& img);	//赋值浅拷贝
	bool open(const char* filename);
	bool save(const char* filename);
	T* ptr() const;
	T* ptr(unsigned row) const;
	T* ptr(unsigned row, unsigned col) const;
	T* ptr(unsigned row, unsigned col);
	T* ptr(const Point& pt) const;
	T* ptr(const Point& pt);
	Image_ roi(const Rect& r) const;
	void setroi(const Rect& r);

	unsigned width;
	unsigned height;
	unsigned stepsize;   //单位byte
	int* count;
	uchar* start;        //所有数据的启始指针
	uchar* data;         //当前图像启始指针
	uchar bpp;
	uchar channel;
};

typedef Image_<uchar> Image;
typedef Image_<double> Image2d;

template <class T> class Region_
{
public:
	int x;
	int y;
	Image_<T> img;
};


const static char* get_suffix(const char* filename)
{
	const char* ext = strrchr(filename, '.');
	if (ext == nullptr)
		return nullptr;
	else return ++ext;
}

template <class T> Image_<T>::Image_()
:width(0), height(0), stepsize(0), bpp(0), channel(0)
{
	count = nullptr;
	start = nullptr;
	data = nullptr;
}

template <class T> Image_<T>::Image_(const char* filename) :Image_()
{
	open(filename);
}

template <class T> Image_<T>::Image_(unsigned _width, unsigned _height, uchar _channel, bool init_value, T _value)
:width(_width), height(_height), channel(_channel), bpp(sizeof(T) * _channel * 8), stepsize(_width * sizeof(T) * _channel)
{
	start = data = new uchar[height * stepsize];
	count = new int(1);
	if (init_value)
	{
		T* temp = (T*)data;
		for (unsigned i = 0; i < _width * _height * _channel; i++)
		{
			*temp++ = _value;
		}
	}
}

template <class T> Image_<T>::Image_(uchar* p, unsigned _width, unsigned _height, uchar _channel)
:width(_width), height(_height), channel(_channel), bpp(sizeof(T)* _channel * 8), stepsize(_width * sizeof(T)* _channel)
{
	start = data = new uchar[height * stepsize];
	count = new int(1);
	memcpy(start, p, height * stepsize);
}

template <class T> Image_<T>::Image_(const Image_& img, bool copy)
:width(img.width), height(img.height), channel(img.channel), bpp(img.bpp)
{
	if (copy)
	{
		stepsize = width * bpp / 8;
		start = data = new uchar[stepsize * height];
		uchar* temp = data;
		uchar* src_temp = img.data;
		for (unsigned r = 0; r < height; r++)
		{
			memcpy(temp, src_temp, stepsize);
			temp += stepsize;
			src_temp += img.stepsize;
		}
		count = new int(1);
	}
	else
	{
		start = img.start;
		(*img.count)++;
		count = img.count;
		data = img.data;
		stepsize = img.stepsize;
	}
}

template <class T> template <class T2> Image_<T>::Image_(const Image_<T2>& img)
:width(img.width), height(img.height), channel(img.channel), bpp(sizeof(T) * 8 * img.channel), stepsize(img.width * sizeof(T) * img.channel)
{
	start = data = new uchar[stepsize * height];

	for (unsigned r = 0; r < height; r++)
	{
		T* temp = ptr(r);
		T2* src_temp = img.ptr(r);
		for (unsigned c = 0; c < width; c++)
		{
			for (unsigned ch = 0; ch < channel; ch++)
			{
				*temp++ = static_cast<T> (*src_temp++);
			}
		}
	}
	count = new int(1);
}

template <class T> void Image_<T>::release()
{
	if (count != nullptr && *count > 1)
	{
		(*count)--;
	}
	else
	{
		if (start != nullptr)
		{
			delete[] start;
			start = nullptr;
		}
		if (count != nullptr)
		{
			delete count;
			count = nullptr;
		}
	}
}

template <class T> Image_<T>::~Image_()
{
	release();
}

template <class T> Image_<T>& Image_<T>:: operator = (const Image_& img)
{
	release();
	start = img.start;
	width = img.width;
	height = img.height;
	(*img.count)++;
	count = img.count;
	stepsize = img.stepsize;
	bpp = img.bpp;
	channel = img.channel;
	data = img.data;
	return *this;
}

template <class T> bool Image_<T>::open(const char* filename)
{
	release();

	const char* ext = get_suffix(filename);
	if (ext == nullptr)
		return false;
	if (strcmp(ext, "bmp") == 0 || strcmp(ext, "BMP") == 0)//bmp
	{
		CBitmap bitmap(filename);
		width = bitmap.GetWidth();
		height = bitmap.GetHeight();
		bpp = bitmap.GetBitCount();
		if (bpp == 8)
		{
			channel = bpp / 8;
			stepsize = width * bpp / 8;
			unsigned size = stepsize * height;
			start = data = new uchar[size];
			count = new int(1);

			if (bitmap.GetBitmapHeaderHeight())
			{
				for (unsigned i = 0; i < height; i++)
				{
					uchar* temp = ptr(i);
					RGBA *BitmapData = (RGBA *)bitmap.GetBits() + (height - 1 - i) * width;
					for (unsigned j = 0; j < width; j++)
					{
						*temp++ = BitmapData->Red;
						BitmapData++;
					}
				}

			}
			else
			{
				uchar* temp = data;
				RGBA *BitmapData = (RGBA *)bitmap.GetBits();
				for (unsigned i = 0; i < height * width; i++)
				{
					*temp++ = BitmapData->Red;
					BitmapData++;
				}
			}
			return true;
	     }
		else return false;
	}
	//return true;  //
	//else if (strcmp(ext, "png") == 0 || strcmp(ext, "PNG") == 0)//png
	//{
	//	FILE* fp;
	//	if (fopen_s(&fp, filename, "rb") != 0){
	//		abort();
	//	}
	//	uchar buf[PNG_BYTES_TO_CHECK];
	//	/* Read in some of the signature bytes */
	//	if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
	//		abort();
	//	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	//	Return nonzero (true) if they match */
	//	if (buf[0] != 0x89 || buf[1] != 'P' || buf[2] != 'N' || buf[3] != 'G'){
	//		abort();
	//	}
	//	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	//	if (!png_ptr) abort();
	//	png_infop info_ptr = png_create_info_struct(png_ptr);
	//	if (!info_ptr) {
	//		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	//		abort();
	//	}
	//	if (setjmp(png_jmpbuf(png_ptr))){
	//		/* Free all of the memory associated with the png_ptr and info_ptr */
	//		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	//		fclose(fp);
	//		abort();
	//	}
	//	png_init_io(png_ptr, fp);
	//	/* If we have already read some of the signature */
	//	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
	//	png_read_info(png_ptr, info_ptr);
	//	width = png_get_image_width(png_ptr, info_ptr);
	//	height = png_get_image_height(png_ptr, info_ptr);
	//	png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	//	bpp = png_get_bit_depth(png_ptr, info_ptr);
	//	stepsize = width * bpp / 8;
	//	channel = png_get_channels(png_ptr, info_ptr);
	//	// Read any color_type into 8bit depth, RGBA format.
	//	// See http://www.libpng.org/pub/png/libpng-manual.txt
	//	//if (bit_depth == 16)
	//	//	png_set_strip_16(png_ptr);
	//	//if (color_type == PNG_COLOR_TYPE_PALETTE)
	//	//	png_set_palette_to_rgb(png_ptr);
	//	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	//	//if (color_type == PNG_COLOR_TYPE_GRAY && m_bpp < 8)
	//	//	png_set_expand_gray_1_2_4_to_8(png_ptr);
	//	//if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	//	//	png_set_tRNS_to_alpha(png_ptr);
	//	// These color_type don't have an alpha channel then fill it with 0xff.
	//	//if (color_type == PNG_COLOR_TYPE_RGB ||
	//	//	color_type == PNG_COLOR_TYPE_GRAY ||
	//	//	color_type == PNG_COLOR_TYPE_PALETTE)
	//	//	png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	//	//if (color_type == PNG_COLOR_TYPE_GRAY ||
	//	//	color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	//	//	png_set_gray_to_rgb(png_ptr);
	//	//png_read_update_info(png_ptr, info_ptr);
	//	data = new uchar[height * stepsize];
	//	uchar* temp = data;
	//	for (uint y = 0; y < height; y++)
	//	{
	//		png_read_row(png_ptr, temp, NULL);
	//		temp += stepsize;
	//	}
	//	fclose(fp);
	//	return true;
	//}
	else return false;
}

template <class T> bool Image_<T>::save(const char* filename)
{
	bool Result = true;

	std::ofstream file(filename, std::ios::out | std::ios::binary);

	if (file.is_open() == false) {
		return false;
	}

	BITMAP_FILEHEADER bfh;
	BITMAP_HEADER bh;
	memset(&bfh, 0, sizeof(bfh));
	memset(&bh, 0, sizeof(bh));

	bfh.Signature = BITMAP_SIGNATURE;
	bfh.BitsOffset = BITMAP_FILEHEADER_SIZE + sizeof(BITMAP_HEADER);
	bfh.Size = (width * height * bpp) / 8 + bfh.BitsOffset;

	bh.HeaderSize = sizeof(BITMAP_HEADER);
	bh.BitCount = bpp;

	if (bpp == 32) {
		bh.Compression = 3; // BITFIELD
		bh.AlphaMask = 0xff000000;
		bh.BlueMask = 0x00ff0000;
		bh.GreenMask = 0x0000ff00;
		bh.RedMask = 0x000000ff;
	}
	else if (bpp == 16) {
		bh.Compression = 3; // BITFIELD
		bh.AlphaMask = 0x00000000;
		bh.BlueMask = 0x0000001f;
		bh.GreenMask = 0x000007E0;
		bh.RedMask = 0x0000F800;
	}
	else {
		bh.Compression = 0; // RGB
	}

	unsigned int LineWidth = (width + 3) & ~3;

	bh.Planes = 1;
	bh.Height = (int)height;
	bh.Width = (int)width;
	bh.SizeImage = (LineWidth * bpp * height) / 8;
	bh.PelsPerMeterX = 3780;
	bh.PelsPerMeterY = 3780;

	if (bpp == 8) 
	{
		uchar* Bitmap = new uchar[bh.SizeImage];

		BGRA *Palette = 0;
		unsigned int PaletteSize = 256;

		Palette = new BGRA[PaletteSize];
		for (int i = 0; i < 256; i++)
		{
			Palette[i].Red = i;
			Palette[i].Green = i;
			Palette[i].Blue = i;
			Palette[i].Alpha = 0xff;
		}

		for (int i = height - 1; i >= 0; i--)
		{
			memcpy(Bitmap + (height - 1 - i) * LineWidth, data + i * stepsize, width);
		}
		
		bfh.BitsOffset += PaletteSize * sizeof(BGRA);

		file.write((char*)&bfh, BITMAP_FILEHEADER_SIZE);
		file.write((char*)&bh, sizeof(BITMAP_HEADER));
		file.write((char*)Palette, PaletteSize * sizeof(BGRA));
		file.write((char*)Bitmap, bh.SizeImage);

		delete[] Bitmap;
		delete[] Palette;
	}
	else
	{
		file.close();
		return false;
	}

	file.close();
	return Result;
}

template <class T> inline T* Image_<T>::ptr() const
{
	return (T*)data;
}

template <class T> inline T* Image_<T>::ptr(unsigned row) const
{
	return (T*)(data + row * stepsize);
}

template <class T> inline T* Image_<T>::ptr(unsigned row, unsigned col) const
{
	return (T*)(data + row * stepsize + col * bpp / 8);
}

template <class T> inline T* Image_<T>::ptr(unsigned row, unsigned col)
{
	return (T*)(data + row * stepsize + col * bpp / 8);
}

template <class T> inline T* Image_<T>::ptr(const Point& pt) const
{
	return (T*)(data + pt.y * stepsize + pt.x * bpp / 8);
}

template <class T> inline T* Image_<T>::ptr(const Point& pt)
{
	return (T*)(data + pt.y * stepsize + pt.x * bpp / 8);
}

template <class T> Image_<T> Image_<T>::roi(const Rect& r) const
{
	Image_ img;
	img.start = start;
	img.width = r.width;
	img.height = r.height;
	(*count)++;
	img.count = count;
	img.stepsize = stepsize;
	img.bpp = bpp;
	img.channel = channel;
	img.data = data + r.y * stepsize + r.x * bpp / 8;
	return img;
}

template <class T> inline void Image_<T>::setroi(const Rect& r)
{
	width = r.width;
	height = r.height;
	data = data + r.y * stepsize + r.x * bpp / 8;
}

template <class T> std::ostream& operator<<(std::ostream& s, const Image_<T>& img)
{
	for (unsigned i = 0; i < img.height; i++)
	{
		T* tmp = img.ptr(i);
		for (unsigned j = 0; j < img.width; j++)
		{
			s << *tmp++ << '\t';
		}
		s << std::endl;
	}
	return s;
}

#endif
