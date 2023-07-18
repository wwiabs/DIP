#include <stdio.h>
#include <stdint.h>
#include <string.h>

//#pragma warning(disable:4996)

#pragma pack(push, 1)

typedef struct _BITMAP_FILE_HEADER {
	uint16_t Signature;
	uint32_t Size;
	uint32_t Reserved;
	uint32_t BitsOffset;
} BITMAP_FILE_HEADER;

typedef struct _BITMAP_INFO_HEADER {
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
} BITMAP_INFO_HEADER;

typedef struct _BGRA {
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} BGRA;

#pragma pack(pop)

const char* get_suffix(const char* file_name)
{
	const char* ext = strrchr(file_name, '.');
	if (ext == NULL)
		return NULL;
	else return ++ext;
}

#define BITMAP_SIGNATURE 0x4d42

int read_bmp(const char* file_name, uint8_t* p, int* w, int* h)
{
	BITMAP_FILE_HEADER fh;
	BITMAP_INFO_HEADER dibh;
	FILE* fp = fopen(file_name, "rb");
	if (fp == NULL)
		return -1;
	fread(&fh, 1, sizeof(BITMAP_FILE_HEADER), fp);
	if (fh.Signature != BITMAP_SIGNATURE)
	{
		fclose(fp);
		return -1;
	}
	fread(&dibh, 1, sizeof(BITMAP_INFO_HEADER), fp);

	if (dibh.BitCount != 8 || dibh.Compression != 0)
	{
		fclose(fp);
		return -1;
	}
	int32_t height = dibh.Height > 0 ? dibh.Height : -dibh.Height;
	if (p == NULL)
	{
		*w = dibh.Width;
		*h = height;
		fclose(fp);
		return 0;
	}

	int32_t padding =((dibh.Width + 3) & ~3) - dibh.Width;
	fseek(fp, fh.BitsOffset, SEEK_SET);
	if (dibh.Height > 0)
	{
		for (int r = height - 1; r >= 0; r--)
		{
			fread(p + r*dibh.Width, 1, dibh.Width, fp);
			fseek(fp, padding, SEEK_CUR);
		}
	}
	else
	{
		for (int r = 0; r < height; r++)
		{
			fread(p + r*dibh.Width, 1, dibh.Width, fp);
			fseek(fp, padding, SEEK_CUR);
		}
	}
	fclose(fp);
	return 0;
}

//ÄÚ´æÁ¬Ðø
int write_bmp(const char* file_name, uint8_t* p, int w, int h)
{
	BITMAP_FILE_HEADER fh;
	BITMAP_INFO_HEADER dibh;
	FILE* fp;
	const char* suffix = get_suffix(file_name);
	if (strcmp(suffix, "bmp") != 0 && strcmp(suffix, "BMP") != 0)//bmp
		return -1;
	fp = fopen(file_name, "wb");
	if (fp == NULL)
		return -1;	
	fh.Signature = BITMAP_SIGNATURE;
	fh.BitsOffset = sizeof(BITMAP_FILE_HEADER)+sizeof(BITMAP_INFO_HEADER)+sizeof(BGRA)* 256;
	fh.Reserved = 0;
	fh.Size = ((w + 3)&~3)*h + fh.BitsOffset;
	fwrite(&fh, 1, sizeof(BITMAP_FILE_HEADER), fp);
	dibh.Width = w;
	dibh.Height = h;
	dibh.BitCount = 8;
	dibh.Compression = 0;
	dibh.Planes = 1;
	dibh.HeaderSize = sizeof(BITMAP_INFO_HEADER);
	dibh.SizeImage = 0;
	dibh.PelsPerMeterX = 0;
	dibh.PelsPerMeterY = 0;
	dibh.ClrUsed = 0;
	dibh.ClrImportant = 0;
	fwrite(&dibh, 1, sizeof(BITMAP_INFO_HEADER), fp);
	BGRA Palette[256];
	for (int i = 0; i < 256; i++)
	{
		Palette[i].Alpha = 0x00;
		Palette[i].Blue = i;
		Palette[i].Green = i;
		Palette[i].Red = i;
	}
	fwrite(Palette, sizeof(BGRA), 256, fp);
	int32_t padding = ((w + 3) & ~3) - w;
	uint8_t tmp[4] = {0};
	for (int r = h - 1; r >= 0; r--)
	{
		fwrite(p + r*w, 1, w, fp);
		fwrite(tmp, 1, padding, fp);
	}
	fclose(fp);
 	return 0;
}
