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
#include <wait.h>

#define totalsize 7
#define size 3

int *lttrs;
int *nums;



void wait_sem(int semid,int i) {
	struct sembuf s ={i,-1,SEM_UNDO};
	semop(semid,&s,1);
}

void signal_sem(int semid,int i) {
	struct sembuf s ={i,1,SEM_UNDO};
	semop(semid,&s,1);
}

/*Utility function to print the arrays. Array is of size totalsize defined above.*/

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

/*Checks if the array in question is sorted. Offset has a value of either totalsize or 0, to determine how far into the semaphore set you need to go, based on whether the numbers or letters array is being considered. The first totalsize semaphores in the set are for the numbers array, the second for the letters array.*/
int sorted(int *arr, int sem_id, int offset) {
	for (int i=0;i<totalsize-1;i++) {
		wait_sem(sem_id,i+offset);
		wait_sem(sem_id,i+offset+1);
		if (*(arr+i)>*(arr+i+1)) {
			signal_sem(sem_id,i+offset);
			signal_sem(sem_id,i+offset+1);
			return 0;
		}
		signal_sem(sem_id,i+offset);
		signal_sem(sem_id,i+offset+1);
	}
	return 1;
}

/*Takes a pointer to each of the two arrays, an offset into the arrays and the id of the semaphore set as arguments.
Checks the numbers array first, starting from the offset and going size elements. Then does the same for the letters array.*/
void sort(int* l, int* n, int offset,int sem_id) {
	for (int i=0;i<size-1;i++) {
		wait_sem(sem_id,i+offset); //Lock both elements before doing the comparison so they
		wait_sem(sem_id,i+offset+1); //aren't modified by anything else mid-compare.
		int x = *(n+i+offset);
		int y = *(n+i+1+offset);
		if ((isalpha(y)&&!isalpha(x)) || ((x>y)&&(isalpha(y))^!isalpha(x))) {//if the left is a number and the right is a letter, swap them OR if the left and right are both letters or both numbers and the left is larger than the right
			*(n+i+offset) = y;
			*(n+i+1+offset) = x;
		}
		signal_sem(sem_id,i+offset); //Unlock since we're done comparing/switching
		signal_sem(sem_id,i+offset+1);
	}
	for (int i=0;i<size-1;i++) {
		wait_sem(sem_id,i+offset+totalsize);
		wait_sem(sem_id,i+offset+1+totalsize);
		int x = *(l+i+offset);
		int y = *(l+i+1+offset);
		if (x>y) { //simpler logic here, numbers are smaller than letters so they filter left automatically
			*(l+i+offset) = y;
			*(l+i+1+offset) = x;
		}
		signal_sem(sem_id,i+offset+totalsize);
		signal_sem(sem_id,i+offset+1+totalsize);
	}
}

/*Exchanges numbers in the letters array with letters in the numbers array provided both occur at the same index.*/
void exchange(int* lttrs, int* nums, int offset,int sem_id) {
	for (int i=0;i<size;i++) {
		wait_sem(sem_id,i+offset);
		wait_sem(sem_id,i+offset+totalsize);
		int l = *(lttrs+i+offset);
		int n = *(nums+i+offset);
		if (!isalpha(l) && isalpha(n)) {
			*(lttrs+i+offset) = n;
			*(nums+i+offset) = l;
		}
		signal_sem(sem_id,i+offset);
		signal_sem(sem_id,i+offset+totalsize);
	}
}
/*Sanitize checks the input from the user to make sure that every character is either a letter or a number and that there are the same number of number and letters.
It also changes numbers from their ascii representation into the integer representation.*/
int sanitize(char* input) {
	int n = 0;
	int l = 0;
	for(int i=0;i<totalsize*2;i++) {
		int next = *(input+i);		
		if (isalpha(next)) {
			l++;
		}
		else {if (isdigit(next)) {
			*(input+i) = next-'0';
			n++;
		}
		else {
			if (next==0) {
				printf("Please enter the full 7 characters for each array. ");
			}
			else {
				printf("A character that is neither a number nor a letter was found. ");
			}
			return 0;
		}}
	}
	if (n!=l) {
		printf("The number of letters and numbers was not equal. ");
	}
	return (n==l);
}

/*Initializes the number and letter arrays based on the input given by the user. If the input the arrays are initialized to the values provided in the assignment.*/
void init(int* lttrs, int* nums, char* input) {
	if (*input&&sanitize(input)) {
		for (int i=0;i<totalsize;i++) {
			*(lttrs+i) = *(input+i);
			*(nums+i) = *(input+i+totalsize);
		}
	}
	else {
		printf("Input was invalid, using the default arrays.\n");
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
	
	printf("Please enter the starting values for the letters array\nPlease note that only the first 7 characters will be considered.\n");

	scanf("%s",a);
	printf("Please enter the starting values for the numbers array\nPlease note that only the first 7 characters will be considered.\n");
	scanf("%s",a+totalsize);

	init(lttrs,nums,str);
	
	int j; //J is used to determine which of the three processes this is, and thus the parts of the array it is responsible for
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
	while (!(sorted(lttrs,sem_id,totalsize) && sorted(nums,sem_id,0))) { //continue until both arrays are completely sorted
		sort(lttrs,nums,j*(size-1),sem_id);
		exchange(lttrs,nums,j*(size-1),sem_id);
	}
	if (pid!=0) {
		wait(NULL); //Wait for both children to terminate.
		wait(NULL);
		printf("\nAfter sorting:\nNumbers: \n");
		pr(nums);
		printf("Letters: \n");
		pr(lttrs);
		shmdt(lttrs); //clean up the shared memory
		shmdt(nums);
	}
	exit(0);
}
