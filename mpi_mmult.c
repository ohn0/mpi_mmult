#include "mmult.h"

int main(int argc, char** argv)
{
	//This program performs NxM matrix multiplication given two matrices, A and B.
	//The result is stored in matrix C and is written to the file results.
	//The way I did it is to take A and split each row into an individual vector as 
	//the master.
	//The master then sends all the vectors to the slaves.
	//Each of the slaves also gets a copy of the B matrix.
	//Each slave then takes the vector and takes each element from the vector and 
	//multiplies it by a column of the B matrix, summing up the results as well.
	//The resultant value is then placed into a vector that the slave allocates.
	//Once all the values are added into the vector, the slave sends that vector
	//back to the master, who places it into the C matrix.
	//The master waits for all the rows to be processed, and once they are done,
	//terminates the slaves and writes the resultant matrix into the results file
	//and quits.
	if(argc == 1){printf("Invalid number of arguments. quitting.\n"); return 0;}
	int nrows, ncols;

	//Allocate matrix structs.
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

	//Initialize MPI API
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);


	int ansDimension;
	int vecSize;

	//Entering master block
	if(myid == 0){
		//Fill the matrix structs with values from the provided files.
		gen_matrix(argv[1], A);
		gen_matrix(argv[2], B);
		//matSize holds the number of values in each matrix.
		matSize[0] = A->rows * A->columns;
		matSize[1] = B->rows * B->columns;
		//Ensure that the matrices can be legally multiplied.
		if(B->rows != A->columns){
			validFlag = 0;
		}
		//start timer. Broadcast the size and validity to slaves.
		starttime = MPI_Wtime();
		MPI_Bcast(&validFlag, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(matSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
		
		//Broadcast the matrices, their rows, and their columsn to all the slaves.
		//Letting the slaves generate their own matrices resulted in lots of
		//file reading errors.
		MPI_Bcast(A->matrix, matSize[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(B->matrix, matSize[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(A->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(B->rows),1,  MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(A->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&(B->columns), 1, MPI_INT, 0, MPI_COMM_WORLD);
		int i;
		
		//Allocate the result matrix(C), along with it's dimension.
		//Allocate a vector that will hold a row from A and be given to all the slaves.
		//Broadcast the size of the vector, the slaves will know what to do with it.
		ansDimension = A->rows * B->columns;
		C->matrix = malloc(sizeof(double) * ansDimension);
		vector = malloc(sizeof(double) * A->columns);
		vecSize = sizeof(double) * A->columns;
		MPI_Bcast(&vecSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if(validFlag){
			//If matrix multiplication is valid, convert each row from A into a vector and send
			//that vector to a slave.
			int k;
			for(k = 0; k < min(numprocs-1, A->rows); k++){
				int j;
				for(j = 0; j < A->columns; j++){
					vector[j] = A->matrix[(rowsCompleted * A->columns) + j];
				}
				MPI_Send(vector, A->columns, MPI_DOUBLE, k+1, k+1, MPI_COMM_WORLD);
				rowsCompleted++;
			}
			//If we still had rows to process(more rows than num processors), 
			//then we start waiting for a slave to be free and sending the slave
			//a new vector to work on.
			//Once all rows are done, the master breaks from this loop and gets the end time.
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
			//Write the results out.
			for(i = 0; i < ansDimension; i++){
				if(i%A->columns == 0){printf("\n");}
				printf("%lf ", C->matrix[i]);
			}
			printf("\n");
			//Stop the slaves.
			for(i = 1; i <= min(numprocs-1, A->rows) ;i++){
				MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			}
		}
		
	}
	else{
		//Slave code
		//Wait for all the information from the master to arrive.
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
		//Allocate the vector that will hold the row from A.
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
			//Begin working on the current vector, multiplying it with 
			//each column of B and summing the result, putting the value
			//in another vector. Once done, send result to master and wait
			//for another vector or quit if the master is done working.
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
	//Compare results with the single threaded mmult method.
	//Deallocate matrices and quit.
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
