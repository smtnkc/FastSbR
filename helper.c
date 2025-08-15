#include "helper.h"

void swap (int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

int* createRandomArray(int A_LEN, int SEED)
{
	int i, *A_middle, *A;
	int A_middle_LEN = A_LEN-2; // Length of middle array (excluding first and last elements)
	A_middle = calloc(A_middle_LEN, sizeof(int));
	A = calloc(A_LEN, sizeof(int));
	srand(SEED);

	for(i=0; i<A_middle_LEN; i++) // Fill middle array with sequential numbers from 1 to A_middle_LEN
		A_middle[i] = i+1;

	for(i = A_middle_LEN-1; i > 0; i--) // Fisher-Yates shuffle algorithm to randomize middle array
	{
		int j = rand() % (i+1);
		swap(&A_middle[i], &A_middle[j]);
	}

	A[0] = 0;
	A[A_LEN-1] = A_LEN-1;
	memcpy(&A[1], A_middle, A_middle_LEN * sizeof(int));

	return A;
}

int* createSemiRandomArray(int A_LEN, int SEED)
{
	int i, *A_middle, *A;
	int A_middle_LEN = A_LEN-2; // Length of middle array (excluding first and last elements)
	A_middle = calloc(A_middle_LEN, sizeof(int));
	A = calloc(A_LEN, sizeof(int));
	srand(SEED);

	for(i=0; i<A_middle_LEN; i++) // Fill middle array with sequential numbers from 1 to A_middle_LEN
		A_middle[i] = i+1;

	int nRevelsals = pow(log(A_middle_LEN),2); // Number of reversals = log(A_middle_LEN)^2

	for(i = 0; i < nRevelsals; i++) // Perform random reversals
	{
		int rev_begin = rand() % A_middle_LEN;
		int rev_end = rand() % A_middle_LEN;
		if(rev_begin > rev_end)
			swap(&rev_begin, &rev_end);
		reverse(A_middle, rev_begin, rev_end);
	}

	A[0] = 0;
	A[A_LEN-1] = A_LEN-1;
	memcpy(&A[1], A_middle, A_middle_LEN * sizeof(int));

	return A;
}

void printArray(int A[], int s, int e)
{
	int i;
	for(i=s; i<e; i++)
		printf("%d, ", A[i]);

	printf("%d\n", A[i]);
}

int* readArrayFromFile(int A_LEN) {
	char filename[20];
	int i;
	sprintf(filename, "elements/N%d.txt", A_LEN);
	FILE *f = fopen(filename, "r");
	int* A = calloc(A_LEN, sizeof(int));
	for(i=0;i<A_LEN;i++)
		fscanf(f, "%d ", &A[i]);
	fclose(f);
	return A;
}

int* getPortBoundaries(int A[], int A_LEN, int myProcId, int nProcs, MPI_Status status, int MSG_TAG)
{
	int initPortSize = A_LEN/nProcs;
	int myPortBegin = myProcId * initPortSize;
	int myPortEnd, prevPortEnd;

	if(myProcId == nProcs)
		myPortEnd = A_LEN - 1;
	else
		myPortEnd = ((myProcId + 1) * initPortSize) - 1;

	// EXTEND PORTIONS TO PREVENT STRIP-BREAKS
	while(myPortEnd < A_LEN-1 && abs(A[myPortEnd] - A[myPortEnd+1]) == 1)
		myPortEnd++;

	// Use non-blocking communication to avoid potential deadlocks
	MPI_Request send_request, recv_request;
	int destProcId = (myProcId + 1) % nProcs;
	int srcProcId = (nProcs + (myProcId-1)) % nProcs;
	
	// Start non-blocking send and receive operations
	MPI_Isend(&myPortEnd, 1, MPI_INT, destProcId, MSG_TAG, MPI_COMM_WORLD, &send_request);
	MPI_Irecv(&prevPortEnd, 1, MPI_INT, srcProcId, MSG_TAG, MPI_COMM_WORLD, &recv_request);
	
	// Wait for both operations to complete
	MPI_Wait(&recv_request, &status);
	MPI_Wait(&send_request, MPI_STATUS_IGNORE);

	if(myProcId == 0)
		myPortBegin = 0;
	else
		myPortBegin = prevPortEnd + 1;

	if(myPortBegin > myPortEnd || myPortBegin >= A_LEN)
		myPortBegin = myPortEnd = -1; // THE LAST PORTION IS EMPTY

	int* myPortBoundaries = (int*)calloc(2, sizeof(int));
	myPortBoundaries[0] = myPortBegin;
	myPortBoundaries[1] = myPortEnd;

	return myPortBoundaries;
}

int* getGlobalMinMax(int ARRAY[], int nWorkers, int ARRAY_LEN, MPI_Status status, int MSG_TAG)
{
	int i;

	// SEND ARRAY WITH COMPUTE TAG
	for(i = 1; i <= nWorkers; i++)
		MPI_Send(&ARRAY[0], ARRAY_LEN, MPI_INT, i, COMPUTE, MPI_COMM_WORLD);

	int *workerPortBoundaries = (int *)calloc(nWorkers*2, sizeof(int));

	// RECEIVE PORT ENDS
	for(i = 1; i <= nWorkers; i++)
		MPI_Recv(&workerPortBoundaries[(i*2)-1], 1, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD, &status);

	// DEFINE PORT BEGINS
	workerPortBoundaries[0] = 0;
	for(i = 1; i < nWorkers; i++)
		workerPortBoundaries[i*2] = workerPortBoundaries[(i*2)-1] + 1; // W_BEGIN = PREV_W_END + 1

	// SEND UPDATED PORT BOUNDARIES
	for(i = 1; i <= nWorkers; i++)
		MPI_Send(&workerPortBoundaries[(i-1)*2], 2, MPI_INT, i, MSG_TAG + 1, MPI_COMM_WORLD);

	int *lDecMinMax = (int *)calloc(nWorkers*2, sizeof(int));

	// RECEIVE LOCAL DEC_MIN and DEC_MAX VALUES
	for(i = 1; i <= nWorkers; i++)
		MPI_Recv(&lDecMinMax[(i-1)*2], 2, MPI_INT, i, MSG_TAG + 2, MPI_COMM_WORLD, &status);

	int gDecMin = lDecMinMax[0];
	int gDecMax = lDecMinMax[1];

	// FIND GLOBAL DEC_MIN and DEC_MAX
	for(i = 2; i < nWorkers *2 ; i += 2)
	{
		if(lDecMinMax[i] < gDecMin)
			gDecMin = lDecMinMax[i];
		if(lDecMinMax[i+1] > gDecMax)
			gDecMax = lDecMinMax[i+1];
	}

	int* gDecMinMax = (int*)calloc(2, sizeof(int));
	gDecMinMax[0] = gDecMin;
	gDecMinMax[1] = gDecMax;

	return gDecMinMax;
}

int* getLocalMinMax(int LOCAL[], int nWorkers, int myProcId, int ARRAY_LEN, MPI_Status status, int MSG_TAG)
{
	int initPortSize = ARRAY_LEN / nWorkers;
	int myPortEnd, myPortBoundaries[2];
	int myPortBegin = (myProcId-1) * initPortSize;
	int myDecMin, myDecMax;

	if(myProcId == nWorkers)
		myPortEnd = ARRAY_LEN - 1; // THE LAST WORKER
	else
		myPortEnd = (myProcId * initPortSize) - 1;

	// EXTEND PORTIONS TO PREVENT STRIP-BREAKS
	while(myPortEnd != ARRAY_LEN - 1 && abs(LOCAL[myPortEnd] - LOCAL[myPortEnd + 1]) == 1)
		myPortEnd++;

	// SEND MY PORT END
	MPI_Send(&myPortEnd, 1, MPI_INT, ROOT, MSG_TAG, MPI_COMM_WORLD);

	// RECEIVE MY UPDATED PORT BOUNDARIES
	MPI_Recv(&myPortBoundaries, 2, MPI_INT, ROOT, MSG_TAG + 1, MPI_COMM_WORLD, &status);
	myPortBegin = myPortBoundaries[0];
	myPortEnd = myPortBoundaries[1];

	if(myPortBegin != -1 && myPortEnd != -1) {
		node** strips = generateStrips(LOCAL, myPortBegin, myPortEnd, ARRAY_LEN);
		myDecMin = findMin(strips[0]);
		myDecMax = findMax(strips[0]);
	} else {
		myDecMin = MAX_VAL;
		myDecMax = MIN_VAL;
	}

	int* myDecMinMax = (int*)calloc(2, sizeof(int));
	myDecMinMax[0] = myDecMin;
	myDecMinMax[1] = myDecMax;

	return myDecMinMax;
}

void insert(node** head, int strip_no, int val)
{
	node* newNode;
	newNode = (node*)calloc(1, sizeof(node));
	if(newNode == NULL)
		printf("Failed to Allocate Memory");

	newNode->data = val;
	newNode->strip_no = strip_no;
	newNode->next = NULL;

	if((*head)==NULL)
		(*head)=newNode;
	else
	{
		node* temp = (*head);
		while(temp->next != NULL)
		{
			temp = temp->next;
		}
	 	temp->next = newNode;
	}
}

void freeList(node** head)
{
	node* temp;
	while ((*head) != NULL)
	{
		temp = (*head);
		(*head) = (*head)->next;
		free(temp);
	}
}

void printStrips(node** decs, node** incs)
{
	node *curr1, *curr2;
	curr1 = (*decs);
	curr2 = (*incs);
	printf("DECS: { ");
	while(curr1)
	{
		printf("%d ", curr1->data);

		if(curr1->next != NULL && curr1->strip_no != curr1->next->strip_no)
			printf("}{ ");

		curr1 = curr1->next ;
	}
	printf("}\nINCS: { ");
	while(curr2)
	{
		printf("%d ", curr2->data);

		if(curr2->next != NULL && curr2->strip_no != curr2->next->strip_no)
			printf("}{ ");

		curr2 = curr2->next ;
	}
	printf("}\n");
}

void printMyStrips(node** decs, node** incs, int myProcId, int itr)
{
	node *curr1, *curr2;
	curr1 = (*decs);
	curr2 = (*incs);
	printf("DECS_P%d_I%d: { ", myProcId, itr);
	while(curr1)
	{
		printf("%d ", curr1->data);

		if(curr1->next != NULL && curr1->strip_no != curr1->next->strip_no)
			printf("}{ ");

		curr1 = curr1->next ;
	}
	printf("}\nINCS_P%d_I%d: { ", myProcId, itr);
	while(curr2)
	{
		printf("%d ", curr2->data);

		if(curr2->next != NULL && curr2->strip_no != curr2->next->strip_no)
			printf("}{ ");

		curr2 = curr2->next ;
	}
	printf("}\n");
}

node** generateStrips(int A[], int port_begin, int port_end, int A_LEN)
{
	node* decs = NULL;
	node* incs = NULL;

	if(port_begin == -1 || port_end == -1)
	{
		// IF PORTION IS EMPTY
		return NULL;
	}

	int i, dec_no = 0, inc_no = 0, is_first_dec = 1;
	int gene_begin, gene_end, gene_curr, gene_prev, gene_next;

	for(i=port_begin; i<=port_end; i++)
	{
		gene_begin = A[port_begin];
		gene_end = A[port_end];
		gene_curr = A[i];

		if(i > 0)
			gene_prev = A[i-1];
		if(i < A_LEN - 1)
			gene_next = A[i+1];

		if(gene_curr == 0)
		{
			// add the first element to increasing strips
			insert(&incs, inc_no, gene_curr);
		}
		else if(gene_curr == A_LEN - 1)
		{
			// add the last element to increasing strips
			if(gene_curr - gene_prev != 1)
			{
				// if the last element is alone
				inc_no++;
			}
			insert(&incs, inc_no, gene_curr);
		}
		else
		{
			// this part is for the middle elements
			if(gene_curr - gene_prev == 1)
				insert(&incs, inc_no, gene_curr);
			else if(gene_curr - gene_prev == -1)
				insert(&decs, dec_no, gene_curr);
			else
			{
				// add this element to a new strip
				if(gene_curr - gene_next == -1)
				{
					// it will be a new increasing strip
					inc_no++;
					insert(&incs, inc_no, gene_curr);
				}
				else
				{
					// it will be a new decreasing strip
					if(!is_first_dec)
						dec_no++;
					insert(&decs, dec_no, gene_curr);
					is_first_dec = 0;
				}
			}
		}
	}

	node** strips = (node**)calloc(1, sizeof(decs)+sizeof(incs));
	strips[0] = decs;
	strips[1] = incs;
	return strips;
}

int findMin(node* head)
{
	if (head == NULL)
		return MAX_VAL;
		
	int min = head->data;
	node* curr = head->next;
	
	while (curr)
	{
		if (curr->data < min)
			min = curr->data;
		curr = curr->next;
	}
	return min;
}

int findMax(node* head)
{
	if (head == NULL)
		return MIN_VAL;
		
	int max = head->data;
	node* curr = head->next;
	
	while (curr)
	{
		if (curr->data > max)
			max = curr->data;
		curr = curr->next;
	}
	return max;
}

int findIndexOf(int gene, int A[], int A_LEN)
{
	// Early return for invalid inputs
	if (A == NULL || A_LEN <= 0)
		return -1;
	
	// Linear search with early termination
	int i;
	for (i = 0; i < A_LEN; i++) {
		if (gene == A[i])
			return i;
	}
	
	return -1;
}

void reverse(int A[], int strip_begin, int strip_end)
{
	int i, j = strip_end;
	for(i=0; i<(strip_end-strip_begin+1)/2; i++)
	{
		swap(&A[i+strip_begin], &A[j]);
		j--;
	}
}

void reverseAnInc(int A[], int A_LEN)
{
	int i, gene_begin, gene_end;
	for(i=0; i<A_LEN; i++)
	{
		if(A[i]+1 == A[i+1])
			continue;
		else
		{
			gene_begin = A[i+1];
			break;
		}
	}
	int strip_begin = findIndexOf(gene_begin, A, A_LEN);
	for(i=strip_begin; i<A_LEN; i++)
	{
		if(A[i]+1 == A[i+1])
			continue;
		else
		{
			gene_end = A[i];
			break;
		}
	}
	int strip_end = findIndexOf(gene_end, A, A_LEN);
	reverse(A, strip_begin, strip_end);
}

int hasBreakpoints(int A[], int A_LEN)
{
	int i;
	for(i=1;i<A_LEN;i++)
	{
		if (A[i] != A[i-1] + 1)
			return 1;
	}
	return 0;
}
