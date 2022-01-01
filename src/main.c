#include <windows.h>
#include "pool.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>

void test();
int tests = 0;
double t = 0;

int main() {
	wmt_queue* taskqueue = initTaskQueue(MAX_TASKS);
	wmt_pool* pool = initPool(taskqueue, MAX_THREADS);
	t = (double)clock();
	for (int i = 0; i < 1000; i++) {
		addTask(taskqueue, (wmt_task_func)test, (wmt_task_arg)NULL);
	}
	while (tests < 100) {  }
	printf(" time: %lf", (double)(clock()-t)/(double)CLOCKS_PER_SEC);
	return 0;
}

void test() {
	int i = 1;
	void* m;
	for (i; i < INT_MAX / 10; i++) {
		m = malloc(sizeof(int) * 100);
	}
	tests++;
	return;
}