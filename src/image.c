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
    if (!strcmp(line, "P6")) fail(ERR_FILE_READ, "");

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
}

int image_dither(Image* imgp) {
    int i;
    int w = imgp->width;
    int h = imgp->height;
    PxRGB* px = imgp->data;

    for (i = 0; i < imgp->data_size; i++) {
        int oluma = (px[i].g + px[i].g + px[i].b) / 3;
        int nluma = oluma > 127 ? 255 : 0;
        int dluma = oluma - nluma;
        if(i%w)
            px[i+1].g += MIN(dluma * 7.0/16.0, 255);
        if(i/w != h) {
            px[i+w-1].g += MIN(dluma * 3.0/16.0, 255);
            px[i+w  ].g += MIN(dluma * 5.0/16.0, 255);
            px[i+w+1].g += MIN(dluma * 1.0/16.0, 255);
        }

        px[i].r = nluma;
        px[i].g = nluma;
        px[i].b = nluma;
    }
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
}

int image_compose(Image* img1p, Image* img2p, int x, int y) {
    int i;

    for ( i = 0; i < img2p->data_size; i++) {
        int ix = i % img2p->width + x;
        int iy = i / img2p->height + y;
        if (ix >= img1p->width || ix < 0) continue;
        if (iy >= img1p->height || iy < 0) continue;
        int ii = img1p->width * iy + ix;
        img1p->data[ii] = img2p->data[i];
    }
}

int image_contrast(Image* imgp, double x) {
    int i;

    for ( i = 0; i < imgp->data_size; i++) {
        imgp->data[i].r = CLAMP_8B(((imgp->data[i].r - 256/2) * x ) + 256/2);
        imgp->data[i].g = CLAMP_8B(((imgp->data[i].g - 256/2) * x ) + 256/2);
        imgp->data[i].b = CLAMP_8B(((imgp->data[i].b - 256/2) * x ) + 256/2);
    }
}

/* TODO: implement */

int image_brightness(Image* imgp, int db) {
    int i;
    for (i = 0; i < imgp->data_size; i++) {
        imgp->data->r += db;
        imgp->data->g += db;
        imgp->data->b += db;
    }
}

int image_hue(Image* imgp, int dh) {

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