
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _SolverOptions
{
  int solverId;
  int isSet;
  char solverName[64] ;
  int iSize;
  int * iparam;
  int dSize;
  double * dparam;
  int filterOn;
  double * dWork;
  int * iWork;
  int numberOfInternalSolvers;
  struct _SolverOptions * internalSolvers;
} SolverOptions;

typedef struct
{
  int size;
  double * M;
  double * q;
} LinearComplementarityProblem;


void lcp_lexicolemke(LinearComplementarityProblem* problem, double *zlem , double *wlem , int *info , SolverOptions* options)
{
  /* matrix M of the lcp */
  double * M = problem->M;

  /* size of the LCP */
  int dim = problem->size;
  int dim2 = 2*(dim+1);

  int i,drive,block,Ifound;
  int ic,jc;
  int ITER;
  int nobasis;
  int itermax = options->iparam[0];


  double z0,zb,dblock;
  double pivot, tovip;
  double tmp;
  int *basis;
  double** A;

  /*output*/

  options->iparam[1] = 0;

  /* Allocation */

  basis = (int *)malloc(dim*sizeof(int));
  A = (double **)malloc(dim*sizeof(double*));

  for (ic = 0 ; ic < dim; ++ic)
    A[ic] = (double *)malloc(dim2*sizeof(double));

  for (ic = 0 ; ic < dim; ++ic)
    for (jc = 0 ; jc < dim2; ++jc)
      A[ic][jc] = 0.0;

  /* construction of A matrix such that
   * A = [ q | Id | -d | -M ] with d = (1,...1)
   */

  for (ic = 0 ; ic < dim; ++ic)
    for (jc = 0 ; jc < dim; ++jc)
      A[ic][jc+dim+2] = -M[dim*jc+ic];

  for (ic = 0 ; ic < dim; ++ic) A[ic][0] = problem->q[ic];

  for (ic = 0 ; ic < dim; ++ic) A[ic][ic+1 ] =  1.0;
  for (ic = 0 ; ic < dim; ++ic) A[ic][dim+1] = -1.0;

  /* End of construction of A */

  Ifound = 0;


  for (ic = 0 ; ic < dim  ; ++ic) basis[ic]=ic+1;

  drive = dim+1;
  block = 0;
  z0 = A[block][0];
  ITER = 0;

  /* Start research of argmin lexico */
  /* With this first step the covering vector enter in the basis */

  for (ic = 1 ; ic < dim ; ++ic)
  {
    zb = A[ic][0];
    if (zb < z0)
    {
      z0    = zb;
      block = ic;
    }
    else if (zb == z0)
    {
      for (jc = 0 ; jc < dim ; ++jc)
      {
        dblock = A[block][1+jc] - A[ic][1+jc];
        if (dblock < 0)
        {
          break;
        }
        else if (dblock > 0)
        {
          block = ic;
          break;
        }
      }
    }
  }

  /* Stop research of argmin lexico */

  pivot = A[block][drive];
  tovip = 1.0/pivot;

  /* Pivot < block , drive > */

  A[block][drive] = 1;
  for (ic = 0       ; ic < drive ; ++ic) A[block][ic] = A[block][ic]*tovip;
  for (ic = drive+1 ; ic < dim2  ; ++ic) A[block][ic] = A[block][ic]*tovip;

  /* */

  for (ic = 0 ; ic < block ; ++ic)
  {
    tmp = A[ic][drive];
    for (jc = 0 ; jc < dim2 ; ++jc) A[ic][jc] -=  tmp*A[block][jc];
  }
  for (ic = block+1 ; ic < dim ; ++ic)
  {
    tmp = A[ic][drive];
    for (jc = 0 ; jc < dim2 ; ++jc) A[ic][jc] -=  tmp*A[block][jc];
  }

  nobasis = basis[block];
  basis[block] = drive;

  while (ITER < itermax && !Ifound)
  {

    ++ITER;

    if (nobasis < dim + 1)      drive = nobasis + (dim+1);
    else if (nobasis > dim + 1) drive = nobasis - (dim+1);

    /* Start research of argmin lexico for minimum ratio test */

    pivot = 1e20;
    block = -1;

    for (ic = 0 ; ic < dim ; ++ic)
    {
      zb = A[ic][drive];
      if (zb > 0.0)
      {
        z0 = A[ic][0]/zb;
        if (z0 > pivot) continue;
        if (z0 < pivot)
        {
          pivot = z0;
          block = ic;
        }
        else
        {
          for (jc = 1 ; jc < dim+1 ; ++jc)
          {
            dblock = A[block][jc]/pivot - A[ic][jc]/zb;
            if (dblock < 0) break;
            else if (dblock > 0)
            {
              block = ic;
              break;
            }
          }
        }
      }
    }
    if (block == -1) break;

    if (basis[block] == dim+1) Ifound = 1;

    /* Pivot < block , drive > */

    pivot = A[block][drive];
    tovip = 1.0/pivot;
    A[block][drive] = 1;

    for (ic = 0       ; ic < drive ; ++ic) A[block][ic] = A[block][ic]*tovip;
    for (ic = drive+1 ; ic < dim2  ; ++ic) A[block][ic] = A[block][ic]*tovip;

    /* */

    for (ic = 0 ; ic < block ; ++ic)
    {
      tmp = A[ic][drive];
      for (jc = 0 ; jc < dim2 ; ++jc) A[ic][jc] -=  tmp*A[block][jc];
    }
    for (ic = block+1 ; ic < dim ; ++ic)
    {
      tmp = A[ic][drive];
      for (jc = 0 ; jc < dim2 ; ++jc) A[ic][jc] -=  tmp*A[block][jc];
    }

    nobasis = basis[block];
    basis[block] = drive;

  } /* end while*/

  for (ic = 0 ; ic < dim; ++ic)
  {
    drive = basis[ic];
    if (drive < dim + 1)
    {
      zlem[drive-1] = 0.0;
      wlem[drive-1] = A[ic][0];
    }
    else if (drive > dim + 1)
    {
      zlem[drive-dim-2] = A[ic][0];
      wlem[drive-dim-2] = 0.0;
    }
  }

  options->iparam[1] = ITER;

  if (Ifound) *info = 0;
  else *info = 1;

  free(basis);

  for (i = 0 ; i < dim ; ++i) free(A[i]);
  free(A);
}


