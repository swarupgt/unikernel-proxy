#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>  
#include <time.h>


void log_info(LogContext* ctx, const char* fmt, ...) {
    if (ctx == NULL || fmt == NULL) {
        return;
    }

    int fd = ctx->outfd;

    // write tag
    const char* tag = NULL;
    if (ctx->isStdLogger) {
        tag = "\033[32m[info] \033[0m";
    } else {
        tag = "[info] ";
    }
    write(fd, tag, strlen(tag));

    // write time
    char timebuf[64];
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now); 
    strftime(timebuf, sizeof(timebuf), "[%Y-%m-%d %H:%M:%S] ", &tm_now);
    write(fd, timebuf, strlen(timebuf));

    // write log message
    char buf[1024]; 
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    write(fd, buf, strlen(buf));

    // done
}


void log_debug(LogContext* ctx, const char* fmt, ...) {
    if ( ctx == NULL || fmt == NULL ) {
        return;
    }

    int fd = ctx->outfd;

    // write tag
    char* tag = NULL;
    if ( ctx->isStdLogger ) {
        tag = "\033[1;33m[debug] \033[1;0m";
    } else {
        tag = "[debug] ";
    }
    write(fd, tag, strlen(tag));

    // write time
    char timebuf[64];
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);  
    strftime(timebuf, sizeof(timebuf), "[%Y-%m-%d %H:%M:%S] ", &tm_now);
    write(fd, timebuf, strlen(timebuf));

    // write log message
    char buf[1024]; 
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    write(fd, buf, strlen(buf));

    // done
}


void log_warn(LogContext* ctx, const char* fmt, ...) {
    if ( ctx == NULL || fmt == NULL ) {
        return;
    }

    int fd = ctx->errfd;

    // write tag
    char* tag = NULL;
    if ( ctx->isStdLogger ) {
        tag = "\033[1;35m[warning] \033[1;0m";
    } else {
        tag = "[warning] ";
    }
    write(fd, tag, strlen(tag));

    // write time
    char timebuf[64];
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now); 
    strftime(timebuf, sizeof(timebuf), "[%Y-%m-%d %H:%M:%S] ", &tm_now);
    write(fd, timebuf, strlen(timebuf));

    // write log message
    char buf[1024]; 
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    write(fd, buf, strlen(buf));

    // done
}

void log_error(LogContext* ctx, const char* fmt, ...) {
    if ( ctx == NULL || fmt == NULL ) {
        return;
    }

    int fd = ctx->errfd;

    // write tag
    char* tag;
    if ( ctx->isStdLogger ) {
        tag = "\033[1;31m[error] \033[1;0m";
    } else {
        tag = "[error] ";
    }
    write(fd, tag, strlen(tag));

    // write time
    char timebuf[64];
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);  
    strftime(timebuf, sizeof(timebuf), "[%Y-%m-%d %H:%M:%S] ", &tm_now);
    write(fd, timebuf, strlen(timebuf));

    // write log message
    char buf[1024]; 
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    write(fd, buf, strlen(buf));
}


void get_std_logger(LogContext* ctx) {
    ctx->outfd = fileno(stdout);
    ctx->errfd = fileno(stderr);
    ctx->isStdLogger = 1;
}

int get_file_logger(LogContext* ctx, char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd < 0 ){
        exit(1);
    }
    ctx->isStdLogger = 0;
    ctx->errfd = fd;
    ctx->outfd = fd;
    return fd;
}

