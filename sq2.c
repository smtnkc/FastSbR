#include "helper.h"

int main(int argc, char** argv)
{
	int *ARRAY;
	int *TEMP;
	int ARRAY_LEN = atoi(argv[1]);
	int i = 0, itr = 0, decMin, decMax, tempDecMin, tempDecMax;
	struct timeval time_start, time_end;

	ARRAY = readArrayFromFile(ARRAY_LEN);
	TEMP = calloc(ARRAY_LEN, sizeof(int));

	#if PRINT_SORTING_STEPS
		printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS AT THE BEGINNING
	#endif

	gettimeofday(&time_start, NULL);

	while(hasBreakpoints(ARRAY, ARRAY_LEN))
	{
		itr++;
		memcpy(&TEMP[0], ARRAY, ARRAY_LEN * sizeof(int));

		node** strips = generateStrips(ARRAY, 0, ARRAY_LEN-1, ARRAY_LEN); // NULL IF ARRAY IS EMPTY

		decMin = MAX_VAL; // RESET
		decMax = MIN_VAL; // RESET
		if(strips != NULL)
		{
			decMin = findMin(strips[0]);
			decMax = findMax(strips[0]);
		}

		int rhoMinStart = findIndexOf(decMin-1, ARRAY, ARRAY_LEN);
		int rhoMinEnd = findIndexOf(decMin, ARRAY, ARRAY_LEN);
		if(rhoMinStart > rhoMinEnd)
			swap(&rhoMinStart, &rhoMinEnd);

		int rhoMaxStart = findIndexOf(decMax, ARRAY, ARRAY_LEN);
		int rhoMaxEnd = findIndexOf(decMax+1, ARRAY, ARRAY_LEN);
		if(rhoMaxStart > rhoMaxEnd)
			swap(&rhoMaxStart, &rhoMaxEnd);

		reverse(TEMP, rhoMinStart + 1, rhoMinEnd); // REVERSE TEMP BY RHO_MIN

		node** tempStrips = generateStrips(TEMP, 0, ARRAY_LEN-1, ARRAY_LEN);  // NULL IF PORTION IS EMPTY
		tempDecMin = MAX_VAL;
		tempDecMax = MIN_VAL;
		if(tempStrips != NULL)
		{
			tempDecMin = findMin(tempStrips[0]);
			tempDecMax = findMax(tempStrips[0]);
		}

		if(tempDecMin != MAX_VAL || tempDecMax != MIN_VAL)
		{
			// TEMP HAS A DECREASING STRIP
			memcpy(&ARRAY[0], TEMP, ARRAY_LEN * sizeof(int)); // KEEP THE REVERSAL RHO_MIN
		}
		else
		{
			reverse(ARRAY, rhoMaxStart, rhoMaxEnd-1); // REVERSE ARRAY BY RHO_MAX

			strips = generateStrips(ARRAY, 0, ARRAY_LEN-1, ARRAY_LEN); // NULL IF ARRAY IS EMPTY

			decMin = MAX_VAL; // RESET
			decMax = MIN_VAL; // RESET
			if(strips != NULL)
			{
				decMin = findMin(strips[0]);
				decMax = findMax(strips[0]);
			}

			if(decMin == MAX_VAL && decMax == MIN_VAL && hasBreakpoints(ARRAY, ARRAY_LEN))
			{
				// ALL STRIPS ARE INCREASING
				reverseAnInc(ARRAY, ARRAY_LEN);
			}
		}

		#if PRINT_SORTING_STEPS
			printArray(ARRAY, 0, ARRAY_LEN-1); // PRINTS IN EACH STEP
		#endif
	}

	gettimeofday(&time_end, NULL);

	double passedTime = ((double) (time_end.tv_usec - time_start.tv_usec) / 1000000 + (double) (time_end.tv_sec - time_start.tv_sec));

	printf("SQ2,%d,%d,%d,%f\n", ARRAY_LEN, 1, itr, passedTime);

	FILE *stdo = fopen("stats.csv", "a");
	fprintf(stdo, "SQ2,%d,%d,%d,%f\n", ARRAY_LEN, 1, itr, passedTime);
	fclose(stdo);
}
