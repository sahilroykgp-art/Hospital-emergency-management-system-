#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ANSI Terminal Color Codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"
#define COLOR_GRAY    "\033[0;90m"

/* Enable Virtual Terminal Processing on Windows */
static inline void init_terminal_colors(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

/* Clear Screen Helper via ANSI escape sequence (portable, fast, no subshell) */
static inline void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

/* Case-insensitive string equality (Pure C11 standard) */
static inline bool string_equals_ci(const char *s1, const char *s2) {
    if (!s1 || !s2) return false;
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return false;
        }
        s1++;
        s2++;
    }
    return *s1 == '\0' && *s2 == '\0';
}

/* Format current time into human-readable string: YYYY-MM-DD HH:MM:SS */
static inline void get_current_timestamp(char *buffer, size_t size) {
    if (!buffer || size == 0) return;
    time_t raw_time = time(NULL);
    struct tm *time_info = localtime(&raw_time);
    if (time_info) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
    } else {
        snprintf(buffer, size, "Unknown");
    }
}

/* Safe string input with newline trimming and stdin cleanup */
static inline bool get_input_string(char *dest, size_t max_len) {
    if (!dest || max_len == 0) return false;
    if (fgets(dest, (int)max_len, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\n') {
            dest[len - 1] = '\0';
        } else if (!feof(stdin)) {
            /* flush excess characters in buffer */
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        return true;
    } else {
        dest[0] = '\0';
        return false;
    }
}

/* Safe integer input with range validation */
static inline int get_input_int(const char *prompt, int min_val, int max_val) {
    char buffer[64];
    int val = 0;
    while (true) {
        if (feof(stdin)) {
            return max_val; /* Return max_val (e.g. Exit) safely on EOF */
        }
        if (prompt) {
            printf("%s", prompt);
        }
        if (!get_input_string(buffer, sizeof(buffer)) && feof(stdin)) {
            return max_val;
        }
        if (strlen(buffer) == 0) {
            printf(COLOR_RED "  Invalid input. Please enter a number between %d and %d.\n" COLOR_RESET, min_val, max_val);
            continue;
        }
        char *endptr;
        val = (int)strtol(buffer, &endptr, 10);
        if (endptr != buffer && *endptr == '\0' && val >= min_val && val <= max_val) {
            return val;
        }
        printf(COLOR_RED "  Invalid input (%s). Must be between %d and %d.\n" COLOR_RESET, buffer, min_val, max_val);
    }
}

/* Press Enter to continue prompt */
static inline void pause_console(void) {
    if (feof(stdin)) return;
    printf("\n" COLOR_GRAY "Press [Enter] to return to menu..." COLOR_RESET);
    char buf[8];
    get_input_string(buf, sizeof(buf));
}

#endif /* COMMON_H */
