#include <stdio.h>
#include <string.h>
//#ifndef __LITTLE_ENDIAN__
//#ifndef __BIG_ENDIAN__
//#define __LITTLE_ENDIAN__
//#endif
//#endif
//
//#ifdef __LITTLE_ENDIAN__
//#define BITMAP_SIGNATURE 0x4d42
//#else
//#define BITMAP_SIGNATURE 0x424d
//#endif
#pragma warning(disable:4996)

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

typedef struct tagBITMAPINFOHEADER {
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
	//uint32_t RedMask;
	//uint32_t GreenMask;
	//uint32_t BlueMask;
	//uint32_t AlphaMask;
	//uint32_t CsType;
	//uint32_t Endpoints[9]; // see http://msdn2.microsoft.com/en-us/library/ms536569.aspx
	//uint32_t GammaRed;
	//uint32_t GammaGreen;
	//uint32_t GammaBlue;
} BITMAPINFOHEADER;

typedef struct _BGRA {
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} BGRA;

#pragma pack(pop)

const static char* get_suffix(const char* filename)
{
	const char* ext = strrchr(filename, '.');
	if (ext == nullptr)
		return nullptr;
	else return ++ext;
}

int read_bmp(const char* filename, uint8_t* p, int* w, int* h)
{
	FILE* pf;
	if ((pf = fopen(filename, "rb")) == NULL)
		return -1;

	BITMAP_FILEHEADER fh;
	fread(&fh, sizeof(BITMAP_FILEHEADER), 1, pf);
	if (fh.Signature != 0x4d42)
	{
		fclose(pf);
		return -1;
	}
	BITMAPINFOHEADER dibh;
	fread(&dibh, sizeof(BITMAPINFOHEADER), 1, pf);
	if (dibh.BitCount != 8 || dibh.Compression != 0)
	{
		fclose(pf);
		return -1;
	}
	int32_t height = dibh.Height > 0 ? dibh.Height : -dibh.Height;
	if (p == NULL)
	{
		*w = dibh.Width;
		*h = height;
		fclose(pf);
		return -1;
	}
	else if (*w != dibh.Width || *h != height)
	{
		fclose(pf);
		return -1;
	}
	uint32_t linewidth = (dibh.Width + 3)&~3;
	uint8_t *pline = new uint8_t[linewidth];
	fseek(pf, sizeof(BITMAP_FILEHEADER) + fh.BitsOffset, SEEK_SET);
	if (dibh.Height > 0)
	{
		for (int r = *h - 1; r >= 0; r--)
		{
			fread(pline, linewidth, 1, pf);
			memcpy(p + r**w, pline, *w);
		}
	}
	else
	{
		for (int r = 0; r < *h; r++)
		{
			fread(pline, linewidth, 1, pf);
			memcpy(p + r**w, pline, *w);
		}
	}
	fclose(pf);
	delete[] pline;
	return 0;
}

int save_bmp(const char* filename, uint8_t* p, int w, int h)
{
	const char* suffix = get_suffix(filename);
	if (strcmp(suffix, "bmp") != 0 && strcmp(suffix, "BMP") != 0)//bmp
		return -1;
	FILE* pf;
	if ((pf = fopen(filename, "wb")) == NULL)
		return -1;
	BITMAP_FILEHEADER fh;
	fh.Signature = 0x4d42;
	fh.BitsOffset = sizeof(BITMAP_FILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(BGRA)* 256;
	fh.Reserved = 0;
	fh.Size = ((w + 3)&~3)*h + fh.BitsOffset;
	fwrite(&fh, sizeof(BITMAP_FILEHEADER), 1, pf);
	BITMAPINFOHEADER dibh;
	dibh.Width = w;
	dibh.Height = h;
	dibh.BitCount = 8;
	dibh.Compression = 0;
	dibh.Planes = 1;
	dibh.HeaderSize = sizeof(BITMAPINFOHEADER);
	dibh.SizeImage = 0;
	dibh.PelsPerMeterX = 0;
	dibh.PelsPerMeterY = 0;
	dibh.ClrUsed = 0;
	dibh.ClrImportant = 0;
	fwrite(&dibh, sizeof(BITMAPINFOHEADER), 1, pf);
	BGRA *Palette = new BGRA[256];
	for (int i = 0; i < 256; i++)
	{
		Palette[i].Alpha = 0x00;
		Palette[i].Blue = i;
		Palette[i].Green = i;
		Palette[i].Red = i;
	}
	fwrite(Palette, sizeof(BGRA), 256, pf);
	uint32_t linewidth = (w + 3)&~3;
	uint8_t *tmp = new uint8_t[linewidth];
	memset(tmp, 0, linewidth);
	for (int r = h-1; r >= 0; r--)
	{
		memcpy(tmp, p + r*w, w);
		fwrite(tmp, linewidth, 1, pf);
	}
	delete[] tmp;
	delete[] Palette;
	fclose(pf);
 	return 0;
}

