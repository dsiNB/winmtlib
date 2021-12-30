#ifndef WINMTLIB_POOL_H
#define WINMTLIB_POOL_H

#include <windows.h>
typedef void* wmt_task_func;
typedef void* wmt_task_arg;

typedef struct _wmt_thread {
	HANDLE id;
} wmt_thread;

typedef struct _wmt_pool {
	wmt_thread* threads;
	unsigned int size;
} wmt_pool;

typedef struct _wmt_task {
	wmt_task_func func;
	wmt_task_arg arg;
} wmt_task;

typedef struct _wmt_queue {

} wmt_queue;
#endif