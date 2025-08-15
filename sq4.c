#include "helper.h"

int main(int argc, char** argv)
{
	int *ARRAY;
	int ARRAY_LEN = atoi(argv[1]);
	int i = 0, itr = 0, decMin;
	struct timeval time_start, time_end;

	ARRAY = readArrayFromFile(ARRAY_LEN);

	#if PRINT_SORTING_STEPS
		printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS AT THE BEGINNING
	#endif

	gettimeofday(&time_start, NULL);

	while(hasBreakpoints(ARRAY, ARRAY_LEN))
	{
		itr++;
		node** strips = generateStrips(ARRAY, 0, ARRAY_LEN-1, ARRAY_LEN); // NULL IF ARRAY IS EMPTY

		decMin = MAX_VAL; // RESET
		if(strips != NULL)
		{
			decMin = findMin(strips[0]);
		}

		if(decMin != MAX_VAL)
		{
			// DECREASING STRIP EXISTS
			int rhoMinStart = findIndexOf(decMin-1, ARRAY, ARRAY_LEN);
			int rhoMinEnd = findIndexOf(decMin, ARRAY, ARRAY_LEN);
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

	gettimeofday(&time_end, NULL);

	double passedTime = ((double) (time_end.tv_usec - time_start.tv_usec) / 1000000 + (double) (time_end.tv_sec - time_start.tv_sec));

	printf("SQ4,%d,%d,%d,%f\n", ARRAY_LEN, 1, itr, passedTime);

	FILE *stdo = fopen("stats.csv", "a");
	fprintf(stdo, "SQ4,%d,%d,%d,%f\n", ARRAY_LEN, 1, itr, passedTime);
	fclose(stdo);

	return 0;
}