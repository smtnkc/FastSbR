#include "helper.h"

int main(int argc, char** argv)
{
	srand(time(NULL));
	int ARRAY_LEN, i;
	ARRAY_LEN = atoi(argv[1]);
	int SEED = atoi(argv[2]);
	char out_file_name[50];
	sprintf(out_file_name, "elements/N%d.txt", ARRAY_LEN);

	FILE *f = fopen(out_file_name, "w");
	int *ARRAY = calloc(ARRAY_LEN,sizeof(int));
	//ARRAY = createRandomArray(ARRAY_LEN);
	ARRAY = createSemiRandomArray(ARRAY_LEN, SEED);

	for(i=0;i<ARRAY_LEN;i++)
		fprintf(f, "%d ", ARRAY[i]);
	fclose(f);
	return 0;
}
