#ifndef WINMTLIB_H
#define WINMTLIB_H

#include <windows.h>
typedef void* wmt_task_func;
typedef void* wmt_task_arg;
typedef void* wmt_item;
typedef void* wmt_stack_item_ptr;

#define MAX_TASKS 1000
#define MAX_THREADS 8

typedef struct _wmt_thread {
	HANDLE id;
} wmt_thread;

typedef struct _wmt_pool {
	wmt_thread* threads;
	int size;
} wmt_pool;

typedef struct _wmt_task {
	wmt_task_func func;
	wmt_task_arg arg;
} wmt_task;

typedef struct _wmt_queue {
	wmt_item* items;
	int maxitems;
	int writepos;
	int readpos;
	HANDLE mutex;
} wmt_queue;

typedef struct _wmt_stack {
	wmt_item* items;
	int maxitems;
	int sp; /*stack pointer*/
	HANDLE mutex;
} wmt_stack;

wmt_pool* initPool(wmt_queue* queue, int amount);
wmt_queue* initTaskQueue(int amount);
void* tmain(wmt_queue* queue);
int addTask(wmt_queue* queue, wmt_task_func addr, wmt_task_arg arg);
void inc_writepos(wmt_queue* queue);
void inc_readpos(wmt_queue* queue);

wmt_stack* initStack(int size);
wmt_stack_item_ptr wmt_stack_pop(wmt_stack* stk);
int wmt_stack_push(wmt_stack* stk, wmt_stack_item_ptr itemptr);
#endif