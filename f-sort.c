#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
//#include <sys/types.h>
//#include <sys/sem.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//#include <errno.h>
//#include <semaphore.h>

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
	printf("\n");
}

void sort(int* l, int* n) {
	for (int i=0;i<2;i++) {
		if (*(l+i)<*(l+i+1)) {
			int temp = *(l+i);
			*(l+i) = *(l+i+1);
			*(l+i+1) = temp;
		}
	}
}

int main(void) {
	lttrs = malloc(3*sizeof(int));
	assert(lttrs);
	*lttrs = 'E';
	*(lttrs+1) = 1;
	*(lttrs+2) = 7;
	pr(lttrs);
	sort(lttrs,nums);
	pr(lttrs);
	nums = malloc(3*sizeof(int));
	assert(nums);
	*nums = 5;
	*(nums+1) = 'P';
	*(nums+2) = 'M';
	pr(nums);
	//ltrs  = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	//nms  = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	//shmdt(ltrs);
	//shmdt(nums);
	free(lttrs);
	free(nums);
}
