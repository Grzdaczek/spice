#ifndef _IMAGE_H_
#define _IMAGE_H_

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP_8B(a) MIN(MAX(a,0),255)

#include <stdio.h>

typedef struct {
    double* data;
    int size;
} Kernel;

typedef struct {
    int x, y;
} Point;

typedef struct {
    unsigned char r, g, b;
} PxRGB;

typedef struct {
    unsigned char h, s, v;
} PxHSV;

typedef struct {
    PxRGB *data;
    int data_size;
    int width;
    int height;
    int bitdepth;
} Image;

int image_read_ppm(Image* imgp, FILE* imgin);
int image_write_ppm(Image* imgp, FILE* imgout);
int image_invert(Image* imgp);
int image_scale_nn(Image* imgp, int w, int h);
int image_dither(Image* imgp, int bitdepth);
int image_kernel_filter(Image* imgp, Kernel kernel);
int image_resize(Image *imgp, int w, int h, int x, int y);
int image_compose(Image* img1p, Image* img2p, int x, int y);
int image_contrast(Image* img1p, double x);
int image_grayscale(Image* imgp);

Kernel kernel_normalized(int size);
Kernel kernel_sobel(int size);

#endif /* _IMAGE_H_ */