#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

#include <png.h>

struct mappedpng {
	FILE *f;
	png_structp ptr;
	png_infop info;
	png_bytepp rows;
	png_uint_32 x, y;
};

static struct mappedpng map(const char *path) {
	struct mappedpng png = {0};

	png.f = fopen(path, "rb");
	if(!png.f) {
		fprintf(stderr, "Failed to open %s\n", path);
		goto fail;
	}
	
	png.ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png.ptr) {
		fprintf(stderr, "libpng error\n");
		goto fail;
	}

	png.info = png_create_info_struct(png.ptr);
	if(!png.info) {
		fprintf(stderr, "libpng error\n");
		goto fail;
	}

	if(setjmp(png_jmpbuf(png.ptr))) {
		fprintf(stderr, "Failed to read %s\n", path);
		goto fail;
	}

	png_init_io(png.ptr, png.f);
	png_read_info(png.ptr, png.info);

	if(png_get_color_type(png.ptr, png.info) != PNG_COLOR_TYPE_RGBA || png_get_bit_depth(png.ptr, png.info) != 8) {
		fprintf(stderr, "This color space or depth is not implemented, but used in %s\n", path);
		goto fail;
	}

	png.x = png_get_image_width(png.ptr, png.info);
	png.y = png_get_image_height(png.ptr, png.info);

	printf("Info: %s opened\n", path);
	return png;

	fail:
	png_destroy_read_struct(&png.ptr, &png.info, NULL);
	if(png.f) {
		fclose(png.f);
		png.f = NULL;
	}
	png.ptr = NULL;
	abort();
	return png;
}
static void unmap(struct mappedpng png) {
	png_destroy_read_struct(&png.ptr, &png.info, NULL);
	if(png.f)
		fclose(png.f);
}

static struct mappedpng mapwrite(const char *path, const struct mappedpng in) {
	struct mappedpng png = {0};
	png.f = fopen(path, "wb");
	if(!png.f) {
		fprintf(stderr, "Failed to open for write \"%s\"", path);
		goto fail;
	}

	png.ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png.ptr) {
		fprintf(stderr, "Failed to create write context\n");
		goto fail;
	}

	png.info = png_create_info_struct(png.ptr);
	if(!png.info) {
		fprintf(stderr, "Failed to create write context\n");
		goto fail;
	}

	if(setjmp(png_jmpbuf(png.ptr))) {
		fprintf(stderr, "Failed to write\n");
		goto fail;
	}

	png_init_io(png.ptr, png.f);
	png_set_sig_bytes(png.ptr, 0);

	png_set_IHDR(png.ptr, png.info, in.x, in.y,
		8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png.ptr, png.info);
	printf("Info: %s opened\n", path);
	return png;

	fail:
	png_destroy_write_struct(&png.ptr, &png.info);
	if(png.f) {
		fclose(png.f);
		png.f = NULL;
	}
	png.ptr = NULL;
	abort();
	return png;
}
static void unmapwrite(struct mappedpng png) {
	printf("Written!\n");
	png_write_end(png.ptr, png.info);
	png_destroy_write_struct(&png.ptr, &png.info);
	if(png.f)
		fclose(png.f);
}

void blendRGBArow(png_bytep out, png_bytep up, png_bytep ovr, png_uint_32 x, png_uint_32 row, bool mask, float premult) {
	if(!mask)
		premult = 1.f;
	for(png_uint_32 i = 0; i < x; i++) {
		png_byte a = ovr[i * 4 + 3];
		if(a != 0) {
			if(a != 255)
				fprintf(stderr, "Warning: alpha at %"PRIu32" %"PRIu32" is not 0 or 255\n", i, row);
			if(!mask && up[i * 4 + 3] != 0)
				if(((uint32_t*)up)[i] != ((uint32_t*)ovr)[i])
					fprintf(stderr, "Warning: override conflicts with upstream template at %"PRIu32" %"PRIu32"\n", i, row);
				else
					printf("Info: override redefines same value at %"PRIu32" %"PRIu32"\n", i, row);
			((uint32_t*)out)[i] = ((uint32_t*)ovr)[i];
		} else {
			if(premult != 1.f) {
				png_byte r = up[i * 4 + 0],
					g = up[i * 4 + 1],
					b = up[i * 4 + 2];
					r = ((float)r) * premult;
					g = ((float)g) * premult;
					b = ((float)b) * premult;
					out[i * 4 + 0] = r;
					out[i * 4 + 1] = g;
					out[i * 4 + 2] = b;
					out[i * 4 + 3] = 255;
			} else
				((uint32_t*)out)[i] = ((uint32_t*)up)[i];
		}
	}
}

void overlay(struct mappedpng up, struct mappedpng ovr, const char *outpath, bool mask, float factor) {
	void *buf;
	if(ovr.x != up.x || ovr.y != up.y) {
		fprintf(stderr, "autopick resolution mismatch, copying override\n");
		if(mask)
			abort();//TODO
		FILE *f = ovr.f;
		long save = ftell(f);
		fseek(f, 0, SEEK_END);
		long len = ftell(f);
		fseek(f, 0, SEEK_SET);
		buf = malloc(len);
		if(!buf) {
			fprintf(stderr, "Failed to allocate memory!\n");
			exit(-1);
		}
		fread(buf, len, 1, f);
		fseek(f, save, SEEK_SET);

		f = fopen(outpath, "wb");
		assert(f);
		fwrite(buf, len, 1, f);
		fclose(f);
	} else {
		struct mappedpng out = mapwrite(outpath, up);
		png_uint_32 rb = png_get_rowbytes(up.ptr, up.info);
		buf = malloc(rb * up.x * 3);
		if(!buf) {
			fprintf(stderr, "Failed to allocate memory!\n");
			exit(-1);
		}
		void *outP, *upP, *ovrP;
		outP = buf;
		upP = buf + rb;
		ovrP = upP + rb;

		for(png_uint_32 i = 0; i < up.y; i++) {
			png_read_row(up.ptr, upP, NULL);
			png_read_row(ovr.ptr, ovrP, NULL);
			blendRGBArow(outP, upP, ovrP, up.x, i, mask, factor);
			png_write_row(out.ptr, outP);
		}
		unmapwrite(out);
	}
	free(buf);
}

int main(int argc, char **argv) {
	float factor = 1.f;
	if(argc < 3 || argc > 4) {
		printf("Usage: %s pixel_override mask_override [mask factor]\n", argv[0]);
		return 0;
	}
	if(argc == 4)
		factor = (float)atof(argv[3]);
	if(factor > 1.f || factor < 0.f) {
		fprintf(stderr, "Error: factor %f is not valid and should be in range [0, 1]. Setting factor to 1\n", factor);
		factor = 1;
	}
	//up - upstream
	//ovr - override
	struct mappedpng ovr = map(argv[1]),
		up = map("upstream.png");
	overlay(up, ovr, "autopick.png", false, factor);
	unmap(up);
	unmap(ovr);

	ovr = map(argv[2]);
	up = map("upstream_mask.png");
	printf("Info: factor = %f\n", factor);
	overlay(up, ovr, "mask.png", true, factor);
	unmap(up);
	unmap(ovr);
}
