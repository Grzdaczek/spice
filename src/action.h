#ifndef _ACTION_H_
#define _ACTION_H_
#include "image.h"

typedef struct {
    int argc;
    char** argv;
} ActionParams;

typedef int (*ActionDispatcher)(Image *image, ActionParams params);

typedef struct {
    const char* label;
	ActionDispatcher disp;
} ActionArr;

ActionDispatcher match_dispatcher(char* label);

#endif /* _ACTION_H_ */