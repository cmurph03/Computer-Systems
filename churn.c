/*
 * churn.c
 *
 * usage: churn [-v] maxblock\n");
 * 
 *    Allocate & free memory
 *
 *    Tests malloc() and free() by repeatedly allocating and freeing
 *    chunks of memory of up to size maxblock.
 *
 *    -v is verbose mode, and prints a message with each call to malloc/free
 *
 * Bryan Clair 2002-2013
 * v2.0 2010: churn now actually writes data into the memory it's allocated, and checks
 *            that the data survives.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// At any time, churn may have up to NUMSLOTS memory allocations
#define NUMSLOTS 40

char *slots[NUMSLOTS];
size_t sizes[NUMSLOTS];

main(int argc, char *argv[])
{
  int i,n,size,maxblock;
  long j;
  int verbose=0;

  // Read arguments
  maxblock = -1;
  if (argc == 3) {
    if (!strcmp(argv[1],"-d")) {
      maxblock = atoi(argv[2]);
      verbose = 1;
    }
  }
  if (argc == 2)
    maxblock = atoi(argv[1]);

  if (maxblock <= 0) {
    fprintf(stderr,"usage: churn [-d] maxblock\n");
    exit(1);
  }

  // Initialize
  for (i = 0; i<NUMSLOTS; i++) slots[i] = NULL;
  srand(time(NULL));

  long mallocs=0,frees=0;  // number of mallocs, number of frees
  printf("mallocs\tfrees\tsbrk\n");

  // Main loop:
  while (1) {
    n = rand() % NUMSLOTS;
    
    if (slots[n]) {
      // Going to free slot[n]
      // Check contents to see if it's still 0,1,2,3,4,...
      for (j=1; j<sizes[n]; j++) {
	if (*(slots[n]+j) != (char) j) {
	  printf("Memory corruption detected.\n");
	  exit(1);
	}
      }
      
      if (verbose) printf("freeing slot %d.\n",n);
      free(slots[n]);
      slots[n] = NULL;
      frees++;

    } else {
      // Going to malloc slot[n]
      if (verbose) printf("malloc slot %d.\n",n);

      sizes[n] = rand() % maxblock + 1;
      slots[n] = (char *) malloc (sizes[n]);
      if (slots[n] == NULL) {
	fprintf(stderr,"out of memory\n");
	exit(1);
      }
      
      // Fill block with 0,1,2,3,4,...
      for (j=0; j<sizes[n]; j++) *(slots[n]+j) = (char) j;

      mallocs++;
    }
    
    // Periodically report progress
    if ((mallocs + frees) % 1000 == 0) 
      printf("%ld\t%ld\t%p\n",mallocs,frees,sbrk(0));
  }
}

