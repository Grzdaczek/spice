#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "action.h"
#include "err.h"

/*=-------------------------------------------------------------------------=*/

void print_help() {

    printf("#=- Simple PPM image console editor -=#\n");
    printf("Usage:\n");
    printf("  ./spice <input-file> <output-file> [options]\n");
    printf("Options:\n");
    printf("  -scale <width> <height>     Scale the image to target dimentions\n");
    printf("  -resize <width> <height> <x> <y> Resize the canvas without scaling the image\t\n");
    printf("  -compose <path> <x> <y>     Paste the image file at <path> to target position\n");
    printf("  -contrast <amount>          Change contrast of the image by <amount>\n");
    printf("  -sharpen                    Apply sharpening\n");
    printf("  -blur <amount>              Apply box average blur\n");
    printf("  -invert                     Invert image colors\n");
    printf("  -dither                     Apply dithering\n");

    exit(0);
}

int main(int argc, char** argv) {
    int i, j, k;
    char* curr_arg = NULL;
    int queue_len = 0;
    ActionParams queue[256];

    if (argc == 1) print_help();
    if (argc < 3) fail(ERR_NO_OUTPUT, "");

    FILE* imgin = fopen(argv[1], "rb");
    FILE* imgout = fopen(argv[2], "wb");

    if (imgin == NULL) fail(ERR_FILE_OPEN, argv[1]);
    if (imgout == NULL) fail(ERR_FILE_OPEN, argv[2]);
  
    for (i = 3; i < argc; i++) {
        if (argv[i][0] == '-') {
            queue_len += 1;
            queue[queue_len-1].argc = 1;
            queue[queue_len-1].argv = &argv[i];
        }
        else {
            queue[queue_len-1].argc += 1;
        }
    }

    Image img;
    img.data = NULL;
    image_read_ppm(&img, imgin);

    for (i = 0; i < queue_len; i++) {
        ActionDispatcher dispatcher = match_dispatcher(queue[i].argv[0]);
        if (dispatcher) {
            dispatcher(&img, queue[i]);
        }
        else {
            fail(ERR_OPTION_UNKNOWN, "");
        }
        
    }

    image_write_ppm(&img, imgout);
    free(img.data);

    return 0;
}
