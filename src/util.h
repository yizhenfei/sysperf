#ifndef SYSP_UTIL_H
#define SYSP_UTIL_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus__
extern "C" {
#endif /* #ifdef __cplusplus__ */

/* Print debug message. */
void sp_debug(const char *format, ...);
/* Toggle debug message. */
void sp_toggle_debug(bool on);
/* Print informative message. */
void sp_info(const char *format, ...);
/* Print error message and exit the program. */
void sp_error(const char *format, ...);
/* Print error message with current `errno` and exit the program. */
void sp_errno(const char *format, ...);

/*
 * Parse size number from str.
 * Unit character K/M/G (and k/m/g) are supported and
 * calculated as 2^10, 2^20, 2^30 respectively.
 * If the input is ill-formed, SIZE_MAX is returned,
 * and thus, SIZE_MAX itself is considered invalid input :-)
 */
size_t sp_parse_size(const char *str);

/*
 * Calculate the diff between timeval begin and end,
 * end should be greater(later) than begin.
 */
struct timeval;
unsigned long sp_usec_diff(struct timeval *begin, struct timeval *end);

#ifdef __cplusplus__
}
#endif /* #ifdef __cplusplus__ */

#endif /* #ifndef SYSP_UTIL_H */
