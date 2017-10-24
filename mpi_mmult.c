#include "mmult.h"

int main(int argc, char** argv)
{
	int nrows, ncols;
	struct matrix* A = malloc(sizeof(struct matrix));
	struct matrix* B = malloc(sizeof(struct matrix));
	struct matrix* C = malloc(sizeof(struct matrix));

	int myid, numprocs;
	double starttime, endtime;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	if(argc > 1){
		gen_matrix(argv[1], A);
		gen_matrix(argv[2], B);
		C->matrix = malloc(sizeof(double) * A->columns * B->rows);
		printf("Hi from %d\n%p\n%p\n%p\n", myid,A,B,C);
	}
	

	//Need to free all the matrices.

	return 0;
}
