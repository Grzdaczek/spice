#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "err.h"

/*=-------------------------------------------------------------------------=*/

int image_read_ppm(Image* imgp, FILE* imgin) {
    size_t len;
    char* line = NULL;
    int res;

    res = getline(&line, &len, imgin);
    if (strcmp(line, "P6\n")) fail(ERR_FILE_READ, "");

    while (getline(&line, &len, imgin) != -1 && line[0] == '#');

    sscanf(line, "%d %d", &imgp->width, &imgp->height);
    imgp->data_size = imgp->width * imgp->height;

    getline(&line, &len, imgin);
    sscanf(line, "%d", &imgp->bitdepth);

    imgp->data = malloc(imgp->data_size * sizeof(PxRGB));
    int result = fread(imgp->data, sizeof(PxRGB), imgp->data_size, imgin);

    return 0;
}

int image_write_ppm(Image* imgp, FILE* imgout) {
    fprintf(imgout, "P6\n");
    fprintf(imgout, "%d %d\n", imgp->width, imgp->height);
    fprintf(imgout, "%d\n", imgp->bitdepth);
    fwrite(imgp->data, imgp->data_size, sizeof(PxRGB), imgout);
    return 0;
}

int image_invert(Image* imgp) {
    int i;

    for (i = 0; i < imgp->data_size; i++) {
        PxRGB* pix = &imgp->data[i];
        pix->r = 0xFF ^ pix->r;
        pix->g = 0xFF ^ pix->g;
        pix->b = 0xFF ^ pix->b;
    }

    return 0;
}

int image_scale_nn(Image* imgp, int nw, int nh) {
    int i;
    int nsize = nw * nh;
    double sx = (double)nw / imgp->width;
    double sy = (double)nh / imgp->height;

    PxRGB* datp = malloc(nsize * sizeof(PxRGB));

    for (i = 0; i < nsize; i++) {
        int nx = i % nw;
        int ny = i / nw;
        int ox = (int)(nx/sx);
        int oy = (int)(ny/sy);
        datp[i] = imgp->data[ox + oy * imgp->width];
    }

    free(imgp->data);
    imgp->data = datp;
    imgp->width = nw;
    imgp->height = nh;
    imgp->data_size = nsize;

    return 0;
}

int image_dither(Image* imgp, int bitdepth) {
    int i;
    int w = imgp->width;
    int h = imgp->height;
    PxRGB* px = imgp->data;

    unsigned char mask = 0xff << (8 - bitdepth);

    for (i = 0; i < imgp->data_size; i++) {
        unsigned char r = (px[i].r & mask) * (255.0/mask);
        unsigned char g = (px[i].g & mask) * (255.0/mask);
        unsigned char b = (px[i].b & mask) * (255.0/mask);

        int dr = r - px[i].r;
        int dg = g - px[i].g;
        int db = b - px[i].b;
        
        int x = i % imgp->width;
        int y = i / imgp->width;

        if (x != w) {
            px[i+1  ].r -= dr * 7.0/16.0;
            px[i+1  ].g -= dg * 7.0/16.0;
            px[i+1  ].b -= db * 7.0/16.0;
        }
        if (y != h-1) {
            if (x != 0) {
                px[i+w-1].r -= dr * 3.0/16.0;
                px[i+w-1].b -= db * 3.0/16.0;
                px[i+w-1].g -= dg * 3.0/16.0;
            }
            if (x != w) {
                px[i+w+1].r -= dr * 1.0/16.0;
                px[i+w+1].g -= dg * 1.0/16.0;
                px[i+w+1].b -= db * 1.0/16.0;
            }
            px[i+w  ].r -= dr * 5.0/16.0;
            px[i+w  ].g -= dg * 5.0/16.0;
            px[i+w  ].b -= db * 5.0/16.0;
        }

        px[i].r = r;
        px[i].g = g;
        px[i].b = b;
    }

    return 0;
}

