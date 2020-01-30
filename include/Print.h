#ifndef __PRINT_H__
#define __PRINT_H__

#include <stdio.h> // printf() perror()
#include <string.h> // strrchr()
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>

#define PRINT_LEVEL 5 // 打印等级，打印到屏幕
#define OUTPUT_LEVEL 5 // 输出等级，输出到指定文件

#define LEVEL_DEBUG 5
#define LEVEL_INFO  4
#define LEVEL_WARN  3
#define LEVEL_ERROR 2
#define LEVEL_FATAL 1

#if (PRINT_LEVEL >= LEVEL_DEBUG)
    #define DEBUG(fmt, ...) common_printf(stdout, false, LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define DEBUG(fmt, ...)
#endif

#if (PRINT_LEVEL >= LEVEL_INFO)
    #define INFO(fmt, ...) common_printf(stdout, false, LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define INFO(fmt, ...)
#endif

#if (PRINT_LEVEL >= LEVEL_WARN)
    #define WARN(fmt, ...) common_printf(stdout, false, LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define WARN(fmt, ...)
#endif

#if (PRINT_LEVEL >= LEVEL_ERROR)
    #define ERROR(fmt, ...) common_printf(stdout, false, LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
    #define ERRORNO(fmt, ...) common_printf(stdout, true, LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define ERROR(fmt, ...)
    #define ERRORNO(fmt, ...)
#endif

#if (PRINT_LEVEL >= LEVEL_FATAL)
    #define FATAL(fmt, ...) common_printf(stdout, false, LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define FATAL(fmt, ...)
#endif

void _common_printf(FILE* fp, const bool err_no, const char* type_cl, const char* level, 
                 const char* file, const int line, const char* fmt);
int common_printf(FILE* fp, const bool err_no, const int level, 
              const char* file, const int line, const char* fmt, ...);
#endif
