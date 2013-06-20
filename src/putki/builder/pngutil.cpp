#include "pngutil.h"

#include <png.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

		bool write_to_temp(const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height)
		{
			FILE * fp;
			png_structp png_ptr = NULL;
			png_infop info_ptr = NULL;
			size_t x, y;
			png_byte ** row_pointers = NULL;
			int status = -1;
			int depth = 8;

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
							32,
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
					*row++ = (pixbuf[y * width + x] >> 0) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 8) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 16) & 0xff;
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

			fp = fopen("c:\\utput.png", "wb");
			fwrite(wb.output, 1, wb.size, fp);
    
			for (y=0;y<height;y++)
			{
				png_free(png_ptr, row_pointers[y]);
			}

			png_free(png_ptr, row_pointers);
			png_create_info_struct_failed:
			png_destroy_write_struct (&png_ptr, &info_ptr);
			png_create_write_struct_failed:
			fclose(fp);

			return status == 0;
		}
	}

}