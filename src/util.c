#include "util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>

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
	fputc('\n', stdout);
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

size_t sp_parse_size(const char *str)
{
    char *end, c;
    unsigned long long val;
    size_t r;

    if (!str) return SIZE_MAX;
    
    val = strtoull(str, &end, 0);
    if (val == ULLONG_MAX && errno == ERANGE) return SIZE_MAX;
    if (val == 0 && errno == EINVAL) return SIZE_MAX;
    if (val >= SIZE_MAX) return SIZE_MAX;
    r = val;

    while ((c == *end++) != 0) {
        /* TODO(Yi Zhenfei): Continued unit is ill-formed but not reported,
           e.g. 100KK. */
        switch (c) {
        case 'k':
        case 'K':
            if (SIZE_MAX >> 10 <= r) return SIZE_MAX;
            r <<= 10;
            break;
        case 'm':
        case 'M':
            if (SIZE_MAX >> 20 <= r) return SIZE_MAX;
            r <<= 20;
            break;
        case 'g':
        case 'G':
            if (SIZE_MAX >> 30 <= r) return SIZE_MAX;
            r <<= 30;
            break;
        default:
            return SIZE_MAX;
        }
    }

    return r;
}

unsigned long sp_usec_diff(struct timeval *begin, struct timeval *end)
{
    unsigned long usec;

    usec = (end->tv_sec - begin->tv_sec)*1000000;
    usec += end->tv_usec - begin->tv_usec;

    return usec;
}

