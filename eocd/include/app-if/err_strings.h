#ifndef ERROR_STRINGS_H
#define ERROR_STRINGS_H
#include <app-if/err_codes.h>

char *err_strings[] = {
"",
"Wrong type of request",
"Channel already exist",
"Channel not exist",
"No onfiguration profiles aviliable",
"No memory",
"Cannot find device",
"Some of command parameters is wrong",
"Wrong configuration profile name",
"Configuration profile not exist",
"Configuration profile already exist",
"Configuration profile is in use"
};

#define ERR_STR_SIZE sizeof(err_codes)

#endif
