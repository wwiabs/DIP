/*
* Windows Bitmap File Loader
* Version 1.2.5 (20120929)
*
* Supported Formats: 1, 4, 8, 16, 24, 32 Bit Images
* Alpha Bitmaps are also supported.
* Supported compression types: RLE 8, BITFIELDS
*
* Created by: Benjamin Kalytta, 2006 - 2012
* Thanks for bug fixes goes to: Chris Campbell
*
* Licence: Free to use, URL to my source and my name is required in your source code.
*
* Source can be found at http://www.kalytta.com/bitmap.h
*
* Warning: This code should not be used in unmodified form in a production environment.
* It should only serve as a basis for your own development.
* There is only a minimal error handling in this code. (Notice added 20111211)
*/

#ifndef BITMAP_MODIFIED_H
#define BITMAP_MODIFIED_H

//#include <iostream>
#include <fstream>
//#include <string>
#include "data_type.h"
#ifndef __LITTLE_ENDIAN__
#ifndef __BIG_ENDIAN__
#define __LITTLE_ENDIAN__
#endif
#endif

#ifdef __LITTLE_ENDIAN__
#define BITMAP_SIGNATURE 0x4d42
#else
#define BITMAP_SIGNATURE 0x424d
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef __int32 int32_t;
#elif defined(__GNUC__) || defined(__CYGWIN__) || defined(__MWERKS__) || defined(__WATCOMC__) || defined(__PGI) || defined(__LCC__)
#include <stdint.h>
#else
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;
typedef int int32_t;
#endif

#pragma pack(push, 1)

typedef struct _BITMAP_FILEHEADER {
	uint16_t Signature;
	uint32_t Size;
	uint32_t Reserved;
	uint32_t BitsOffset;
} BITMAP_FILEHEADER;

#define BITMAP_FILEHEADER_SIZE 14

typedef struct _BITMAP_HEADER {
	uint32_t HeaderSize;
	int32_t Width;
	int32_t Height;
	uint16_t Planes;
	uint16_t BitCount;
	uint32_t Compression;
	uint32_t SizeImage;
	int32_t PelsPerMeterX;
	int32_t PelsPerMeterY;
	uint32_t ClrUsed;
	uint32_t ClrImportant;
	uint32_t RedMask;
	uint32_t GreenMask;
	uint32_t BlueMask;
	uint32_t AlphaMask;
	uint32_t CsType;
	uint32_t Endpoints[9]; // see http://msdn2.microsoft.com/en-us/library/ms536569.aspx
	uint32_t GammaRed;
	uint32_t GammaGreen;
	uint32_t GammaBlue;
} BITMAP_HEADER;


#pragma pack(pop)

class CBitmap {
private:
	BITMAP_FILEHEADER m_BitmapFileHeader;
	BITMAP_HEADER m_BitmapHeader;
	uchar *m_BitmapData;
	unsigned int m_BitmapSize;

public:

	CBitmap() : m_BitmapData(0), m_BitmapSize(0)  {
		Dispose();
	}

	CBitmap(const char* Filename) : m_BitmapData(0), m_BitmapSize(0) {
		Load(Filename);
	}

	~CBitmap() {
		Dispose();
	}

	void Dispose() {
		if (m_BitmapData) {
			delete[] m_BitmapData;
			m_BitmapData = 0;
		}
		memset(&m_BitmapFileHeader, 0, sizeof(m_BitmapFileHeader));
		memset(&m_BitmapHeader, 0, sizeof(m_BitmapHeader));
	}

	/* Load specified Bitmap and stores it as RGBA in an internal buffer */

	bool Load(const char *Filename) {
		std::ifstream file(Filename, std::ios::binary | std::ios::in);

		if (file.bad()) {
			return false;
		}

		if (file.is_open() == false) {
			return false;
		}

		Dispose();

		file.read((char*)&m_BitmapFileHeader, BITMAP_FILEHEADER_SIZE);
		if (m_BitmapFileHeader.Signature != BITMAP_SIGNATURE) {
			return false;
		}

		file.read((char*)&m_BitmapHeader, sizeof(BITMAP_HEADER));

		m_BitmapSize = GetWidth() * GetHeight();
		m_BitmapData = new uchar[m_BitmapSize*m_BitmapHeader.BitCount / 8];

		unsigned int LineWidth = ((GetWidth() * GetBitCount() / 8) + 3) & ~3;
		unsigned ByteWidth = GetWidth() * GetBitCount() / 8;
		unsigned pad = LineWidth - ByteWidth;

		file.seekg(m_BitmapFileHeader.BitsOffset, std::ios::beg);

		int Index = 0;
		bool Result = false;

		if (m_BitmapHeader.Compression == 0 && (m_BitmapHeader.BitCount == 8 || m_BitmapHeader.BitCount == 24)) {
			uchar* newline = m_BitmapData;
			for (unsigned i = 0; i < GetHeight(); i++) {
				file.read((char*)newline, ByteWidth);
				file.seekg(pad, std::ios::cur);
				newline += GetWidth() * m_BitmapHeader.BitCount / 8;
				}
			Result = true;
			}

		file.close();
		return Result;
	}

	unsigned int GetWidth() {
		/* Add plausibility test */
		// if (abs(m_BitmapHeader.Width) > 8192) {
		//	m_BitmapHeader.Width = 8192;
		// }
		return m_BitmapHeader.Width < 0 ? -m_BitmapHeader.Width : m_BitmapHeader.Width;
	}

	unsigned int GetHeight() {
		/* Add plausibility test */
		// if (abs(m_BitmapHeader.Height) > 8192) {
		//	m_BitmapHeader.Height = 8192;
		// }
		return m_BitmapHeader.Height < 0 ? -m_BitmapHeader.Height : m_BitmapHeader.Height;
	}

	unsigned int GetSize() {
		/* Add plausibility test */
		// if (abs(m_BitmapHeader.Height) > 8192) {
		//	m_BitmapHeader.Height = 8192;
		// }
		return m_BitmapSize;
	}

	unsigned int GetBitCount() {
		/* Add plausibility test */
		// if (m_BitmapHeader.BitCount > 32) {
		//	m_BitmapHeader.BitCount = 32;
		// }
		return m_BitmapHeader.BitCount;
	}

	//bool CopyData(void* Buffer) {
	//	bool Result = false;
	//	if (Buffer != 0) {
	//		memcpy(Buffer, m_BitmapData, m_BitmapSize * m_BitmapHeader.BitCount / 8);
	//		Result = true;
	//	}
	//	return Result;
	//}


	void* GetBits() {
		return m_BitmapData;
	}

};

#endif