int setDefaultSolverOptions(SolverOptions* options)
{
  int i;
    printf("Set the Default SolverOptions for the Lemke Solver\n");


  strcpy(options->solverName,"Lemke");

  options->numberOfInternalSolvers=0;
  options->isSet=1;
  options->filterOn=1;
  options->iSize=10;
  options->dSize =10;
  options->iparam = (int *)malloc(options->iSize*sizeof(int));
  options->dparam = (double *)malloc(options->dSize*sizeof(double));
  options->dWork =NULL;
  options->iWork =NULL;
  for (i=0; i<10; i++)
  {
    options->iparam[i]=0;
    options->dparam[i]=0.0;
  }
  options->dparam[0]=1e-6;
  options->iparam[0]=10000;


  return 0;
}

int main()
{
	SolverOptions so;
	setDefaultSolverOptions(&so);
	LinearComplementarityProblem lcp;
	int info;
	int dim = 10;
	double *zlem, *wlem;
	FILE *M, *q;
	int i, j;
	double *wlem_test;
	double min_test;
	
	/* input data */
	lcp.size = dim;
	lcp.M = (double*)malloc(sizeof(double)*dim*dim);
	lcp.q = (double*)malloc(sizeof(double)*dim);
	
	M = fopen("M.dat", "r");
	if (!M)
	{
		printf("M.dat open fail.\n");
		return -10;
	}
	q = fopen("q.dat", "r");
	if (!q)
	{
		printf("q.dat open fail.\n");
		return -20;
	}
	printf("M matrix ========================\n");
	for (i = 0; i < dim*dim; ++i)
	{
		double mii;
		
		if (i%dim==0)
			printf("\n");
			
		fscanf(M, "%lf", &mii);
		lcp.M[i] = mii;
		
		printf("%10.4lf ", mii);
	}
	printf("\n\n");
	printf("q vector ========================\n\n");
	for (i = 0; i < dim; ++i)
	{
		double qi;
		fscanf(q, "%lf", &qi);
		lcp.q[i] = qi;
		
		printf("%10.4lf ", qi);
	}
	printf("\n");
	
	/* output data */
	zlem = (double*)malloc(sizeof(double) * dim);
	wlem = (double*)malloc(sizeof(double) * dim);
	lcp_lexicolemke(&lcp, zlem, wlem, &info, &so);
	
	printf("\nSolution vector =============================\n\n");
	printf("info = %d\n", info);
	for (i = 0; i < dim; ++i)
	{
		printf("%d: %20lf %20lf\n", i, zlem[i], wlem[i]);
	}
	
	printf("\nChecking solution...\n");
	wlem_test = (double*)malloc(sizeof(double) * dim);
	for (i = 0; i < dim; ++i)
	{
		wlem_test[i] = 0;
		for (j = 0; j < dim; ++j)
		{
			wlem_test[i] += lcp.M[dim*i+j] * zlem[j];
		}
		wlem_test[i] += lcp.q[i];
		printf("w diff %d: %20lf\n", i, wlem_test[i] - wlem[i]);
	}
	printf("\nValue of z' * ( M * z + q )\n");
	min_test = 0;
	for (i = 0; i < dim; ++i)
		min_test += zlem[i] * wlem_test[i];
	printf("%lf\n", min_test);
	
	
	free(zlem);
	free(wlem);
	free(wlem_test);
	free(lcp.M);
	free(lcp.q);
	return 0;
}
