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

#define totalsize 7
#define size 3

int *lttrs;
int *nums;


void wait(int semid,int i) {
	struct sembuf s ={i,-1,SEM_UNDO};
	semop(semid,&s,1);
}

void unwait(int semid,int i) {
	struct sembuf s ={i,1,SEM_UNDO};
	semop(semid,&s,1);
}

void pr(int* arr) {
	for (int i=0;i<totalsize;i++) {
		if (isalpha(*(arr+i))){
			printf("%c ", *(arr+i));
		}
		else {
			printf("%d ", *(arr+i));
		}
	}
	printf("\n-----------------------------\n");;
}

int sorted(int *arr) {
	for (int i=0;i<totalsize-1;i++) {
		if (*(arr+i)>*(arr+i+1)) {
			return 0;
		}
	}
	return 1;
}

void sort(int* l, int* n, int offset,int a) {
	for (int i=0;i<size-1;i++) {
		wait(a,i+offset);
		wait(a,i+offset+1);
		int x = *(n+i+offset);
		int y = *(n+i+1+offset);
		if ((isalpha(y)&&!isalpha(x)) || ((x>y)&&(isalpha(y))^!isalpha(x))) {
			*(n+i+offset) = y;
			*(n+i+1+offset) = x;
		}
		unwait(a,i+offset);
		unwait(a,i+offset+1);
	}
	for (int i=0;i<size-1;i++) {
		wait(a,i+offset+totalsize);
		wait(a,i+offset+1+totalsize);
		int x = *(l+i+offset);
		int y = *(l+i+1+offset);
		if (x>y) {
			*(l+i+offset) = y;
			*(l+i+1+offset) = x;
		}
		unwait(a,i+offset+totalsize);
		unwait(a,i+offset+1+totalsize);
	}
}

void exchange(int* lttrs, int* nums, int offset,int a) {
	for (int i=0;i<size;i++) {
		wait(a,i+offset);
		wait(a,i+offset+totalsize);
		int l = *(lttrs+i+offset);
		int n = *(nums+i+offset);
		if (!isalpha(l) && isalpha(n)) {
			*(lttrs+i+offset) = n;
			*(nums+i+offset) = l;
		}
		unwait(a,i+offset);
		unwait(a,i+offset+totalsize);
	}
}

void init(int* lttrs, int* nums) {
	srand(rand());
	for (int i=0;i<totalsize;i++) {
		int j = rand()%2;
		if (j) {
			*(lttrs+i) = rand()%10;
			*(nums+i) = rand()%26+65;
		}
		else {
			*(lttrs+i) =rand()%26+65;
			*(nums+i) = rand()%10;
		}
	}
	/*lttrs = 'E';
	*(lttrs+1) = 4;
	*(lttrs+2) = 7;
	*(lttrs+3) = 'Y';
	*(lttrs+4) = 3;
	*(lttrs+5) = 7;
	*(lttrs+6) = 'A';

	*nums = 5;
	*(nums+1) = 'P';
	*(nums+2) = 'M';
	*(nums+3) = 'M';
	*(nums+4) = 8;
	*(nums+5) = 'G';
	*(nums+6) = 1;*/
}

int main(void) {
	int l_id = shmget(IPC_PRIVATE, totalsize*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	lttrs = shmat(l_id,NULL,0);
	assert(lttrs);
	
	int n_id = shmget(IPC_PRIVATE, totalsize*sizeof(int), IPC_CREAT | 0600 | IPC_EXCL);
	nums = shmat(n_id,NULL,0);
	assert(nums);

	int sem_id = semget(IPC_PRIVATE,2*totalsize,IPC_CREAT | 0660 |IPC_EXCL);
	for (int i=0;i<2*totalsize;i++) {
		semctl(sem_id,i,SETVAL,1);
	}


	init(lttrs,nums);

	printf("numbers: \n");
	pr(nums);
	printf("letters: \n");
	pr(lttrs);
	
	int first_child = 1;
	pid_t pid;
	pid = fork();
	while (!(sorted(lttrs) && sorted(nums))) {
	switch (pid) {
		case -1:
			break;
		case 0:
			if (first_child) {
				sort(lttrs,nums,0,sem_id);
				exchange(lttrs,nums,0,sem_id);
			}
			else {
				sort(lttrs,nums,2*(size-1),sem_id);
				exchange(lttrs,nums,2*(size-1),sem_id);
			}
			break;
		default:
			if (first_child) {
				pid = fork();
				first_child = 0;
			}
			else {
				printf("numbers: \n");
				pr(nums);
				printf("letters: \n");
				pr(lttrs);
				sort(lttrs,nums,size-1,sem_id);
				exchange(lttrs,nums,size-1,sem_id);
			}
			break;
	}}
	if (pid!=0) {
		printf("numbers: \n");
		pr(nums);
		printf("letters: \n");
		pr(lttrs);
	}
	shmdt(lttrs);
	shmdt(nums);
}
