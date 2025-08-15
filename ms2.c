#include "helper.h"

int main(int argc, char** argv)
{ 
	int i, myProcId, nProcs, initPortSize, nWorkers, nPorts, flag=1, itr=0, ARRAY_LEN;
	int *ARRAY;
	int *TEMP;
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
		TEMP = calloc(ARRAY_LEN, sizeof(int));

		#if PRINT_SORTING_STEPS
			printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS AT THE BEGINNING
		#endif

		initPortSize = ARRAY_LEN / nPorts;

		time_start = MPI_Wtime();

		// SEND ARRAY_LEN
		for(i = 1; i <= nWorkers; i++)
			MPI_Send(&ARRAY_LEN, 1, MPI_INT, i, 5000, MPI_COMM_WORLD);

		while(hasBreakpoints(ARRAY, ARRAY_LEN))
		{
			itr++;
			memcpy(&TEMP[0], ARRAY, ARRAY_LEN * sizeof(int));

			int* gDecMinMax = getGlobalMinMax(ARRAY, nWorkers, ARRAY_LEN, status, 1000);

			int rhoMinStart = findIndexOf(gDecMinMax[0]-1, ARRAY, ARRAY_LEN);
			int rhoMinEnd = findIndexOf(gDecMinMax[0], ARRAY, ARRAY_LEN);
			if(rhoMinStart > rhoMinEnd)
				swap(&rhoMinStart, &rhoMinEnd);

			int rhoMaxStart = findIndexOf(gDecMinMax[1], ARRAY, ARRAY_LEN);
			int rhoMaxEnd = findIndexOf(gDecMinMax[1]+1, ARRAY, ARRAY_LEN);
			if(rhoMaxStart > rhoMaxEnd)
				swap(&rhoMaxStart, &rhoMaxEnd);

			reverse(TEMP, rhoMinStart + 1, rhoMinEnd); // REVERSE TEMP BY RHO_MIN

			int* gTempDecMinMax = getGlobalMinMax(TEMP, nWorkers, ARRAY_LEN, status, 2000);

			if(gTempDecMinMax[0] != MAX_VAL || gTempDecMinMax[1] != MIN_VAL)
			{
				// TEMP HAS A DECREASING STRIP
				memcpy(&ARRAY[0], TEMP, ARRAY_LEN * sizeof(int)); // KEEP THE REVERSAL RHO_MIN

				for(i = 1; i <= nWorkers; i++)
					MPI_Send(&flag, 1, MPI_INT, i, PASS_RHO_MAX, MPI_COMM_WORLD);
			}
			else
			{
				reverse(ARRAY, rhoMaxStart, rhoMaxEnd-1); // REVERSE ARRAY BY RHO_MAX

				for(i = 1; i <= nWorkers; i++)
					MPI_Send(&flag, 1, MPI_INT, i, RUN_RHO_MAX, MPI_COMM_WORLD);

				gDecMinMax = getGlobalMinMax(ARRAY, nWorkers, ARRAY_LEN, status, 3000);

				if(gDecMinMax[0] == MAX_VAL && gDecMinMax[1] == MIN_VAL && hasBreakpoints(ARRAY, ARRAY_LEN))
				{
					// ALL STRIPS ARE INCREASING
					reverseAnInc(ARRAY, ARRAY_LEN);
				}
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
		MPI_Recv(&ARRAY_LEN, 1, MPI_INT, ROOT, 5000, MPI_COMM_WORLD, &status); // RECEIVE ARRAY_LEN

		initPortSize = ARRAY_LEN / nPorts;

		while(1)
		{
			int LOCAL[ARRAY_LEN], TEMP_LOCAL[ARRAY_LEN];
			MPI_Recv(&LOCAL[0], ARRAY_LEN, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if (status.MPI_TAG == COMPUTE)
			{
				int* myDecMinMax = getLocalMinMax(LOCAL, nWorkers, myProcId, ARRAY_LEN, status, 1000);
				MPI_Send(myDecMinMax, 2, MPI_INT, ROOT, 1002, MPI_COMM_WORLD);

				MPI_Recv(&TEMP_LOCAL[0], ARRAY_LEN, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				int* myTempDecMinMax = getLocalMinMax(TEMP_LOCAL, nWorkers, myProcId, ARRAY_LEN, status, 2000);
				MPI_Send(myTempDecMinMax, 2, MPI_INT, ROOT, 2002, MPI_COMM_WORLD);

				MPI_Recv(&flag, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				if(status.MPI_TAG == RUN_RHO_MAX)
				{
					MPI_Recv(&LOCAL[0], ARRAY_LEN, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
					int* myDecMinMax = getLocalMinMax(LOCAL, nWorkers, myProcId, ARRAY_LEN, status, 3000);
					MPI_Send(myDecMinMax, 2, MPI_INT, ROOT, 3002, MPI_COMM_WORLD);
				}
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
		printf("MS2,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);

		FILE *stdo = fopen(OUTPUT_FILE, "a");
		fprintf(stdo, "MS2,%d,%d,%d,%f\n", ARRAY_LEN, nProcs, itr, time_end - time_start);
		fclose(stdo);
	}
	return 0;
}
