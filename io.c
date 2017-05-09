#include "io.h"

#include <stdlib.h>
#include <stdio.h>

Matrix* newMatrix(char* filename)
{
	Matrix *matrix = malloc(sizeof(Matrix));
	read_matrix_binaryformat (filename, &matrix->rows, &(matrix->num_rows), &(matrix->num_cols));
	return matrix;
}

void delMatrix(Matrix *matrix)
{
	if(matrix->rows != 0)
		free(matrix->rows);
	free(matrix);
}

void write_matrix_binaryformat (char* filename, double** matrix, int num_rows, int num_cols)
{
	FILE *fp = fopen (filename,"wb");
	
	fwrite (&num_rows, sizeof(int), 1, fp);
	fwrite (&num_cols, sizeof(int), 1, fp);
	fwrite (matrix[0], sizeof(double), num_rows*num_cols, fp);
	fclose (fp);
}

void read_matrix_binaryformat (char* filename, double*** matrix, int* num_rows, int* num_cols)
{
	int i;
	FILE* fp = fopen (filename,"rb");
	fread (num_rows, sizeof(int), 1, fp);
	fread (num_cols, sizeof(int), 1, fp);

	/* storage allocation of the matrix */
	*matrix = (double**)malloc((*num_rows)*sizeof(double*));
	(*matrix)[0] = (double*)malloc((*num_rows)*(*num_cols)*sizeof(double));

	for (i=1; i<(*num_rows); i++)
		(*matrix)[i] = (*matrix)[i-1]+(*num_cols);

	/* read in the entire matrix */
	fread ((*matrix)[0], sizeof(double), (*num_rows)*(*num_cols), fp);
	fclose (fp);
}
