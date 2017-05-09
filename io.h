
#ifndef __IO__
#define __IO__


void write_matrix_binaryformat (char* filename, double** matrix, int num_rows, int num_cols);
void read_matrix_binaryformat (char* filename, double*** matrix, int* num_rows, int* num_cols);

typedef struct
{
	double **rows;
	int num_rows;
	int num_cols;
} Matrix;

Matrix* newMatrix(char* filename);
void delMatrix(Matrix *matrix);

#endif