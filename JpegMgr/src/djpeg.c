#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */

unsigned char* decompress_Jpeg( FILE* pJepgFile, int *img_width, int *img_height )
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	djpeg_dest_ptr dest_mgr = NULL;
	JDIMENSION num_scanlines;
	unsigned char* bmp_buffer = 0;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	/* Specify data source for decompression */
	jpeg_stdio_src(&cinfo, pJepgFile);

	/* Read file header, set default decompression parameters */
	(void) jpeg_read_header(&cinfo, TRUE);

	dest_mgr = jinit_write_bmp(&cinfo, FALSE);

	/* Start decompressor */
	(void) jpeg_start_decompress(&cinfo);

	/* Write output file header */
	(*dest_mgr->start_output) (&cinfo, dest_mgr);

	/* Process data */
	while (cinfo.output_scanline < cinfo.output_height) {
		num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
			dest_mgr->buffer_height);
		(*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
	}

	/* Finish decompression and release memory.
	* I must do it in this order because output module has allocated memory
	* of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
	*/

	*img_width = cinfo.output_width;
	*img_height = cinfo.output_height;

	bmp_buffer = malloc( cinfo.output_height*cinfo.output_width*3 );
	(*dest_mgr->get_pixel_buffer) (&cinfo, dest_mgr, bmp_buffer);

	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return bmp_buffer;			/* suppress no-return-value warnings */
}
