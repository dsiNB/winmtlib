#include "pool.h"
#include <stdio.h>
#include <stdlib.h>

wmt_pool* initPool(wmt_queue* queue, int amount) {
	/*allocate memory*/
	wmt_pool* pool = malloc(sizeof(wmt_pool));
	if (!pool) { return(wmt_pool*)-1; }
	wmt_thread* threads = pool->threads;
	wmt_thread* thisThread;
	if (!pool) { return(wmt_pool*)-1; }
	threads = malloc(sizeof(wmt_thread) * amount);
	if(!threads) { return(wmt_pool*)-1; }

	/*create threads*/
	for (int t = 0; t < amount; t++) {
		thisThread = &threads[0];
		thisThread->id = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)tmain, queue,
			(DWORD)0, NULL);
	}

	printf("[pool init]\n");
	return pool;
}

wmt_queue* initTaskQueue(int amount) {
	/*init task queue*/
	wmt_queue* taskqueue = malloc(sizeof(wmt_queue));
	if (!taskqueue) { return (wmt_queue*)-1; }
	taskqueue->writepos = 0;
	taskqueue->readpos = 0;
	taskqueue->maxitems = amount;
	taskqueue->mutex = CreateMutex(NULL, FALSE, NULL);
	taskqueue->items = (wmt_queue_item*)malloc(sizeof(wmt_task) * amount); /*im trolling*/
	if (!taskqueue->items) { return (wmt_queue*)-1; }

	/*init mutexes*/
	wmt_task* thistask;
	for (int m = 0; m < amount; m++) {
		thistask = (wmt_task*)&taskqueue->items[m];
		thistask->func = NULL;
		thistask->arg = NULL;
		//thistask->mutex = CreateMutex(NULL, FALSE, NULL);
	}

	printf("[init task queue]");
	return taskqueue;
}

int addTask(wmt_queue* queue, wmt_task_func addr, wmt_task_arg arg) {
	wmt_task* thistask = (wmt_task*)&queue->items[queue->writepos];

	if (thistask->func == NULL) { /*if queue slot is available*/
		thistask->func = addr;
		inc_writepos(queue);
	}
	else {
		while (thistask->func != NULL) { /*printf("&");*/ } /*wait for current task in queue slot to be handled*/
		thistask->func = addr;
		inc_writepos(queue);
	}
	//printf("+");
	return 0;
}

void inc_writepos(wmt_queue* queue) {
	int* writepos = &queue->writepos;
	if (*writepos == queue->maxitems - 1) {
		*writepos = 0;
	}
	else {
		queue->writepos++;
	}
}

void inc_readpos(wmt_queue* queue) {
	int* readpos = &queue->readpos;
	if (*readpos == queue->maxitems - 1) {
		*readpos = 0;
	}
	else {
		queue->readpos++;
	}
}

void* tmain(wmt_queue* queue) {
	DWORD claimed;
	while (1) {
		claimed = WaitForSingleObject(queue->mutex, INFINITE);
		if (claimed == WAIT_OBJECT_0) {
			wmt_task* thistask = (wmt_task*)&queue->items[queue->readpos];

			if (thistask->func != NULL) { /*if queue slot is available*/
				void (*task)() = thistask->func;
				thistask->func = NULL;
				inc_readpos(queue);
				ReleaseMutex(queue->mutex);
				task();
			}
			else {
				while (thistask->func == NULL) { } /*wait for current task in queue slot to be handled*/
				void (*task)() = thistask->func;
				if (!task) {return (void*)-1;}
				thistask->func = NULL;
				inc_readpos(queue);
				ReleaseMutex(queue->mutex);
				task();
			}
			
		}	
	}
	return (void*)0;
}