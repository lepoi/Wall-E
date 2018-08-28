#define C_RED	"\e[1;31m"
#define C_GRN	"\e[1;32m"
#define C_BLU	"\e[1;34m"
#define C_YLW	"\e[1;33m"
#define C_RST	"\e[0m"

#include <stdio.h>
#include <stdarg.h>

void line_number_log(unsigned int line_number) {
	printf(C_BLU"[%d] "C_RST, line_number);
}

void error_log(unsigned int line_number, const char *message, ...) {
	va_list arglist;
	printf(C_RED"[ERROR]"C_RST);
	line_number_log(line_number);
	va_start(arglist, message);
	vprintf(message, arglist);
	va_end(arglist);
	printf("\n");
}
