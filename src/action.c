#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "action.h"
#include "err.h"

/*=-------------------------------------------------------------------------=*/

int dispatch_invert(Image *imgp, ActionParams params) {
    return image_invert(imgp);
}

int dispatch_scale(Image *imgp, ActionParams params) {
    int x, y;
    if(params.argc != 3) return 1;
    sscanf(params.argv[1], "%d", &x);
    sscanf(params.argv[2], "%d", &y);
    return image_scale_nn(imgp, x, y);
}

int dispatch_dither(Image *imgp, ActionParams params) {
    return image_dither(imgp);
}

int dispatch_blur(Image *imgp, ActionParams params) {
    int x = 5;
    if (params.argc >= 2) sscanf(params.argv[1], "%d", &x);
    return image_kernel_filter(imgp, kernel_normalized(x));
}

int dispatch_sharpen(Image *imgp, ActionParams params) {
    int x = 5;
    if (params.argc >= 2) sscanf(params.argv[1], "%d", &x);
    return image_kernel_filter(imgp, kernel_sobel(x));
}

int dispatch_resize(Image *imgp, ActionParams params) {
    int w, h;
    int x = 0;
    int y = 0;
    if (params.argc >= 3) {
        sscanf(params.argv[1], "%d", &w);
        sscanf(params.argv[2], "%d", &h);
    }
    if (params.argc >= 5) {
        sscanf(params.argv[3], "%d", &x);
        sscanf(params.argv[4], "%d", &y);
    }
    return image_resize(imgp, w, h, x, y);
}

int dispatch_compose(Image *imgp, ActionParams params) {
    int x = 0;
    int y = 0;
    if (params.argc >= 4) {
        sscanf(params.argv[2], "%d", &x);
        sscanf(params.argv[3], "%d", &y);
    }
    Image i;
    image_read_ppm(&i, fopen(params.argv[1], "r"));
    int result = image_compose(imgp, &i, x, y);
    free(i.data);
    return result;
}

int dispatch_contrast(Image *imgp, ActionParams params) {
    double x = 0;
    if (params.argc >= 2) sscanf(params.argv[1], "%lf", &x);
    return image_contrast(imgp, x);
}

ActionArr DISPATCHERS[] = {
    {"-invert", dispatch_invert},
    {"-scale",  dispatch_scale},
    {"-dither",  dispatch_dither},
    {"-blur",  dispatch_blur},
    {"-sharpen",  dispatch_sharpen},
    {"-resize",  dispatch_resize},
    {"-compose",  dispatch_compose},
    {"-contrast",  dispatch_contrast},
};

ActionDispatcher match_dispatcher(char* label) {
    int i;
    int len = sizeof(DISPATCHERS)/sizeof(ActionArr);
    ActionArr* cur_disp = NULL;

    for (i = 0; i < len; i++) {
        if (!strcmp(DISPATCHERS[i].label, label)) return DISPATCHERS[i].disp;
    }

	fail(ERR_OPTION_UNKNOWN, label);
    return NULL;
}