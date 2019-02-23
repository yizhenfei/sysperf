#ifndef SYSP_UTIL_H
#define SYSP_UTIL_H

#include <stdbool.h>

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
 * Bind current thread to a cpu core,
 * Return true if succeeded, otherwise false is returned and errno is set.
 */
bool sp_bind_cpu(unsigned cpu_id);

#endif /* #ifndef SYSP_UTIL_H */
