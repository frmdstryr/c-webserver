/*
 * dbg.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jrm
 *
 *  Macros for logging and debugging
 *
 *  Taken and slightly modified from:
 *      http://c.learncodethehardway.org/book/ex20.html
 */


#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#ifdef NDEBUG
#define log_debug(M, ...)
#else
#define log_debug(M, ...) fprintf(stderr, "[DEBUG] (%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); }
#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define check(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); errno=0; goto error; }
#define sentinel(M, ...)  { log_error(M, ##__VA_ARGS__); errno=0; goto error; }
#define check_mem(A) check((A), "Out of memory.")
#define check_debug(A, M, ...) if(!(A)) { log_debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif
