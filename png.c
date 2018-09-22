#include <stdio.h>
#include <string.h>
#include <png.h>
#include "err_msg.h"

#define PNG_BYTES_TO_CHECK 4
extern const char* get_suffix(const char* file_name);

int read_png(const char* file_name, unsigned char* p, int* w, int* h)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_byte check_png_buf[PNG_BYTES_TO_CHECK];
	FILE * fp = fopen(file_name, "rb");
	if (fp == NULL)
		return OPEN_FAILED;
	
	fread(check_png_buf, 1, PNG_BYTES_TO_CHECK, fp);
	if (png_sig_cmp(check_png_buf, 0, PNG_BYTES_TO_CHECK) != 0)
	{
		fclose(fp);
		return TYPE_MISMATCH;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return ERROR;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return ERROR;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return ERROR;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

	if (color_type != PNG_COLOR_TYPE_GRAY || bit_depth != 8)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return TYPE_UNCOVER;
	}

	if (p == NULL)
	{
		*w = width;
		*h = height;
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return 0;
	}
	int number_passes = png_set_interlace_handling(png_ptr);
	for (int pass = 0; pass < number_passes; pass++)
	{
		for (png_uint_32 row = 0; row < height; row++)
			 png_read_row(png_ptr, p + row * width, NULL);
	}
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return 0;
}

int write_png(const char *file_name, unsigned char* p, int w, int h)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;

	const char* suffix = get_suffix(file_name);
	if (strcmp(suffix, "png") != 0 && strcmp(suffix, "PNG") != 0)
		return TYPE_MISMATCH;

	/* Open the file */
	fp = fopen(file_name, "wb");
	if (fp == NULL)
		return OPEN_FAILED;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return ERROR;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return ERROR;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return ERROR;
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* Set the image information here.  Width and height are up to 2^31,
	* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	*/
	png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_GRAY,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);


	/* The easiest way to write the image (you may have a different memory
	* layout, however, so choose what fits your needs best).  You need to
	* use the first method if you aren't handling interlacing yourself.
	*/

	for (int y = 0; y < h; y++)
		png_write_row(png_ptr, p + y * w);

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(fp);

	/* That's it */
	return 0;
}
