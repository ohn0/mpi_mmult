#include "mmult.h"

int main(int argc, char** argv)
{
	int nrows, ncols;
	struct matrix* A = malloc(sizeof(struct matrix));
	struct matrix* B = malloc(sizeof(struct matrix));
	struct matrix* C = malloc(sizeof(struct matrix));
	double* vector;
	int myid, numprocs, sender;
	int rowsCompleted = 0;
	int currentRow = 0;
	double starttime, endtime;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
			gen_matrix(argv[1], A);
			gen_matrix(argv[2], B);

	if(argc > 1){
	//	gen_matrix(argv[1], A);
	//	gen_matrix(argv[2], B);
		int ansDimension = A->columns * B->rows;
		C->matrix = malloc(sizeof(double) * ansDimension);

//		printf("Hi from %d\n%p\n%p\n%p\n", myid,A,B,C);
		if(myid == 0){
			printf("master\n");

			MPI_Bcast(A->matrix, A->rows * A->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(B->matrix, B->rows * B->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(A->rows,1,  MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(A->columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
			printf("%d\n", A->columns);			
vector = malloc(sizeof(double) * A->columns);
			MPI_Bcast(vector, A->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			int k;
			for(k = 0; k < min(numprocs-1, A->rows); k++){
				int j;
				for(j = 0; j < A->columns; j++){
					vector[j] = A->matrix[(rowsCompleted * A->columns) + j];
					printf("%d ", (rowsCompleted * A->columns) + j);
				}
				printf("sending...\n");
				MPI_Send(vector, A->columns, MPI_DOUBLE, k+1, k+1, MPI_COMM_WORLD);
				rowsCompleted++;
			}
			while(rowsCompleted <  A->rows){
				rowsCompleted++;		
			}
			
		}
		else{
			MPI_Bcast(A->matrix, A->rows * A->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(B->matrix, B->rows * B->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(A->rows,1,  MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(A->columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(vector, A->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			printf("In slave\n");;
			while(1){
				MPI_Recv(vector, A->columns, MPI_DOUBLE, 0, MPI_ANY_TAG,
					 MPI_COMM_WORLD, &status);
				if(status.MPI_TAG == 0){
					break;
				}
			}
			printf("%d got the vector.\n", myid);
			for(int i = 0; i < A->columns; i++){
				printf("%1.f ", vector[i]);
			}
			printf("\n");
		}

	}
	

	//Need to free all the matrices.
	free(A);
	free(B);
	free(C);
	MPI_Finalize();
	return 0;
}
