//#define MPI_USED
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
	int matSize[2];
	double starttime, endtime;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);


	if(argc > 1){

	//	int ansDimension = A->columns * B->rows;
		int ansDimension;
		int vecSize;
	//	C->matrix = malloc(sizeof(double) * ansDimension);

//		printf("Hi from %d\n%p\n%p\n%p\n", myid,A,B,C);
		if(myid == 0){
			gen_matrix(argv[1], A);
			gen_matrix(argv[2], B);
			print_matrix(A);
			matSize[0] = A->rows * A->columns;
			matSize[1] = B->rows * B->columns;
			MPI_Bcast(matSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
			
			MPI_Bcast(A->matrix, matSize[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(B->matrix, matSize[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(&(A->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&(B->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&(A->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&(B->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
			printf("MATRIX A:\n");
			int i;

			ansDimension = B->columns * A->rows;
			C->matrix = malloc(sizeof(double) * ansDimension);
			printf("Master %d\nA rows MASTER: %d", ansDimension, A->rows);
			vector = malloc(sizeof(double) * A->columns);
			vecSize = sizeof(double) * A->columns;
			MPI_Bcast(&vecSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
			int k;
			for(k = 0; k < min(numprocs-1, A->rows); k++){
				int j;
				for(j = 0; j < A->columns; j++){
					vector[j] = A->matrix[(rowsCompleted * A->columns) + j];
					printf("%d ", vector[j]);//printf("%d ", (rowsCompleted * A->columns) + j);
				}
				printf("sending vector %d\n", k);
				MPI_Send(vector, A->columns, MPI_DOUBLE, k+1, k+1, MPI_COMM_WORLD);
				rowsCompleted++;
			}
			printf("Master out\n");
			int rowsProcessed = 0;
			while(rowsCompleted <=  A->rows){
				int j;
				int freeSlave;
				MPI_Recv(&C->matrix[A->rows * rowsProcessed],
					A->rows, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
					&status);
				printf("Master got a matrix.\n");
				int i; rowsProcessed++;
				for(i = 0; i < A->rows; i++){
					printf("%lf ", C->matrix[rowsProcessed * A->rows + i]);
				}
				printf("\n");
				for(j = 0; j < A->columns; j++){
					vector[j] = A->matrix[(rowsCompleted * A->columns) + j];
				}
				
			}
			printf("not stuck\n");
			
		}
		else{

			MPI_Bcast(matSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
			printf("Waiting...\nm1:%d\n2:%d\n", matSize[0], matSize[1]);
			A->matrix = malloc(sizeof(double) * matSize[0]);
			B->matrix = malloc(sizeof(double) * matSize[1]);
			MPI_Bcast(A->matrix, matSize[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(B->matrix, matSize[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);
			printf("wait before  vec\n");
			MPI_Bcast(&(A->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&(B->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
			printf("wait on vec Arows: %d\n", A->rows);
			MPI_Bcast(&(A->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&(B->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&vecSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
			vector = malloc(vecSize);
	//		MPI_Bcast(vector, A->columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			ansDimension = B->columns * A->rows;
			printf("Slave %d\n", ansDimension);
			while(1){
				printf("Beginning to wait.\n");
				MPI_Recv(vector, A->columns, MPI_DOUBLE, 0, MPI_ANY_TAG,
					 MPI_COMM_WORLD, &status);
				if(status.MPI_TAG == myid){
					printf("Slave breaking\n");
					break;
				}
				else{
					printf("Slave %d's MPI_TAG: %d\n", myid, status.MPI_TAG);
				}
			}
			printf("%d got the vector.\n", myid);
			double* result = malloc(sizeof(double) * B->columns);
			int i;
			
			for(i = 0; i < A->columns; i++){
			//	printf("%1.f ", vector[i]);
			}
			for(i = 0; i < A->rows; i++){
				int j;
				result[i] = 0;
				for(j = 0; j < B->columns; j++){
					result[i] += vector[j] * B->matrix[(i) + (B->rows * j)];
	//				printf("%lf * %lf: %lf ", vector[j], B->matrix[(i * B->columns) + (B->rows * j)], result[i]);
				}
			}
			for(i = 0; i < A->rows; i++){
				printf("%lf ", result[i]);
			}
			printf("\nSLAVE'S A MATRIX: \n");
			print_matrix(A);
			printf("SLAVE'S B MATRIX: \n");
			print_matrix(B);

			printf("\n");
			MPI_Send(result, A->rows, MPI_DOUBLE, 0, status.MPI_TAG, MPI_COMM_WORLD);
		}

	}
	

	//Need to free all the matrices.
	free(A);
	free(B);
	free(C);
	MPI_Finalize();
	return 0;
}
