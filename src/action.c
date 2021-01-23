#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "action.h"
#include "err.h"

#define FAIL_OPTION fail(ERR_OPTION_INVALID, params.argv[0])
#define PARAM_ASSERT_LOAD(index, pattern, ptr) if (1 != sscanf(params.argv[index], pattern, ptr)) FAIL_OPTION

/*=-------------------------------------------------------------------------=*/

int dispatch_invert(Image *imgp, ActionParams params) {
    if (params.argc != 1) FAIL_OPTION;
    return image_invert(imgp);
}

int dispatch_scale(Image *imgp, ActionParams params) {
    int x, y;

    if (params.argc != 3) FAIL_OPTION;
    PARAM_ASSERT_LOAD(1, "%d", &x);
    PARAM_ASSERT_LOAD(2, "%d", &y);

    return image_scale_nn(imgp, x, y);
}

int dispatch_dither(Image *imgp, ActionParams params) {
    int x;

    if (params.argc != 2) FAIL_OPTION;
    PARAM_ASSERT_LOAD(1, "%d", &x);
    if (x <= 0) x = 1;
    if (x > 8) x = 8;

    return image_dither(imgp, x);
}

int dispatch_blur(Image *imgp, ActionParams params) {
    int x;

    if (params.argc != 2) FAIL_OPTION;
    PARAM_ASSERT_LOAD(1, "%d", &x);
    
    return image_kernel_filter(imgp, kernel_normalized(x));
}

int dispatch_sharpen(Image *imgp, ActionParams params) {
    int x;

    if (params.argc != 1) FAIL_OPTION;
    /* if (sscanf(params.argv[1], "%d", &x) != 1) FAIL_OPTION; */

    return image_kernel_filter(imgp, kernel_sobel(x));
}

int dispatch_resize(Image *imgp, ActionParams params) {
    int w, h;
    int x = 0;
    int y = 0;

    if (params.argc != 3 && params.argc != 5) FAIL_OPTION;

    PARAM_ASSERT_LOAD(1, "%d", &w);
    PARAM_ASSERT_LOAD(2, "%d", &h);

    if (params.argc == 5) {
        PARAM_ASSERT_LOAD(3, "%d", &x);
        PARAM_ASSERT_LOAD(4, "%d", &y);
    }

    return image_resize(imgp, w, h, x, y);
}

int dispatch_compose(Image *imgp, ActionParams params) {
    int x = 0;
    int y = 0;
    
    if (params.argc != 2 && params.argc != 4) FAIL_OPTION;
    
    if (params.argc == 4) {
        PARAM_ASSERT_LOAD(2, "%d", &x);
        PARAM_ASSERT_LOAD(3, "%d", &y);
    }
    
    Image i;
    FILE *imgin = fopen(params.argv[1], "r");
    if (!imgin) fail(ERR_FILE_OPEN, params.argv[1]);
    image_read_ppm(&i, imgin);
    int result = image_compose(imgp, &i, x, y);
    free(i.data);

    return result;
}

int dispatch_contrast(Image *imgp, ActionParams params) {
    double x = 0;
    if (params.argc != 2) FAIL_OPTION;
    PARAM_ASSERT_LOAD(1, "%lf", &x);
    return image_contrast(imgp, x);
}

int dispatch_grayscale(Image *imgp, ActionParams params) {
    if (params.argc != 1) FAIL_OPTION;
    return image_grayscale(imgp);
}

ActionArr DISPATCHERS[] = {
    {"--invert", dispatch_invert},
    {"--scale",  dispatch_scale},
    {"--dither",  dispatch_dither},
    {"--blur",  dispatch_blur},
    {"--sharpen",  dispatch_sharpen},
    {"--resize",  dispatch_resize},
    {"--compose",  dispatch_compose},
    {"--contrast",  dispatch_contrast},
    {"--grayscale",  dispatch_grayscale},
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