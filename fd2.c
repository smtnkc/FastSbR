#include "helper.h"

int main(int argc, char** argv)
{
	int* ARRAY;
	int* TEMP;
	int ARRAY_LEN = atoi(argv[1]);
	char* OUTPUT_FILE = argv[2];
	int i = 0, itr = 0;
	int nProcs, nPorts, initPortSize;
	int myProcId, myPortEnd, myPortBegin;
	int myDecMin, myTempDecMin, gDecMin, gTempDecMin;
	int myDecMax, myTempDecMax, gDecMax, gTempDecMax;
	int srcProcIdProcId, destProcIdProcId;
	double time_start, time_end;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &myProcId);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */

	ARRAY = readArrayFromFile(ARRAY_LEN);
	TEMP = calloc(ARRAY_LEN, sizeof(int));

	#if PRINT_SORTING_STEPS
		if(myProcId == 0) printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS AT THE BEGINNING
	#endif

/******************************************************************************************/

	time_start = MPI_Wtime();

	while(hasBreakpoints(ARRAY, ARRAY_LEN))
	{
		itr++;
		memcpy(&TEMP[0], ARRAY, ARRAY_LEN * sizeof(int));

		int* myPortBoundaries = getPortBoundaries(ARRAY, ARRAY_LEN, myProcId, nProcs, status, 1001);
		int myPortBegin = myPortBoundaries[0]; // -1 IF PORTION IS EMPTY
		int myPortEnd = myPortBoundaries[1]; // -1 IF PORTION IS EMPTY

		node** strips = NULL;
		if(myPortBegin != -1 && myPortEnd != -1) {
			strips = generateStrips(ARRAY, myPortBegin, myPortEnd, ARRAY_LEN);
			myDecMin = findMin(strips[0]);
			myDecMax = findMax(strips[0]);
		} else {
			myDecMin = MAX_VAL;
			myDecMax = MIN_VAL;
		}

		MPI_Allreduce(&myDecMin, &gDecMin, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
		MPI_Allreduce(&myDecMax, &gDecMax, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

		int rhoMinStart = findIndexOf(gDecMin-1, ARRAY, ARRAY_LEN);
		int rhoMinEnd = findIndexOf(gDecMin, ARRAY, ARRAY_LEN);
		if(rhoMinStart > rhoMinEnd)
			swap(&rhoMinStart, &rhoMinEnd);

		int rhoMaxStart = findIndexOf(gDecMax, ARRAY, ARRAY_LEN);
		int rhoMaxEnd = findIndexOf(gDecMax+1, ARRAY, ARRAY_LEN);
		if(rhoMaxStart > rhoMaxEnd)
			swap(&rhoMaxStart, &rhoMaxEnd);

		reverse(TEMP, rhoMinStart + 1, rhoMinEnd); // REVERSE TEMP BY RHO_MIN

		int* myTempPortBoundaries = getPortBoundaries(TEMP, ARRAY_LEN, myProcId, nProcs, status, 1002);
		int myTempPortBegin = myTempPortBoundaries[0]; // -1 IF PORTION IS EMPTY
		int myTempPortEnd = myTempPortBoundaries[1]; // -1 IF PORTION IS EMPTY

		if(myTempPortBegin != -1 && myTempPortEnd != -1) {
			strips = generateStrips(TEMP, myTempPortBegin, myTempPortEnd, ARRAY_LEN);
			myTempDecMin = findMin(strips[0]);
			myTempDecMax = findMax(strips[0]);
		} else {
			myTempDecMin = MAX_VAL;
			myTempDecMax = MIN_VAL;
		}

		MPI_Allreduce(&myTempDecMin, &gTempDecMin, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
		MPI_Allreduce(&myTempDecMax, &gTempDecMax, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

		if(gTempDecMin != MAX_VAL || gTempDecMax != MIN_VAL)
		{
			// TEMP HAS A DECREASING STRIP
			memcpy(&ARRAY[0], TEMP, ARRAY_LEN * sizeof(int)); // KEEP THE REVERSAL RHO_MIN
		}
		else
		{
			reverse(ARRAY, rhoMaxStart, rhoMaxEnd-1); // REVERSE ARRAY BY RHO_MAX

			myPortBoundaries = getPortBoundaries(ARRAY, ARRAY_LEN, myProcId, nProcs, status, 1003);
			myPortBegin = myPortBoundaries[0]; // -1 IF PORTION IS EMPTY
			myPortEnd = myPortBoundaries[1]; // -1 IF PORTION IS EMPTY

			myDecMin = gDecMin = MAX_VAL; // RESET
			myDecMax = gDecMax = MIN_VAL; // RESET

			if(myPortBegin != -1 && myPortEnd != -1)
			{
				strips = generateStrips(ARRAY, myPortBegin, myPortEnd, ARRAY_LEN);
				myDecMin = findMin(strips[0]);
				myDecMax = findMax(strips[0]);
			}

			MPI_Allreduce(&myDecMin, &gDecMin, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
			MPI_Allreduce(&myDecMax, &gDecMax, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

			if(gDecMin == MAX_VAL && gDecMax == MIN_VAL && hasBreakpoints(ARRAY, ARRAY_LEN))
			{
				// ALL STRIPS ARE INCREASING
				reverseAnInc(ARRAY, ARRAY_LEN);
			}
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
		printf("FD2,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);

		FILE *stdo = fopen(OUTPUT_FILE, "a");
		fprintf(stdo, "FD2,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);
		fclose(stdo);
	}

	return 0;
}
