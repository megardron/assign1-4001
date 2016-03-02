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

void signal(int semid,int i) {
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

void sort(int* l, int* n, int offset,int sem_id) {
	for (int i=0;i<size-1;i++) {
		wait(sem_id,i+offset);
		wait(sem_id,i+offset+1);
		int x = *(n+i+offset);
		int y = *(n+i+1+offset);
		if ((isalpha(y)&&!isalpha(x)) || ((x>y)&&(isalpha(y))^!isalpha(x))) {
			*(n+i+offset) = y;
			*(n+i+1+offset) = x;
		}
		signal(sem_id,i+offset);
		signal(sem_id,i+offset+1);
	}
	for (int i=0;i<size-1;i++) {
		wait(sem_id,i+offset+totalsize);
		wait(sem_id,i+offset+1+totalsize);
		int x = *(l+i+offset);
		int y = *(l+i+1+offset);
		if (x>y) {
			*(l+i+offset) = y;
			*(l+i+1+offset) = x;
		}
		signal(sem_id,i+offset+totalsize);
		signal(sem_id,i+offset+1+totalsize);
	}
}

void exchange(int* lttrs, int* nums, int offset,int sem_id) {
	for (int i=0;i<size;i++) {
		wait(sem_id,i+offset);
		wait(sem_id,i+offset+totalsize);
		int l = *(lttrs+i+offset);
		int n = *(nums+i+offset);
		if (!isalpha(l) && isalpha(n)) {
			*(lttrs+i+offset) = n;
			*(nums+i+offset) = l;
		}
		signal(sem_id,i+offset);
		signal(sem_id,i+offset+totalsize);
	}
}

int sanitize(char* input) {
	int n = 0;
	int l = 0;
	for(int i=0;i<totalsize*2;i++) {
		int next = *(input+i);
		printf("\n%d %d\n", i, next);
		
		if (isalpha(next)) {
			printf("alpha\n");
			l++;
		}
		else {if (isdigit(next)) {
			printf("num\n");
			*(input+i) = next-'0';
			n++;
		}
		else {
			printf("false\n");
			return 0;
		}}
	}
	return (n==l);
}

void init(int* lttrs, int* nums, char* input) {
	printf("inside the init   %d\n",*input);
	if (*input&&sanitize(input)) {
		for (int i=0;i<totalsize;i++) {
			printf("%d %d\n", i, *(input+i));
				*(lttrs+i) = *(input+i);
				*(nums+i) = *(input+i+totalsize);
		}
	}
	else {
	*nums = 5;
	*(nums+1) = 8;
	*(nums+2) = 4;
	*(nums+3) = 'A';
	*(nums+4) = 6;
	*(nums+5) = 'C';
	*(nums+6) = 0;
	
	*lttrs = 'K';
	*(lttrs+1) = 'J';
	*(lttrs+2) = 3;
	*(lttrs+3) = 'C';
	*(lttrs+4) = 9;
	*(lttrs+5) = 'F';
	*(lttrs+6) = 'B';
	}


	
	/*srand(rand());
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
	}*/
	printf("\nBefore sorting:\nNumbers:\n");
	pr(nums);
	printf("Letters:\n");
	pr(lttrs);
	
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
	
	char str[1000];
	char* a = str; 
	
	printf("Please enter the values to put in the array.\n");

	scanf("%s",a);
	printf("%s",a);

	init(lttrs,nums,str);
	
	int j;
	pid_t pid;
	pid = fork();
	switch (pid) {
		case -1:
			break;
		case 0:
			j = 0;
			break;
		default:
			pid = fork();
			switch(pid) {
				case -1:
					break;
				case 0:
					j=1;
					break;
				default:
					j=2;
					break;
			}
			break;
	}
	while (!(sorted(lttrs) && sorted(nums))) {
		sort(lttrs,nums,j*(size-1),sem_id);
		exchange(lttrs,nums,j*(size-1),sem_id);
	}
	if (pid!=0) {
		printf("\nAfter sorting:\nNumbers: \n");
		pr(nums);
		printf("Letters: \n");
		pr(lttrs);
	}
	shmdt(lttrs);
	shmdt(nums);
}
