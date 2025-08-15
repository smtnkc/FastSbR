#include "helper.h"

int main(int argc, char** argv)
{
	int* ARRAY;
	int ARRAY_LEN = atoi(argv[1]);
	char* OUTPUT_FILE = argv[2];
	int i = 0, itr = 0;
	int nProcs, nPorts, initPortSize;
	int myProcId, myPortEnd, myPortBegin;
	int myDecMin, gDecMin;
	int srcProcId, destProcId;
	double time_start, time_end;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &myProcId);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */

	ARRAY = readArrayFromFile(ARRAY_LEN);

	#if PRINT_SORTING_STEPS
		if(myProcId == 0) printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS AT THE BEGINNING
	#endif

/******************************************************************************************/

	time_start = MPI_Wtime();

	node** strips = NULL;
	while(hasBreakpoints(ARRAY, ARRAY_LEN))
	{
		itr++;
		int* myPortBoundaries = getPortBoundaries(ARRAY, ARRAY_LEN, myProcId, nProcs, status, 1000);
		int myPortBegin = myPortBoundaries[0];
		int myPortEnd = myPortBoundaries[1];

		if (myPortBegin != -1) {
			strips = generateStrips(ARRAY, myPortBegin, myPortEnd, ARRAY_LEN);
			myDecMin = findMin(strips[0]);
		}
		else {
			myDecMin = MAX_VAL;
		}

		gDecMin = MAX_VAL;

		MPI_Allreduce(&myDecMin, &gDecMin, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

		if(gDecMin != MAX_VAL)
		{
			// DECREASING STRIP EXISTS
			int rhoMinStart = findIndexOf(gDecMin-1, ARRAY, ARRAY_LEN);
			int rhoMinEnd = findIndexOf(gDecMin, ARRAY, ARRAY_LEN);
			if(rhoMinStart > rhoMinEnd)
			{
				swap(&rhoMinStart, &rhoMinEnd);
			}
			reverse(ARRAY, rhoMinStart+1, rhoMinEnd);
		}
		else if (hasBreakpoints(ARRAY, ARRAY_LEN))
		{
			// ALL STRIPS ARE INCREASING
			reverseAnInc(ARRAY,ARRAY_LEN);
		}

		#if PRINT_SORTING_STEPS
			if(myProcId == 0) printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS IN EACH STEP
		#endif
	}

	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
	time_end = MPI_Wtime();
	MPI_Finalize();

/******************************************************************************************/

	if(myProcId == 0)
	{
		printf("FD4,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);

		FILE *stdo = fopen(OUTPUT_FILE, "a");
		fprintf(stdo, "FD4,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);
		fclose(stdo);
	}
	return 0;
}
