#include "winmtlib.h"
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
	taskqueue->items = (wmt_item*)malloc(sizeof(wmt_task) * amount); /*im trolling*/
	if (!taskqueue->items) { return (wmt_queue*)-1; }

	/*init mutexes*/
	wmt_task* thistask;
	for (int m = 0; m < amount; m++) {
		thistask = (wmt_task*)&taskqueue->items[m];
		thistask->func = NULL;
		thistask->arg = NULL;
	}

	printf("[init task queue]");
	return taskqueue;
}

int addTask(wmt_queue* queue, wmt_task_func addr, wmt_task_arg arg) {
	wmt_task* thistask = (wmt_task*)&queue->items[queue->writepos];

	if (thistask->func == NULL) { /*if queue slot is available*/
		thistask->func = addr;
		thistask->arg = arg;
		inc_writepos(queue);
	}
	else {
		while (thistask->func != NULL) { /*printf("&");*/ } /*wait for current task in queue slot to be handled*/
		thistask->func = addr;
		thistask->arg = arg;
		inc_writepos(queue);
	}
	//printf("+");
	return 0;
}

void inc_writepos(wmt_queue* queue) {
	int* writepos = &queue->writepos;
	_asm {
		mov esi, writepos
		mov eax, dword ptr[esi] //*readpos

		mov ebx, queue
		mov ecx, dword ptr [ebx+4] //maxitems value
		dec ecx

		mov ebx, eax
		inc eax //queue->readpos++
		xor edx, edx
		cmp ecx, ebx
		cmove eax, edx
		mov dword ptr [esi], eax
	}
	/*if (*writepos == queue->maxitems - 1) {
		*writepos = 0;
	}
	else {
		queue->writepos++;
	}*/
}

void inc_readpos(wmt_queue* queue) {
	int* readpos = &queue->readpos;
	_asm {
		mov esi, readpos
		mov eax, dword ptr [esi] //*readpos
		
		mov ebx, queue
		mov ecx, dword ptr [ebx+4] //maxitems value
		dec ecx

		mov ebx, eax
		inc eax //queue->readpos++
		xor edx, edx
		cmp ecx, ebx
		cmove eax, edx
		mov dword ptr [esi], eax
	}

/*	if (*readpos == queue->maxitems - 1) {
		*readpos = 0;
	}
	else {
		queue->readpos++;
	}*/
}

void* tmain(wmt_queue* queue) {
	DWORD claimed;
	tmain_loop:
		claimed = WaitForSingleObject(queue->mutex, INFINITE);
		if (claimed == WAIT_OBJECT_0) {
			wmt_task* thistask = (wmt_task*)&queue->items[queue->readpos];
			void* (*task)(void*) = NULL;
			wmt_task_arg arg = NULL;
			_asm {
				mov ebx, handle_task
				mov ecx, while_no_func
				mov esi, thistask
				mov eax, dword ptr [esi]
				cmp eax, 0
				cmove ebx, ecx
				jmp ebx
			while_no_func:
				mov eax, dword ptr [esi]
				cmp eax, 0
				je while_no_func
			}
			/*if (thistask->func != NULL) { //if queue slot is available
				goto handle_task;
			}
			else {
				while (thistask->func == NULL) { } //wait for current task in queue slot to be handled
				goto handle_task;
			}*/
		handle_task:
			task = thistask->func;
			arg = thistask->arg;
			thistask->func = NULL;
			thistask->arg = NULL;
			inc_readpos(queue);
			ReleaseMutex(queue->mutex);
			task(arg);
	goto tmain_loop;
	}
	return (void*)0;
}

wmt_stack* initStack(int size) {
	wmt_stack* stk = malloc(sizeof(wmt_stack));
	if (!stk) { return(wmt_stack*)-1; }
	stk->items = malloc(sizeof(wmt_item) * size);
	if (!stk->items) { return(wmt_stack*)-1; }
	stk->sp = 0;
	stk->mutex = CreateMutex(NULL, FALSE, NULL);
	return stk;
}

wmt_item wmt_stack_pop(wmt_stack* stk) {
	wmt_item itemptr = 0;
	DWORD waitResult = WaitForSingleObject(stk->mutex, INFINITE);
	if (waitResult == WAIT_OBJECT_0) {
		if (stk->sp == 0) { return 0; }
		itemptr = stk->items[stk->sp];
		stk->items[stk->sp] = 0;
		stk->sp--;
		ReleaseMutex(stk->mutex);
	}

	return itemptr;
}

int wmt_stack_push(wmt_stack* stk, wmt_stack_item_ptr itemptr) {
	DWORD waitResult = WaitForSingleObject(stk->mutex, INFINITE);
	if (waitResult == WAIT_OBJECT_0) {
		if (stk->sp == stk->maxitems - 1) { return 1; }
		stk->sp++;
		stk->items[stk->sp] = itemptr;
		ReleaseMutex(stk->mutex);
	}
	return 0;
}