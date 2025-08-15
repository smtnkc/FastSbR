#ifndef HELPER_H_
#define HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <sys/time.h>
#include <math.h>

typedef struct Node
{
	int data;
	int strip_no;
	struct Node *next;
} node;

#define MAX_VAL INT_MAX
#define MIN_VAL INT_MIN
#define ROOT 0
#define COMPUTE 100
#define STOP 200
#define RUN_RHO_MAX 300
#define PASS_RHO_MAX 400
#define PRINT_SORTING_STEPS 0

void swap (int *a, int *b);
void freeStrips(node** strips);
int* createRandomArray(int A_LEN, int SEED);
int* createSemiRandomArray(int A_LEN, int SEED);
void printArray(int A[], int s, int e);
int* readArrayFromFile(int A_LEN);
int* getPortBoundaries(int A[], int A_LEN, int myProcId, int nProcs, MPI_Status status, int MSG_TAG);
int* getGlobalMinMax(int ARRAY[], int nWorkers, int ARRAY_LEN, MPI_Status status, int MSG_TAG);
int* getLocalMinMax(int LOCAL[], int nWorkers, int myProcId, int ARRAY_LEN, MPI_Status status, int MSG_TAG);
void insert(node** head, int strip_no, int val);
void freeList(node** head);
void printStrips(node** decs, node** incs);
void printMyStrips(node** decs, node** incs, int myProcId, int itr);
node** generateStrips(int A[], int port_begin, int port_end, int A_LEN);
int findMin(node* head);
int findMax(node* head);
int findIndexOf(int gene, int A[], int A_LEN);
void reverse(int A[], int strip_begin, int strip_end);
void reverseAnInc(int A[], int A_LEN);
int hasBreakpoints(int A[], int A_LEN);

#endif