int image_kernel_filter(Image* imgp, Kernel kernel) {
    int i;
    int j;
    int kw = kernel.size;
    double ksum = 0;
    PxRGB* data = malloc(imgp->data_size * sizeof(PxRGB));

    for (i = 0; i < imgp->data_size; i++) {
        double r = 0;
        double g = 0;
        double b = 0;
        ksum = 0;

        for (j = 0; j < kw*kw; j++) {
            int ix = (i % imgp->width) + (j % kw - kernel.size + 1);
            int iy = (i / imgp->width) + (j / kw - kernel.size + 1);
            int ii = imgp->width * iy + ix;

            if (ix < 0 || iy < 0) continue;
            if (ii >= imgp->data_size || ii < 0) continue;

            r += imgp->data[ii].r * kernel.data[j];
            g += imgp->data[ii].g * kernel.data[j];
            b += imgp->data[ii].b * kernel.data[j];
            ksum += kernel.data[j];
        }
        data[i].r = (int)CLAMP_8B(r / ksum);
        data[i].g = (int)CLAMP_8B(g / ksum);
        data[i].b = (int)CLAMP_8B(b / ksum);
    }

    free(imgp->data);
    free(kernel.data);
    imgp->data = data;

    return 0;
}

int image_resize(Image* imgp, int w, int h, int x, int y) {
    int i;
    PxRGB* data = malloc(w * h * sizeof(PxRGB));

    for ( i = 0; i < w * h; i++) {
        int ix = i % w - x;
        int iy = i / w - y;
        if (ix >= imgp->width || ix < 0) continue;
        if (iy >= imgp->height || iy < 0) continue;
        int ii = imgp->width * iy + ix;
        data[i] = imgp->data[ii];
    }

    free(imgp->data);
    imgp->data = data;
    imgp->width = w;
    imgp->height = h;
    imgp->data_size = w * h;

    return 0;
}

int image_compose(Image* img1p, Image* img2p, int x, int y) {
    int i;

    for ( i = 0; i < img2p->data_size; i++) {
        int ix = i % img2p->width + x;
        int iy = i / img2p->width + y;
        if (ix >= img1p->width || ix < 0) continue;
        if (iy >= img1p->height || iy < 0) continue;
        int ii = img1p->width * iy + ix;
        img1p->data[ii] = img2p->data[i];
    }

    return 0;
}

int image_contrast(Image* imgp, double x) {
    int i;

    for ( i = 0; i < imgp->data_size; i++) {
        imgp->data[i].r = CLAMP_8B(((imgp->data[i].r - 256/2) * x ) + 256/2);
        imgp->data[i].g = CLAMP_8B(((imgp->data[i].g - 256/2) * x ) + 256/2);
        imgp->data[i].b = CLAMP_8B(((imgp->data[i].b - 256/2) * x ) + 256/2);
    }

    return 0;
}

int image_grayscale(Image* imgp) {
    int i;

    for (i = 0; i < imgp->data_size; i++) {
        unsigned char luma = (imgp->data[i].r*65.0 + imgp->data[i].g*129.0 + imgp->data[i].b*25.0) / 219.0;
        imgp->data[i].r = luma;
        imgp->data[i].g = luma;
        imgp->data[i].b = luma;
    }
    
    return 0;
}

/*=-------------------------------------------------------------------------=*/

Kernel kernel_normalized(int size) {
    int i;
    Kernel k;
    k.size = size;
    k.data = malloc(k.size * k.size * sizeof(double));
    for (i = 0; i < size * size; i++) k.data[i] = 1;
    return k;
}

Kernel kernel_sobel(int size) {
    Kernel k;
    k.size = 3;
    k.data = malloc(k.size* k.size * sizeof(double));
    k.data[0] = 0; k.data[1] = 1;  k.data[2] = 0;
    k.data[3] = -1;  k.data[4] = 5;  k.data[5] = -1;
    k.data[6] = 0;  k.data[7] = -1;  k.data[8] = 0;
    return k;
}