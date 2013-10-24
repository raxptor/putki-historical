#include "pngutil.h"

#include <putki/builder/resource.h>
#include <putki/builder/builder.h>

#include <png.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

namespace putki
{
	namespace pngutil
	{
		struct write_buffer
		{
			char *output;
			size_t size;
		};

		namespace
		{
			void write(png_structp png_ptr, png_bytep data, png_size_t length)
			{
				write_buffer* p = (write_buffer*) png_get_io_ptr(png_ptr); 

				size_t nsize = p->size + length;

				/* allocate or grow buffer */
				if (p->output)
					p->output = (char*)realloc(p->output, nsize);
				 else
					p->output = (char*)malloc(nsize);

				/* copy new bytes to end of buffer */
				memcpy(p->output + p->size, data, length);
				p->size += length;
			}

			void flush(png_structp png_ptr)
			{

			}
		}

		write_buffer write_to_mem(unsigned int *pixbuf, unsigned int width, unsigned int height)
		{
			png_structp png_ptr = NULL;
			png_infop info_ptr = NULL;
			size_t x, y;
			png_byte ** row_pointers = NULL;
			int status = -1;

			png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
			if (png_ptr == NULL) {
				goto png_create_write_struct_failed;
			}

			info_ptr = png_create_info_struct(png_ptr);
			if (info_ptr == NULL) {
				goto png_create_info_struct_failed;
			}

			/* Set image attributes. */
			png_set_IHDR (png_ptr,
							info_ptr,
							width,
							height,
							8,
							PNG_COLOR_TYPE_RGBA,
							PNG_INTERLACE_NONE,
							PNG_COMPRESSION_TYPE_DEFAULT,
							PNG_FILTER_TYPE_DEFAULT);
    
			/* Initialize rows of PNG. */
			row_pointers = (png_byte**) png_malloc(png_ptr, height * sizeof (png_byte *));
			for (y = 0; y < height; ++y)
			{
				png_byte *row = (png_byte*) png_malloc(png_ptr, width * 4);
				row_pointers[y] = row;
				for (x = 0; x < width; ++x) 
				{
					*row++ = (pixbuf[y * width + x] >> 16) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 8) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 0) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 24) & 0xff;
				}
			}
    
			/* Write the image data to "fp". */

			write_buffer wb;
			wb.size = 0;
			wb.output = 0;

			png_set_write_fn(png_ptr, &wb, &write, &flush);

			png_set_rows (png_ptr, info_ptr, row_pointers);
			png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

			/* The routine has successfully written the file, so we set
				"status" to a value which indicates success. */

			status = 0;

			for (y=0;y<height;y++)
			{
				png_free(png_ptr, row_pointers[y]);
			}

			png_free(png_ptr, row_pointers);
			png_create_info_struct_failed:
			png_destroy_write_struct (&png_ptr, &info_ptr);
			png_create_write_struct_failed:

			return wb;
		}

		std::string write_to_temp(builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height)
		{
			std::string outpath;
			write_buffer wb = write_to_mem(pixbuf, width, height);
			if (wb.output)
			{
				outpath = putki::resource::save_temp(builder, path, wb.output, (long long) wb.size);
				::free(wb.output);
			}

			return outpath;
		}

		std::string write_to_output(builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height)
		{
			std::string outpath;
			write_buffer wb = write_to_mem(pixbuf, width, height);
			if (wb.output)
			{
				outpath = putki::resource::save_output(builder, path, wb.output, (long long) wb.size);
				::free(wb.output);
			}
			return outpath;
		}

		bool load(const char *path, loaded_png *out)
		{
			png_structp png_ptr;
			png_infop info_ptr;
			unsigned int sig_read = 0;
			int color_type, interlace_type;
			FILE *fp;
 
			if (!(fp = fopen(path, "rb")))
				return false;

			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
 
			if (png_ptr == NULL) 
			{
				fclose(fp);
				return false;
			}
 
			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) 
			{
				fclose(fp);
				png_destroy_read_struct(&png_ptr, NULL, NULL);
				return false;
			}
 
			png_init_io(png_ptr, fp);
			png_set_sig_bytes(png_ptr, sig_read);

			png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
 
			png_uint_32 width, height;
			int bit_depth;
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

			out->width = width;
			out->height = height;
 
			unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
			out->pixels = (unsigned int *) ::malloc(4 * width * height);
			out->bpp = 32;
 
			png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
 			
			for (unsigned int i = 0; i < height; i++)
			{
				unsigned int *outptr = &out->pixels[width * i];
				unsigned char *inptr = row_pointers[i];

				for (unsigned int x=0;x<width;x++)
				{
					if (row_bytes >= width * 4)
					{
						*outptr++ = (inptr[3] << 24) | (inptr[0] << 16) | (inptr[1] << 8) | inptr[2];
						inptr += 4;
					}
					else if (row_bytes >= width * 3)
					{
						*outptr++ = (0xff << 24) | (inptr[0] << 16) | (inptr[1] << 8) | inptr[2];
						inptr += 3;
					}
				}
			}
 
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);


 
			/* Close the file */
			fclose(fp);
			return true;
		}

		void free(loaded_png *png)
		{

		}
	}

}