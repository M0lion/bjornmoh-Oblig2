#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#include "io.h"

#define NUM_THREADS 4

//make sure each process has the required data
void distributeData();

//do calculation
void multiply();

//gather all the data in c in proc 0
void consolidateData();

int my_rank, num_procs;

Matrix *a, *b, c;

int num_rows;
int num_cols;

int depth;

int rowsPerProc;
int startRow;
int endRow;

double **rows;

double **result;

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("usage: %s a b output\n", argv[0]);
		return 0;
	}

	MPI_Init (&argc, &argv);

	MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size (MPI_COMM_WORLD, &num_procs);

	if(my_rank == 0)
	{
		a = newMatrix(argv[1]);
		b = newMatrix(argv[2]);
	}

	distributeData();
	multiply();
	consolidateData();


	if(my_rank == 0)
	{
		write_matrix_binaryformat (argv[3], c.rows, c.num_rows, c.num_cols);
		delMatrix(a);
	}

	delMatrix(b);

	MPI_Finalize ();

	return 0;
}

void multiply()
{
	result = malloc(rowsPerProc * sizeof(double*));
	result[0] = malloc(rowsPerProc * num_cols * sizeof(double));
	int i;
	int j;
	int k;

	#pragma omp parallel shared(result) private(i,j,k) num_threads(NUM_THREADS)
	{
		printf("Hello MP %d\n", omp_get_thread_num());

		#pragma omp for
		for(i = 0; i < rowsPerProc; i++)
		{
			result[i] = result[0] + (num_cols * i);
		}

		#pragma omp for collapse(2) nowait
		for(i = 0; i < rowsPerProc; i++)
		{
			for(j = 0; j < num_cols; j++)
			{
				result[i][j] = 0;
				for(k = 0; k < depth; k++)
				{
					result[i][j] += rows[i][k] * b->rows[k][j];
				}
			}
		}
	}//end parallel
}

void consolidateData()
{
	int i;
	if(my_rank == 0)
	{
		c.num_rows = num_rows;
		c.num_cols = num_cols;
		c.rows = malloc(num_rows * sizeof(double*));
		c.rows[0] = malloc(num_rows * num_cols * sizeof(double));
		int row = 0;
		for(i = 0; i < rowsPerProc; i++)
		{
			c.rows[row] = c.rows[0] + (row * num_cols);
			memcpy(c.rows[row], result[i], num_cols * sizeof(double));
			row++;
		}

		int j;
		for(i = 1; i < num_procs; i++)
		{
			for(j = 0; j < rowsPerProc; j++)
			{
				c.rows[row] = c.rows[0] + (row * num_cols);
				MPI_Recv(c.rows[row], num_cols, MPI_DOUBLE, i, 101, MPI_COMM_WORLD, 0);
				row++;
			}
		}

		i--;
		while(row < num_rows)
		{
			MPI_Recv(c.rows[row], num_cols, MPI_DOUBLE, i, 101, MPI_COMM_WORLD, 0);
			row++;
		}
	}
	else
	{
		for(i = 0; i < rowsPerProc; i++)
		{
				MPI_Send(result[i], num_cols, MPI_DOUBLE, 0, 101, MPI_COMM_WORLD);
		}
	}
}

void distributeData()
{
	if(my_rank == 0)
	{
		num_rows = a->num_rows;
		num_cols = b->num_cols;
		depth = a->num_cols;
	}

	MPI_Bcast(&num_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&num_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&depth, 1, MPI_INT, 0, MPI_COMM_WORLD);

	rowsPerProc = num_rows / num_procs;
	startRow = rowsPerProc * my_rank;
	endRow = startRow + rowsPerProc;

	if(my_rank == num_procs - 1)
	{
		endRow += num_rows % num_procs;
		rowsPerProc += num_rows % num_procs;
	}

	if(my_rank == 0)
	{
		int i;
		int j;

		//send rows
		int row = endRow;
		for(i = 1; i < num_procs; i++)
		{
			for(j = 0; j < rowsPerProc; j++)
			{
				MPI_Send(a->rows[row], depth, MPI_DOUBLE, i, 101, MPI_COMM_WORLD);
				row++;
			}
		}
		i--;
		while(row < num_rows)
		{
			MPI_Send(a->rows[row], depth, MPI_DOUBLE, i, 101, MPI_COMM_WORLD);
			row++;
		}

		//send b
		
		MPI_Bcast(&b->num_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&b->num_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

		for(i = 0; i < b->num_rows; i++)
		{
			MPI_Bcast(b->rows[i], b->num_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		}

		rows = a->rows;
	}
	else
	{
		int i;
		int j;

		//recv rows
		rows = malloc(rowsPerProc * sizeof(double*));

		for(i = 0; i < rowsPerProc; i++)
		{
			rows[i] = malloc(num_cols * sizeof(double));
			MPI_Recv(rows[i], depth, MPI_DOUBLE, 0, 101, MPI_COMM_WORLD, 0);
		}

		//recv b
		b = malloc(sizeof(Matrix));
		b->rows = 0;
		MPI_Bcast(&b->num_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&b->num_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

		b->rows = malloc(b->num_rows * sizeof(double*));
		for(i = 0; i < b->num_rows; i++)
		{
			b->rows[i] = malloc(sizeof(double) * b->num_cols);
			MPI_Bcast(b->rows[i], b->num_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		}
	}
}