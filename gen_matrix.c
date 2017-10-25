#include "mmult.h"

int gen_matrix(char* fileA, struct matrix* M)
{
	double* matrix;
	FILE* fp = fopen(fileA, "r");
	int rowCount = 0;
	int newlineCounter = 0;
	char dontCountRows = 0;
	int c=0;
	double n;
	fscanf(fp, "%d", &rowCount);
	fscanf(fp, "%d", &newlineCounter);
	printf("%d %d\n", newlineCounter, rowCount);
	matrix = malloc((int)(newlineCounter * rowCount) * sizeof(double));
	int i;
	i = 0;
//	rewind(fp);
	while(fscanf(fp, "%lf", &n) != EOF){
		matrix[i++] = n;	
	//	printf("Got %f\n", matrix[i]);
	}
	printf("wat\n");
	M->rows = rowCount;
	M->columns = newlineCounter;
	M->matrix = matrix;
	double val;
	fclose(fp);
	return 0;
}

int print_matrix(struct matrix* M){
	int i, j;
	int rowsDone = 0;
	for(i = 0; i < M->rows; i++){
		for(j = 0; j < M->columns; j++){
			printf("%lf ", M->matrix[3*i+j]);
		}
		printf("\n");
	}
	return 0;
		
}

