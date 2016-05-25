#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <png.h>

int main (int argc, char** argv) {

	if (argc != 4) { printf("Usage: %s [dfile.p8.png] [ifile.p8.png] [ofile.p8.png]\n",argv[0]); return 1;}

	const char* dfname = argv[1];
	const char* pfname = argv[2];
	const char* ofname = argv[3];

	FILE* dfile = fopen(dfname,"rb");

	if (!dfile) {
		perror("fopen data file"); return 1; }

	FILE* pfile = fopen(pfname,"rb");

	if (!pfile) {
		perror("fopen picture file"); return 1; }

	char pnghead[8];

	if (fread(pnghead,1,8,dfile) != 8) { 
		perror("read data"); return 1;}

	if (png_sig_cmp(pnghead,0,8) != 0) { 
		fprintf(stderr,"data file is not a PNG!\n"); return 1;}

	png_structp d_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!d_png) {fprintf(stderr,"Can't create png_structp for data img\n"); return 1;}

	png_infop d_inf = png_create_info_struct(d_png);

	png_init_io(d_png, dfile);
	png_set_sig_bytes(d_png, 8);

	png_read_info(d_png, d_inf);

	uint32_t d_w = png_get_image_width(d_png, d_inf);
	uint32_t d_h = png_get_image_height(d_png, d_inf);

	if ((d_w != 160) || (d_h != 205)) {
		fprintf(stderr,"Data image doesn't match PICO-8's dimensions.\n"
				"PICO-8 cartridge PNGs have a size of 160x205.\n");
		return 1;}

	int d_col = png_get_color_type(d_png, d_inf);
	if (d_col != PNG_COLOR_TYPE_RGB_ALPHA) {
		fprintf(stderr,"Data image should be 8-bit RGBA.\n"); return 1; }


	int d_passes = png_set_interlace_handling(d_png);
	png_read_update_info(d_png,d_inf);

	png_bytep* d_rowptrs = malloc(sizeof(png_bytep) * d_h);
	for (int y=0; y < d_h; y++) d_rowptrs[y] = malloc(png_get_rowbytes(d_png, d_inf));

	png_read_image(d_png, d_rowptrs);
	fclose(dfile);

	if (fread(pnghead,1,8,pfile) != 8) { 
		perror("read picture"); return 1;}

	if (png_sig_cmp(pnghead,0,8) != 0) { 
		fprintf(stderr,"picture file is not a PNG!\n"); return 1;}

	png_structp p_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!p_png) {fprintf(stderr,"Can't create png_structp for picture img\n"); return 1;}

	png_infop p_inf = png_create_info_struct(p_png);

	png_init_io(p_png, pfile);
	png_set_sig_bytes(p_png, 8);

	png_read_info(p_png, p_inf);

	uint32_t p_w = png_get_image_width(p_png, p_inf);
	uint32_t p_h = png_get_image_height(p_png, p_inf);

	if ((p_w != 160) || (p_h != 205)) {
		fprintf(stderr,"Picture image doesn't match PICO-8's dimensions.\n"
				"PICO-8 cartridge PNGs have a size of 160x205.\n");
		return 1;}

	int p_col = png_get_color_type(p_png, p_inf);

	if (p_col != PNG_COLOR_TYPE_RGB_ALPHA)
		printf("Picture PNG is not RGBA. Trying to convert...\n");

	switch(p_col) {

		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(p_png);
			if (png_get_valid(p_png,p_inf,PNG_INFO_tRNS))
				png_set_tRNS_to_alpha(p_png);
			png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
			break;
		case PNG_COLOR_TYPE_GRAY:
			png_set_gray_to_rgb(p_png);
			png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			png_set_gray_to_rgb(p_png);
			break;
		case PNG_COLOR_TYPE_RGB:
			png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
			break;
	}

	int p_passes = png_set_interlace_handling(p_png);
	png_read_update_info(p_png,p_inf);

	png_bytep* p_rowptrs = malloc(sizeof(png_bytep) * p_h);
	for (int y=0; y < p_h; y++) p_rowptrs[y] = malloc(png_get_rowbytes(p_png, p_inf));

	png_read_image(p_png, p_rowptrs);
	fclose(pfile);

	// ---- output file

	FILE* ofile = fopen(ofname,"wb");
	if (!ofile) {
		perror("fopen output file"); return 1; }

	png_structp o_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!o_png) {fprintf(stderr,"Can't create png_structp for output img\n"); return 1;}

	png_infop o_inf = png_create_info_struct(o_png);
	if (!o_inf) {fprintf(stderr,"Can't create png_infop for output img\n"); return 1;}

	png_init_io(o_png,ofile);

	png_set_IHDR(o_png, o_inf, 160, 205, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(o_png, o_inf);

	for (int iy=0; iy < 205; iy++) {
		for (int ix=0; ix < 160; ix++) {
			p_rowptrs[iy][ix*4  ] = ((p_rowptrs[iy][ix*4  ] & 0xFC) | (d_rowptrs[iy][ix*4  ] & 0x03));
			p_rowptrs[iy][ix*4+1] = ((p_rowptrs[iy][ix*4+1] & 0xFC) | (d_rowptrs[iy][ix*4+1] & 0x03));
			p_rowptrs[iy][ix*4+2] = ((p_rowptrs[iy][ix*4+2] & 0xFC) | (d_rowptrs[iy][ix*4+2] & 0x03));
			p_rowptrs[iy][ix*4+3] = ((p_rowptrs[iy][ix*4+3] & 0xFC) | (d_rowptrs[iy][ix*4+3] & 0x03));
		}
	}

	png_write_image(o_png, p_rowptrs);
	png_write_end(o_png, NULL);
	
	for (int y=0; y < p_h; y++) {
		free(d_rowptrs[y]);
		free(p_rowptrs[y]); }

	free(d_rowptrs); free(p_rowptrs);

	fclose(ofile);

}
