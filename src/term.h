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

/*************************************************************************
 * ASCII characters                                                      *
 *************************************************************************/
#define TPRINT_ROWS 0
#define TPRINT_COLS 0
#define TPRINT_CLEAR() printf("\033[H\033[J")
#define TPRINT_GOTO_TOPLEFT() printf("\033[0;0H")
#define TPRINT_HIDE_CURSOR() printf("\e[?25l")
#define TPRINT_SHOW_CURSOR() printf("\e[?25h")
#define _TPRINT_GET_SIZE() do { \
    struct winsize w;                                                     \
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {                     \
        perror("ERROR: ioctl can't capture terminal's rows/columns.");    \
        return 1;                                                         \
    }                                                                     \
    g_termRows = w.ws_row;                                                \
    g_termCols = w.ws_col;                                                \
} while(0)

#define TPRINT_INIT() do {                                                \
    TPRINT_CLEAR();                                                       \
    TPRINT_HIDE_CURSOR();                                                 \
    TPRINT_GOTO_TOPLEFT();                                                \
    _TPRINT_GET_SIZE();                                                   \
} while(0)

#define TPRINT_PRINT_AT(x, y, c) printf("\033[%d;%dH%c", (y), (x), (c));

/* size must be at least the rows, colunms specified */
#define ASSERT_SIZE_OK(rows, cols) assert((rows) <= g_termRows && (cols) <= g_termCols)

/**************************************************************************
 * Unicode characters                                                     *
 **************************************************************************/
#define TPRINT_WCLEAR() wprintf(L"\033[H\033[J");
#define TPRINT_WINIT() do {                                               \
    setlocale(LC_ALL, "");                                                \
    wprintf(L"\033[H\033[J");                                             \
    TPRINT_HIDE_CURSOR();                                                 \
    _TPRINT_GET_SIZE();                                                   \
    /* hack to take up the whole space - print blank char at the corner*/ \
    const wchar_t blank = L'\u00A0';                                      \
    TPRINT_WPRINT_AT(g_termCols, g_termRows, blank);                      \
} while(0)

#define TPRINT_WPRINT_AT(x, y, wc) wprintf(L"\033[%d;%dH%lc", (y), (x), (wc))

#define TPRINT_PIXEL(x, y) do {                                           \
    const wchar_t full = L'\u2588';                                      \
    TPRINT_WPRINT_AT(2*(x), (y), full);                                   \
    TPRINT_WPRINT_AT(2*(x)+1, (y), full);                                 \
} while(0)

#define TPRINT_ERASE(x, y) do {                                          \
    const wchar_t blank = L'\u00A0';                                      \
    TPRINT_WPRINT_AT(2*(x), (y), blank);                                  \
    TPRINT_WPRINT_AT(2*(x)+1, (y), blank);                                \
} while(0)


/* terminal's rows */
static int g_termRows = 0;
/* terminal's columns */
static int g_termCols = 0;

} // extern "C"
#endif // __cplusplus

#endif // TERM_H
