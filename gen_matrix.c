#include "mmult.h"

int gen_matrix(char* fileA, struct matrix* M)
{
	double* matrix;
	FILE* fp = fopen(fileA, "r");
	int rowCount = 0;
	int newlineCounter = 0;
	char dontCountRows = 0;
	int c=0;
	while((c = getc(fp)) != EOF){
		if(c != '\n'){
			if(dontCountRows == 0 && c != ' '){
				rowCount++;
			}
		}
		else{
			dontCountRows = 1;
			newlineCounter++;
		}
	}
	matrix = malloc(newlineCounter * rowCount * sizeof(double*));
	int i;
	//rewind(fp);
	for(i = 0; i < newlineCounter * rowCount; i++){
		fscanf(fp, "%f", matrix[i]);
		printf("%f ", matrix[i]);printf("\n");
	}
	M->rows = rowCount;
	M->columns = newlineCounter;
	M->matrix = matrix;
	double val;
	fclose(fp);
	return 0;
}
