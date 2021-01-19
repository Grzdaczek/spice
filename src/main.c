#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "action.h"
#include "err.h"

/*=-------------------------------------------------------------------------=*/

void print_help() {

    printf(">-- Simple portable Pixmap Image Console Editor --<\n");
    printf("Usage:\n");
    printf("  ./spice {--help|-h}         Print help\n");
    printf("  ./spice <input-file> <output-file> [options]\n");
    printf("                              Apply sequence of [options] to <input-file> image and save\n");
    printf("                              result to the <output-file> image\n");
    printf("Options:\n");
    printf("  -contrast <amount>          Change contrast of the image by <amount>\n");
    printf("  -sharpen                    Apply sharpening\n");
    printf("  -blur <radius>              Apply box average blur\n");
    printf("  -invert                     Invert image colors\n");
    printf("  -dither                     Apply dithering\n");
    printf("  -scale <width> <height>     Scale the image to target dimentions\n");
    printf("  -compose <path> <?x> <?y>   Paste the image file at <path> at target position\n");
    printf("  -resize <width> <height> <?x> <?y>\t\n");
    printf("                              Resize the canvas without scaling the image,\t\n");
    printf("                              and move the image to target position\t\n");
    
    exit(0);
}

int main(int argc, char** argv) {
    int i, j, k;
    char* curr_arg = NULL;
    int queue_len = 0;
    ActionParams queue[256];

    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) print_help();

    if (argc < 2) fail(ERR_NO_INPUT, "");
    FILE* imgin = fopen(argv[1], "rb");
    if (imgin == NULL) fail(ERR_FILE_OPEN, argv[1]);

    if (argc < 3) fail(ERR_NO_OUTPUT, "");
    FILE* imgout = fopen(argv[2], "wb");
    if (imgout == NULL) fail(ERR_FILE_OPEN, argv[2]);
  
    for (i = 3; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == '-') {
            queue_len += 1;
            queue[queue_len-1].argc = 1;
            queue[queue_len-1].argv = &argv[i];
        }
        else {
            if (i == 3) fail(ERR_OPTION_UNKNOWN, argv[i]);
            queue[queue_len-1].argc += 1;
        }
    }

    Image img;
    img.data = NULL;
    image_read_ppm(&img, imgin);

    for (i = 0; i < queue_len; i++) {
        ActionDispatcher dispatcher = match_dispatcher(queue[i].argv[0]);
        if (dispatcher) {
            if (dispatcher(&img, queue[i]))
                fail(ERR_OPTION_INVALID, queue[i].argv[0]);
        }
        else {
            fail(ERR_OPTION_UNKNOWN, queue[i].argv[0]);
        }
    }

    image_write_ppm(&img, imgout);
    free(img.data);

    return 0;
}
