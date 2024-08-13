#include "log.h"

static struct {
	int level;
} L;

static const char *LEVEL_NAMES[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

void log_set_level(int level) {
	L.level = level;
}

int find_num_digits(int num) {
    if (num == 0) return 0;
    return (1 + find_num_digits(num / 10));
}

void log_log(int level, const char *function, int function_len, int line, const char *fmt, ...) {
	if (level < L.level) {
		return;
	}

    const int MAX_FUNC_LEN = 20;
    int line_len = find_num_digits(line);
    int padding_len = MAX_FUNC_LEN - function_len - line_len;
    if (padding_len < 0) padding_len = 0;

    int fn_len = (function_len + line_len) > MAX_FUNC_LEN ? MAX_FUNC_LEN - 1 - line_len : MAX_FUNC_LEN - 1;

	printf("%-5s %.*s:%d%*s : ", LEVEL_NAMES[level], fn_len, function, line, padding_len, "");

	va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\r\n");
}
