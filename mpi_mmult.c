#include "mmult.h"

int main(int argc, char** argv)
{
	if(argc == 1){printf("Invalid number of arguments. quitting.\n"); return 0;}
	int nrows, ncols;
	struct matrix* A = malloc(sizeof(struct matrix));
	struct matrix* B = malloc(sizeof(struct matrix));
	struct matrix* C = malloc(sizeof(struct matrix));
	double* vector = NULL;
	int myid, numprocs, sender;
	int rowsCompleted = 0;
	int currentRow = 0;
	int matSize[2];
	int validFlag = 1;
	double starttime, endtime;
	double* result = NULL;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);


	int ansDimension;
	int vecSize;
	if(myid == 0){
		gen_matrix(argv[1], A);
		gen_matrix(argv[2], B);
		print_matrix(A);
		matSize[0] = A->rows * A->columns;
		matSize[1] = B->rows * B->columns;
		if(B->rows != A->columns){
			validFlag = 0;
		}
		starttime = MPI_Wtime();
		MPI_Bcast(&validFlag, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(matSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
		
		MPI_Bcast(A->matrix, matSize[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(B->matrix, matSize[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(A->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(B->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(A->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(B->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
		int i;

		ansDimension = A->rows * B->columns;
		C->matrix = malloc(sizeof(double) * ansDimension);
		vector = malloc(sizeof(double) * A->columns);
		vecSize = sizeof(double) * A->columns;
		MPI_Bcast(&vecSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if(validFlag){
			int k;
			for(k = 0; k < min(numprocs-1, A->rows); k++){
				int j;
				for(j = 0; j < A->columns; j++){
					vector[j] = A->matrix[(rowsCompleted * A->columns) + j];
				}
				MPI_Send(vector, A->columns, MPI_DOUBLE, k+1, k+1, MPI_COMM_WORLD);
				rowsCompleted++;
			}
			int rowsProcessed = 0;
			int waitingSlaveID = 0;
			while(rowsProcessed < A->rows){
				int j;
				int freeSlave;
				double interResult[B->rows];
				printf("Waiting to receive from %d.\n", (waitingSlaveID % (numprocs-1))+1);
				MPI_Recv(interResult, B->rows, MPI_DOUBLE, (waitingSlaveID % (numprocs-1))+1,(waitingSlaveID %(numprocs-1))+1 ,MPI_COMM_WORLD,&status); waitingSlaveID++;
				int i;
				for(i = 0; i < B->rows; i++){
					C->matrix[B->rows * rowsProcessed + i] = interResult[i];
				}
				rowsProcessed++;

				if(rowsCompleted < A->rows){
				for(j = 0; j < A->columns; j++){
					vector[j] = A->matrix[(rowsCompleted * A->columns) + j];
				}
				printf("SENDING TO %d", status.MPI_TAG);
				MPI_Send(vector, A->columns, MPI_DOUBLE, status.MPI_TAG, status.MPI_TAG,
					MPI_COMM_WORLD);
				rowsCompleted++;
				}
			}
			endtime = MPI_Wtime();
			printf("Breaking from master.\n");
			for(i = 0; i < ansDimension; i++){
				if(i%A->columns == 0){printf("\n");}
				printf("%lf ", C->matrix[i]);
			}
			printf("\n");
			for(i = 1; i <= min(numprocs-1, A->rows) ;i++){
				MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			}
		}
		
	}
	else{
		
		MPI_Bcast(&validFlag, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		MPI_Bcast(matSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
		A->matrix = malloc(sizeof(double) * matSize[0]);
		B->matrix = malloc(sizeof(double) * matSize[1]);
		MPI_Bcast(A->matrix, matSize[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(B->matrix, matSize[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(A->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(B->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(A->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(B->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&vecSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		vector = malloc(vecSize);
		ansDimension = A->rows * B->columns;
		while(1 && validFlag ){
			if(myid > A->rows){printf("%d breaking.\n", myid);break;}
			MPI_Recv(vector, A->columns, MPI_DOUBLE, 0, MPI_ANY_TAG,
				 MPI_COMM_WORLD, &status);
			if(status.MPI_TAG == 0){
				printf("Slave breaking\n");
				break;
			}
			else{
				printf("Slave %d's MPI_TAG: %d\n", myid, status.MPI_TAG);
			}
			printf("%d got the vector.\n", myid);
			result = malloc(sizeof(double) * B->columns);
			int i;
			
			for(i = 0; i < B->columns; i++){
				int j;
				result[i] = 0;
				for(j = 0; j < A->columns; j++){
					result[i] += vector[j] * B->matrix[(i) + (B->rows * j)];
					}
			}
			printf("%d sending.\n", myid);	
			MPI_Send(result, B->rows, MPI_DOUBLE, 0, status.MPI_TAG, MPI_COMM_WORLD);
		}
		if(!validFlag){
			printf("Invalid matrix, slave %d quitting.\n", myid);
		}else{printf("%d done working, escaping..\n", myid);}
	}

	if(myid == 0 && !validFlag){printf("Invalid matrix entered. Master quitting.\n");}
	else if(myid == 0 && validFlag){
		printf("Total time taken: %f\n", endtime- starttime);
		struct matrix* D = malloc(sizeof(struct matrix));
		D->matrix = malloc(sizeof(double) * A->rows * B->columns);
		mmult(A, B, D);
		compare_matrices(C, D);
		char* resultFile = "results";
		FILE* result = fopen("results", "w+");
		int i;
		for(i = 0; i < A->rows * B->columns; i++){
			if(i > 0 && (i%A->columns==0)){
				fprintf(result, "\n");
			}
			fprintf(result, " %lf ", C->matrix[i]);
		}
		free_matrix(C); free_matrix(D);
	}
	free_matrix(A); free_matrix(B); 
	free(vector); free(result);
	printf("Freed everything.\n");
	MPI_Finalize();
	return 0;
}
