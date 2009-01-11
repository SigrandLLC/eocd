#ifndef ERROR_STRINGS_H
#define ERROR_STRINGS_H
#include <app-if/err_codes.h>

static char *err_strings[] = { "", "Wrong type of request",
	"Channel already exist", "Channel not exist",
	"No onfiguration profiles aviliable", "No memory", "Cannot find device",
	"Some of command parameters is wrong", "Configuration profile not exist",
	"Configuration profile already exist", "Configuration profile is in use",
	"Channel not maintain SHDSL data base", "Unexpected error",
	"Requested element not exist",
	"Some settings of profile not compatible with devices",
	"Profile is read-only", "Finction is not implemented" };

#define ERR_STR_SIZE sizeof(err_codes)

#endif
