/*
  Original Author: Joshua Stough, Washington and Lee University This
  code was initially obtained from
  http://sc12.supercomputing.org/hpceducator/PythonForParallelism/codes/parallelQuicksort.c.
  Later, Arnaud Legrand mainly made a few cosmetic changes so that it
  is easier to use for performance evaluation purposes.

  This code quicksorts a random list of size given by the argument
  (default 1M) and times both sequential quicksort and parallel (using
  Pthreads).
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "omp.h"

#define DNUM 1000000
/*Magic number, doesn't seem to be worth to parallelize sub array of size < 10000*/
#define SIZE_THRESHOLD 10000
#define output_result(version, elems, threads, results) printf("%s quicksort for %i elements using %i threads took: %lf sec.\n", version, elems, threads, results)

//for sequential and parallel implementation
int partition(double lyst[], int lo, int hi);
void quicksortHelper(double lyst[], int lo, int hi);
void quicksort(double lyst[], int size);
int isSorted(double lyst[], int size);

//for parallel implementation
void parallelQuicksortHelper(double lyst[], int low, int high);

//for the builtin qsort, for fun:
int compare_doubles(const void *a, const void *b);

/*
Main method:
-generate random list
-time sequential quicksort
-time parallel quicksort
-time standard qsort
*/
int main(int argc, char *argv[])
{
  struct timeval start, end;
  double diff;
  int start_sequential = 1;

  srand(time(NULL));            //seed random

  int NUM = DNUM;
  if (argc >= 2)                //user specified list size.
  {
    NUM = atoi(argv[1]);
  }
  if (argc >= 3) {
    if (!strcmp("--noseq", argv[2]))
      start_sequential = 0;
  }

  //Want to compare sorting on the same list,
  //so backup.
  double *lystbck = (double *) malloc(NUM * sizeof(double));
  double *lyst = (double *) malloc(NUM * sizeof(double));

  //Populate random original/backup list.
  for (int i = 0; i < NUM; i++) {
    lystbck[i] = 1.0 * rand() / RAND_MAX;
  }

  /*This goto is intentional, it's trolly enough and I'm too lazy to indent
   * And that's what the compiler gonna generate anyway */
  if (!start_sequential)
    goto parallel;
  //copy list.
  memcpy(lyst, lystbck, NUM * sizeof(double));


  //Sequential mergesort, and timing.
  gettimeofday(&start, NULL);
  quicksort(lyst, NUM);
  gettimeofday(&end, NULL);

  if (!isSorted(lyst, NUM)) {
    printf("Oops, lyst did not get sorted by quicksort.\n");
  }
  //Compute time difference.
  diff = ((end.tv_sec * 1000000 + end.tv_usec)
          - (start.tv_sec * 1000000 + start.tv_usec)) / 1000000.0;
  output_result("Sequential", NUM, 1, diff);
parallel:



  //Now, parallel quicksort.

  //copy list.
  memcpy(lyst, lystbck, NUM * sizeof(double));
  int nthreads;

#pragma omp parallel
#pragma omp master
  {
  nthreads = omp_get_num_threads();
  gettimeofday(&start, NULL);
  parallelQuicksortHelper(lyst, 0, NUM - 1);
  gettimeofday(&end, NULL);
  }

  if (!isSorted(lyst, NUM)) {
    printf("Oops, lyst did not get sorted by parallelQuicksort.\n");
  }
  //Compute time difference.
  diff = ((end.tv_sec * 1000000 + end.tv_usec)
          - (start.tv_sec * 1000000 + start.tv_usec)) / 1000000.0;
  output_result("Parallel", NUM, nthreads, diff);


  /*Same here, intentional*/
  if (!start_sequential)
    goto end;
  //Finally, built-in for reference:
  memcpy(lyst, lystbck, NUM * sizeof(double));
  gettimeofday(&start, NULL);
  qsort(lyst, NUM, sizeof(double), compare_doubles);
  gettimeofday(&end, NULL);

  if (!isSorted(lyst, NUM)) {
    printf("Oops, lyst did not get sorted by qsort.\n");
  }
  //Compute time difference.
  diff = ((end.tv_sec * 1000000 + end.tv_usec)
          - (start.tv_sec * 1000000 + start.tv_usec)) / 1000000.0;
  output_result("Builtin", NUM, 1, diff);
end:

  free(lyst);
  free(lystbck);
}

void quicksort(double lyst[], int size)
{
  quicksortHelper(lyst, 0, size - 1);
}

void quicksortHelper(double lyst[], int lo, int hi)
{
  if (lo >= hi)
    return;
  int b = partition(lyst, lo, hi);
  quicksortHelper(lyst, lo, b - 1);
  quicksortHelper(lyst, b + 1, hi);
}

inline int partition(double lyst[], int lo, int hi)
{
  int b = lo;
  /*Get rid of rand, it's too costly compared to function's execution time
   * (*yes* in the worst case scenario we are screwed, but so is the sequential version ;))*/
  /*int r = (int) (lo + (hi - lo) * (1.0 * rand() / RAND_MAX));*/
  int r = hi;
  double pivot = lyst[r];
  double tmp;
  /*swap(lyst, r, hi);*/
  for (int i = lo; i < hi; i++) {
    if (lyst[i] < pivot) {
      /*Manually inline swap, juuuust in case (we *really* don't want a function call here)*/
      tmp = lyst[i];
      lyst[i] = lyst[b];
      lyst[b] = tmp;
      b++;
    }
  }
  tmp = lyst[hi];
  lyst[hi] = lyst[b];
  lyst[b] = tmp;
  return b;
}



/*
parallelQuicksortHelper
*/
void parallelQuicksortHelper(double lyst[], int low, int high)
{
  if (low >= high)
    return ;
  int mid = partition(lyst, low, high);
  /*Create tasks for both sub sections, only if the section is big enough*/
#pragma omp task shared(lyst) firstprivate(low, mid) if(high - low > SIZE_THRESHOLD)
  parallelQuicksortHelper(lyst, low, mid - 1);
  /*One task is enough for this kind of recursive algorithm*/
  parallelQuicksortHelper(lyst, mid + 1, high);
#pragma omp taskwait
  return ;
}

//check if the elements of lyst are in non-decreasing order.
//one is success.
int isSorted(double lyst[], int size)
{
  for (int i = 1; i < size; i++) {
    if (lyst[i] < lyst[i - 1]) {
      printf("at loc %d, %e < %e \n", i, lyst[i], lyst[i - 1]);
      return 0;
    }
  }
  return 1;
}

//for the built-in qsort comparator
//from http://www.gnu.org/software/libc/manual/html_node/Comparison-Functions.html#Comparison-Functions
int compare_doubles(const void *a, const void *b)
{
  const double *da = (const double *) a;
  const double *db = (const double *) b;

  return (*da > *db) - (*da < *db);
}
