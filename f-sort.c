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

int sorted(int *arr, int l) {
	int x = *arr;
	int y = *(arr+1);
	int z = *(arr+2);
	return (x<y&&y<z)^!l;
}

void sort(int* l, int* n) {
	for (int i=0;i<2;i++) {
		int x = *(l+i);
		int y = *(l+i+1);
		if (x>y) {
			*(l+i) = y;
			*(l+i+1) = x;
		}
	}
	for (int i=0;i<2;i++) {
		int x = *(n+i);
		int y = *(n+i+1);
		if (x<y) {
			*(n+i) = y;
			*(n+i+1) = x;
		}
	}
}

int main(void) {
	lttrs = malloc(3*sizeof(int));
	assert(lttrs);
	*lttrs = 'E';
	*(lttrs+1) = 4;
	*(lttrs+2) = 7;
	nums = malloc(3*sizeof(int));
	assert(nums);
	*nums = 5;
	*(nums+1) = 'P';
	*(nums+2) = 'M';
	while (!(sorted(lttrs,1) && sorted(nums,0))) {
		pr(lttrs);
		pr(nums);
		sort(lttrs,nums);
	}
	pr(lttrs);
	pr(nums);
	//ltrs  = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	//nms  = shmget(IPC_PRIVATE, 6*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	//shmdt(ltrs);
	//shmdt(nums);
	free(lttrs);
	free(nums);
}
