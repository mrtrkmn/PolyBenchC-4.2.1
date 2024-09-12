/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* atax.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "atax.h"

/* Array initialization. */
static void init_array(int m, int n,
                       DATA_TYPE **A,
                       DATA_TYPE *x)
{
  int i, j;
  DATA_TYPE fn;
  fn = (DATA_TYPE)n;

  for (i = 0; i < n; i++)
    x[i] = 1 + (i / fn);
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      A[i][j] = (DATA_TYPE)((i + j) % n) / (5 * m);
}

/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static void print_array(int n,
                        DATA_TYPE *y)
{
  int i;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("y");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(POLYBENCH_DUMP_TARGET, "\n");
    fprintf(POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, y[i]);
  }
  POLYBENCH_DUMP_END("y");
  POLYBENCH_DUMP_FINISH;
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static void kernel_atax(int m, int n,
                        DATA_TYPE **A,
                        DATA_TYPE *x,
                        DATA_TYPE *y,
                        DATA_TYPE *tmp)
{
  int i, j;

#pragma scop
  for (i = 0; i < n; i++)
    y[i] = 0;
  for (i = 0; i < m; i++)
  {
    tmp[i] = SCALAR_VAL(0.0);
    for (j = 0; j < n; j++)
      tmp[i] = tmp[i] + A[i][j] * x[j];
    for (j = 0; j < n; j++)
      y[j] = y[j] + A[i][j] * tmp[i];
  }
#pragma endscop
}

int main(int argc, char **argv)
{
  /* Retrieve problem size from command-line arguments or defaults. */
  int m = M;
  int n = N;

  if (argc > 2)
  {
    m = atoi(argv[1]);
    n = atoi(argv[2]);
  }
  else
  {
    printf("Using default problem sizes: M=%d, N=%d\n", m, n);
  }

  /* Dynamic memory allocation. */
  DATA_TYPE **A = (DATA_TYPE **)malloc(m * sizeof(DATA_TYPE *));
  for (int i = 0; i < m; i++)
  {
    A[i] = (DATA_TYPE *)malloc(n * sizeof(DATA_TYPE));
  }
  DATA_TYPE *x = (DATA_TYPE *)malloc(n * sizeof(DATA_TYPE));
  DATA_TYPE *y = (DATA_TYPE *)malloc(n * sizeof(DATA_TYPE));
  DATA_TYPE *tmp = (DATA_TYPE *)malloc(m * sizeof(DATA_TYPE));

  /* Initialize array(s). */
  init_array(m, n, A, x);

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_atax(m, n, A, x, y, tmp);

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, y));

  /* Free dynamically allocated memory. */
  for (int i = 0; i < m; i++)
  {
    free(A[i]);
  }
  free(A);
  free(x);
  free(y);
  free(tmp);

  return 0;
}
