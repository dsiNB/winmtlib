#include <windows.h>
#include "pool.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>

void* test(void* arg);
int tests = 0;
double t = 0;

int main() {
	wmt_queue* taskqueue = initTaskQueue(MAX_TASKS);
	wmt_pool* pool = initPool(taskqueue, MAX_THREADS);
	char* str = "Hello";
	t = (double)clock();
	for (int i = 0; i < 500; i++) {
		addTask(taskqueue, (wmt_task_func)test, (wmt_task_arg)str);
	}
	while (tests < 500) {  }
	printf(" time: %lf", (double)(clock()-t)/(double)CLOCKS_PER_SEC);
	return 0;
}

void* test(void* arg) {
	int i = 1;
	for (i; i < INT_MAX / 10; i++) {
		i += 1;
	}
	tests++;
	printf(" %d", tests);
	printf("%s", arg);
	return (void*)0;
}