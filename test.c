#include "mmult.h"
int main(int argc, char** argv)
{
	struct matrix* M = malloc(sizeof(struct matrix));
	gen_matrix(argv[1], M);
	printf("%f\n", M->matrix[0]);
}
