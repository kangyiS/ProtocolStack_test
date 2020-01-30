#include "Print.h"

#define ANSI_COLOR_RED         "\x1b[1;31m"
#define ANSI_COLOR_GREEN       "\x1b[1;32m"
#define ANSI_COLOR_YELLOW      "\x1b[1;33m"
#define ANSI_COLOR_PURPLE      "\x1b[1;35m"
#define ANSI_COLOR_WHITE       "\x1b[1;37m"
#define ANSI_COLOR_CYAN        "\x1b[29m"
#define ANSI_COLOR_CYAN_LESS   "\x1b[29m"
#define ANSI_COLOR_RESET       "\x1b[0m"

#define PRINT_DEBUG            "DEBUG"
#define PRINT_INFO             "INFO"
#define PRINT_WARN             "WARN"
#define PRINT_ERROR            "ERROR"
#define PRINT_FATAL            "FATAL"
#define PRINT_UNKNOWN          "UNKNOWN"

#define PRINT_LEN 1024*10  // 一次最多可打印输出10KB的数据

void _common_printf(FILE* fp, const bool err_no, const char* type_cl, const char* level, 
                 const char* file, const int line, const char* fmt)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char datestr[20];
    struct tm tm;
    time_t timesec = tv.tv_sec;
    localtime_r(&timesec, &tm);
    strftime(datestr, sizeof(datestr), "%Y-%m-%d %H:%M:%S", &tm);
    // file是绝对路径，打印出来太长，使用strrchr()找到最后一个'/'，后面的就是文件名
    const char* file_name = strrchr(file, '/');
    file_name++;
    if(err_no == false)
    {
        fprintf(fp, "%s[%s %s %s:%d] %s%s", type_cl, datestr, level, file_name, line, fmt, ANSI_COLOR_RESET);
        fflush(fp);
    }
    else
    {
        fprintf(fp, "%s[%s %s %s:%d] %s: %s%s", type_cl, datestr, level, file_name, line, fmt, strerror(errno), ANSI_COLOR_RESET);
        fflush(fp);
    }
}

int common_printf(FILE* fp, const bool err_no, const int level, const char* file,
                         const int line, const char* fmt, ...)
{
    int res = 0;
    char buf[PRINT_LEN];
    
    va_list args;    
    va_start(args, fmt);
    res = vsnprintf(buf, PRINT_LEN, fmt, args);
    va_end(args);

    if(level == LEVEL_DEBUG)
    {
        _common_printf(fp, err_no, ANSI_COLOR_GREEN, PRINT_DEBUG, file, line, buf);
    }
    else if(level == LEVEL_INFO)
    {
        _common_printf(fp, err_no, ANSI_COLOR_WHITE, PRINT_INFO, file, line, buf);
    }
    else if(level == LEVEL_WARN)
    {
        _common_printf(fp, err_no, ANSI_COLOR_YELLOW, PRINT_WARN, file, line, buf);
    }
    else if(level == LEVEL_ERROR)
    {
        _common_printf(fp, err_no, ANSI_COLOR_RED, PRINT_ERROR, file, line, buf);
    }
    else if(level == LEVEL_FATAL)
    {
        _common_printf(fp, err_no, ANSI_COLOR_PURPLE, PRINT_FATAL, file, line, buf);
    }
    else
    {
        _common_printf(fp, err_no, ANSI_COLOR_WHITE, PRINT_UNKNOWN, file, line, buf);
    }

    return res;
}


