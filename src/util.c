#include "util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static bool g_debug_on = false;

void sp_debug(const char *format, ...)
{
    if (!g_debug_on) return;

	va_list argp;
	va_start(argp, format);
	vfprintf(stderr, format, argp);
	va_end(argp);
	fputc('\n', stderr);
}

void sp_toggle_debug(bool on)
{
    g_debug_on = on;
}

void sp_info(const char *format, ...)
{
    va_list argp;
	va_start(argp, format);
	vfprintf(stdout, format, argp);
	va_end(argp);
	fputc('\n', stderr);
}

void sp_error(const char *format, ...)
{
    va_list argp;
	va_start(argp, format);
	vfprintf(stderr, format, argp);
	va_end(argp);
	fputc('\n', stderr);
	exit(1);
}

void sp_errno(const char *format, ...)
{
    int err = errno;
    
    va_list argp;
	va_start(argp, format);
	vfprintf(stderr, format, argp);
	va_end(argp);
    fprintf(stderr, "error: %s\n", strerror(err));
	exit(1);    
}
