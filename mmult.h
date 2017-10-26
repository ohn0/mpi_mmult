#ifndef _MPI_H
#define _MPI_H
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/times.h>
#define min(x, y) ((x)<(y)?(x):(y))
struct matrix{
	int rows;
	int columns;
	double* matrix;
};
int gen_matrix(char*, struct matrix*);
int print_matrix(struct matrix*);
int free_matrix(struct matrix*);
int mmult(struct matrix*, struct matrix*, struct matrix*);
void compare_matrices(struct matrix*, struct matrix*);
#endif
