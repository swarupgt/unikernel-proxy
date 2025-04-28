#ifndef UNIKERNEL_PROXY_LOGGER_H
#define UNIKERNEL_PROXY_LOGGER_H


#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

#define LOG_LEVEL_ACTIVE LOG_LEVEL_INFO

typedef struct LogContext {
    int log_level;
    int outfd; // for output stream 
    int errfd; // for error  stream
    int isStdLogger; // if output is to std stream
} LogContext;

void log_info(LogContext* ctx, const char* fmt, ...);
void log_debug(LogContext* ctx, const char* fmt, ...);
void log_warn(LogContext* ctx, const char* fmt, ...);
void log_error(LogContext* ctx, const char* fmt, ...);
void get_std_logger(LogContext* ctx);
int get_file_logger(LogContext* ctx, char* filename);

#endif