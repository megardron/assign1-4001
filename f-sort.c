#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>

#define totalsize 6
#define size 3

int *lttrs;
int *nums;

void pr(int* arr) {
	for (int i=0;i<3;i++) {
		if (isalpha(*(arr+i))){
			printf("%c ", *(arr+i));
		}
		else {
			printf("%d ", *(arr+i));
		}
	}
	printf("\n-----------------------------\n");
}

int sorted(int *arr) {
	int x = *arr;
	int y = *(arr+1);
	int z = *(arr+2);
	return (x<=y&&y<=z);
}

void sort(int* l, int* n, int offset) {
	for (int i=0;i<size-1;i++) {
		int x = *(n+i+offset);
		int y = *(n+i+1+offset);
		if ((isalpha(y)&&!isalpha(x)) || ((x>y)&&(isalpha(y))^!isalpha(x))) {
			*(n+i+offset) = y;
			*(n+i+1+offset) = x;
		}
	}
	for (int i=0;i<size-1;i++) {
		int x = *(l+i+offset);
		int y = *(l+i+1+offset);
		if (x>y) {
			*(l+i+offset) = y;
			*(l+i+1+offset) = x;
		}
	}
}

void exchange(int* lttrs, int* nums, int offset) {
	for (int i=0;i<size;i++) {
		int l = *(lttrs+i+offset);
		int n = *(nums+i+offset);
		if (!isalpha(l) && isalpha(n)) {
			*(lttrs+i+offset) = n;
			*(nums+i+offset) = l;
		}
	}
}

int main(void) {
	
	int l_id = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	//lttrs = malloc(totalsize*sizeof(int));
	lttrs = shmat(l_id,NULL,0);
	assert(lttrs);
	*lttrs = 'E';
	*(lttrs+1) = 4;
	*(lttrs+2) = 7;
	//nums = malloc(totalsize*sizeof(int));
	int n_id = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	nums = shmat(n_id,NULL,0);
	assert(nums);
	*nums = 5;
	*(nums+1) = 'P';
	*(nums+2) = 'M';
	while (!(sorted(lttrs) && sorted(nums))) {
		printf("numbers: \n");
		pr(nums);
		printf("letters: \n");
		pr(lttrs);
		sort(lttrs,nums,0);
		exchange(lttrs,nums,0);
	}
	//sort(lttrs,nums);
	//exchange(lttrs,nums);
	printf("numbers: \n");
	pr(nums);
	printf("letters: \n");
	pr(lttrs);
	//ltrs  = 
	//nms  = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	shmdt(lttrs);
	shmdt(nums);
	//free(lttrs);
	//free(nums);
}
