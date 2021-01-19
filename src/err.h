#ifndef _ERR_H_
#define _ERR_H_

#define ERR_GENERIC "internal"

#define ERR_NO_ARG "no arguments"
#define ERR_NO_OPTION "no options"
#define ERR_NO_INPUT "no input file selected"
#define ERR_NO_OUTPUT "no output file selected"

#define ERR_FILE_OPEN "cannot open the file"
#define ERR_FILE_READ "cannot read ppm image, file might be corrupted"

#define ERR_OPTION_UNKNOWN "unknown option"
#define ERR_OPTION_INVALID "improper use of option"

void fail(char* code, char* comment);

#endif /* _ERR_H */