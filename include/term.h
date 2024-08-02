#ifndef TERM_H
#define TERM_H

#include <cstdio>

#ifdef __cplusplus

/*
 * Description:
 *
 * Header to print at specific coordinates on the terminal. Requires a POSIX system.
 * Does not work on Windows.
 */
extern "C" {
#include <sys/ioctl.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>

#define TPRINT_CLEAR() printf("\033[H\033[J")
#define TPRINT_GOTO_TOPLEFT() printf("\033[0;0H")
#define TPRINT_HIDE_CURSOR() printf("\e[?25l")
#define TPRINT_SHOW_CURSOR() printf("\e[?25h")
#define TPRINT_INIT() do {                                                \
    TPRINT_CLEAR();                                                       \
    TPRINT_HIDE_CURSOR();                                                 \
    TPRINT_GOTO_TOPLEFT();                                                \
    _TPRINT_GET_SIZE();                                                   \
} while(0)

#define TPRINT_PRINT_AT(x, y, c) printf("\033[%d;%dH%c", (y), (x), (c));
} // extern "C"
#endif // __cplusplus

#endif // TERM_H
