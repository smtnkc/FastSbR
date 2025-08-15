#include "helper.h"

int main(int argc, char** argv)
{ 
	int i, myProcId, nProcs, initPortSize, nWorkers, nPorts, itr=0, ARRAY_LEN;
	int *ARRAY;
	char* OUTPUT_FILE;
	double time_start, time_end;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &myProcId);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */

	nWorkers = nPorts = (nProcs-1);

	if(myProcId == ROOT)
	{
		ARRAY_LEN = atoi(argv[1]);
		OUTPUT_FILE = argv[2];
		ARRAY = readArrayFromFile(ARRAY_LEN);

		#if PRINT_SORTING_STEPS
			printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS AT THE BEGINNING
		#endif

		time_start = MPI_Wtime();

		// SEND ARRAY_LEN
		for(i = 1; i <= nWorkers; i++)
			MPI_Send(&ARRAY_LEN, 1, MPI_INT, i, 5000, MPI_COMM_WORLD);

		while(hasBreakpoints(ARRAY, ARRAY_LEN))
		{
			itr++;

			// SEND ARRAY WITH COMPUTE TAG
			for(i = 1; i <= nWorkers; i++)
				MPI_Send(&ARRAY[0], ARRAY_LEN, MPI_INT, i, COMPUTE, MPI_COMM_WORLD);

			int *workerPortBoundaries = (int *)calloc(nPorts*2, sizeof(int));

			// RECEIVE PORT ENDS
			for(i = 1; i <= nWorkers; i++)
				MPI_Recv(&workerPortBoundaries[(i*2)-1], 1, MPI_INT, i, 1000, MPI_COMM_WORLD, &status);

			// DEFINE PORT BEGINS
			workerPortBoundaries[0] = 0; // BEGINNING OF WORKER 1
			for(i = 1; i < nWorkers; i++)
				workerPortBoundaries[i*2] = workerPortBoundaries[(i*2)-1] + 1; // W_BEGIN = PREV_W_END + 1

			// SEND UPDATED PORT BOUNDARIES
			for(i = 1; i <= nWorkers; i++)
				MPI_Send(&workerPortBoundaries[(i-1)*2], 2, MPI_INT, i, 1001, MPI_COMM_WORLD);

			/*******************************************************************/

			int *lDecMin = (int *)calloc(nWorkers, sizeof(int));

			// RECEIVE LOCAL DEC_MIN VALUES
			for(i = 1; i <= nWorkers; i++)
				MPI_Recv(&lDecMin[i-1], 1, MPI_INT, i, 1002, MPI_COMM_WORLD, &status);

			int gDecMin = lDecMin[0];

			// FIND GLOBAL DEC_MIN
			for(i=1;i<nWorkers;i++)
				if(lDecMin[i] < gDecMin)
					gDecMin = lDecMin[i];

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
				printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS IN EACH STEP
			#endif
		}

		// SEND ARRAY WITH STOP TAG
		for(i=1;i<=nWorkers;i++)
			MPI_Send(&ARRAY[0], ARRAY_LEN, MPI_INT, i, STOP, MPI_COMM_WORLD);
	}
	else
	{
		int myPortEnd, myPortBegin, myDecMin, myPortBoundaries[2];

		MPI_Recv(&ARRAY_LEN, 1, MPI_INT, ROOT, 5000, MPI_COMM_WORLD, &status); // RECEIVE ARRAY_LEN
		initPortSize = ARRAY_LEN / nPorts;

		while(1)
		{
			int LOCAL[ARRAY_LEN];
			MPI_Recv(&LOCAL[0], ARRAY_LEN, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if (status.MPI_TAG == COMPUTE)
			{
				myPortBegin = (myProcId-1) * initPortSize;

				if(myProcId == nWorkers)
					myPortEnd = ARRAY_LEN - 1; // THE LAST WORKER
				else
					myPortEnd = (myProcId * initPortSize) - 1;

				// EXTEND PORTIONS TO PREVENT STRIP-BREAKS
				while(myPortEnd != ARRAY_LEN - 1 && abs(LOCAL[myPortEnd] - LOCAL[myPortEnd + 1]) == 1)
					myPortEnd++;

				// SEND MY PORT END
				MPI_Send(&myPortEnd, 1, MPI_INT, ROOT, 1000, MPI_COMM_WORLD);

				// RECEIVE MY UPDATED PORT BOUNDARIES
				MPI_Recv(&myPortBoundaries, 2, MPI_INT, ROOT, 1001, MPI_COMM_WORLD, &status);
				myPortBegin = myPortBoundaries[0];
				myPortEnd = myPortBoundaries[1];

				int myDecMin = MAX_VAL;

				if (myPortBegin != -1 && myPortEnd != -1) {
					node** strips = generateStrips(LOCAL, myPortBegin, myPortEnd, ARRAY_LEN);
					myDecMin = findMin(strips[0]);
				}

				MPI_Send(&myDecMin, 1, MPI_INT, ROOT, 1002, MPI_COMM_WORLD);
			}
			else break;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
	time_end = MPI_Wtime();
	MPI_Finalize();
	

/******************************************************************************************/

	if(myProcId == ROOT)
	{
		printf("MS4,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);

		FILE *stdo = fopen(OUTPUT_FILE, "a");
		fprintf(stdo, "MS4,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);
		fclose(stdo);
	}
	return 0;
}
