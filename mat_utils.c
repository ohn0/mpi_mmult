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

int print_matrix(struct matrix* M)
{	
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

int free_matrix(struct matrix* M)
{
	free(M->matrix);
	free(M);
	return 0;
}

int mmult(struct matrix* A, struct matrix* B, struct matrix* C){
  int i, j, k;
  for (i = 0; i < A->rows; i++) {
    for (j = 0; j < B->columns; j++) {
      C->matrix[i*B->columns + j] = 0;
    }
    for (k = 0; k < A->columns; k++) {
      for (j = 0; j < B->columns; j++) {
	C->matrix[i*B->columns + j] += A->matrix[i*A->columns + k] * 
				       B->matrix[k*B->columns + j];
      }
    }
  }
  return 0;
}

void compare_matrices(struct matrix* A, struct matrix* B){
	int n = A->rows * B->columns;
	int i, j, k;
	for (k = 0; k < n; ++k) {
		if (fabs(A->matrix[k]-B->matrix[k])/fabs(A->matrix[k]) > 1e-12) {
			i = k/B->columns;
			j = k%B->columns;
			printf("a[%d][%d] == %.12g\nb[%d][%d] == %.12g\ndelta == %.12g\nrelerr == %.12g\n",
				i, j, A->matrix[k], i, j, B->matrix[k], fabs(A->matrix[k]-B->matrix[k]), 
				fabs(A->matrix[k]-B->matrix[k])/fabs(A->matrix[k]));
			return;
		}
	}
	printf("Matrices are the same\n");
}

