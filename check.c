#include <stdio.h>
#include <stdlib.h>

#include "io.h"

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("usage: %s a b\n", argv[0]);
		return;
	}

	Matrix *a,*b;

	a = newMatrix(argv[1]);
	b = newMatrix(argv[2]);

	if(a->num_rows != b->num_rows)
	{
		printf("rows mismatch %d-%d\n", a->num_rows, b->num_rows);
		return 0;
	}

	if(a->num_cols != b->num_cols)
	{
		printf("cols mismatch %d-%d\n", a->num_cols, b->num_cols);
		return 0;
	}

	int i;
	int j;

	int misses = 0;

	for(i = 0; i < a->num_rows; i++)
	{
		for(j = 0; j < a->num_cols; j++)
		{
			double diff = a->rows[i][j] - b->rows[i][j];
			if(diff * diff > 0.0001f)
			{
				printf("mismatch %d:%d\n%f:%f\n", i,j,a->rows[i][j],b->rows[i][j]);
				misses++;
				if(misses >= 10)
					return;
			}
		}
	}
}