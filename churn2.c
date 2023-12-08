/*
 * churn2.c
 *    usage: churn2 [-d] maxblock
 *
 *    Test malloc(), calloc(), realloc() and free() by
 *    repeatedly alloc/freeing NUMSLOTS blocks of memory
 *    of random size between 1 and maxblock.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define NUMSLOTS 32
#define CHURNS 20

long *slots[NUMSLOTS];
int sizes[NUMSLOTS];

main(int argc, char *argv[])
{
  int i,j,n,size,maxblock;
  int c=0, f=0, r=0;
  int d=0;
  int cc=0;

  maxblock = -1;
  if (argc == 3) {
    if (!strcmp(argv[1],"-d")) {
      maxblock = atoi(argv[2]);
      d = 1;
    }
  }
  if (argc == 2)
    maxblock = atoi(argv[1]);

  if (maxblock <= 0) {
    fprintf(stderr,"usage: churn2 [-d] maxblock\n");
    exit(1);
  }

  for (i = 0; i<NUMSLOTS; i++) slots[i] = NULL;
  srand(time(NULL));
  
  printf("calloc\trealloc\tfree\tsbrk\n");

  while (cc < CHURNS) {
    for (i = 0; i < NUMSLOTS * NUMSLOTS; i++) {
      n = rand() % NUMSLOTS;

      if (slots[n]) {
	/* Slot is full */
	if (d) printf("Testing slot %d...",n);

	/* Check contents */
	for (j=1; j<sizes[n]; j++) {
	  if (*(slots[n]+j) != j) {
	    printf("Memory error detected.\n");
	    exit(1);
	  }
	}
	
	if (d) printf("ok\n");

	/* Free or realloc at random */
	if (rand() % 2) {
	  /* Free it */
	  if (d) printf("Freeing slot %d.\n",n);
	  free(slots[n]);
	  slots[n] = NULL;
	  f++;

	} else {
	  /* Reallocate it */
	  if (d) printf("Realloc slot %d...",n);
	  size = rand() % maxblock + 1;
	  slots[n] = (long *) realloc(slots[n],size*sizeof(long));
	  if (slots[n] == NULL) {
	    fprintf(stderr,"realloc: out of memory\n");
	    exit(1);
	  }
	  if (sizes[n] < size) {
	    if (d) printf("larger.\n");
	    for (j = sizes[n]; j<size; j++) {
	      *(slots[n]+j) = j;
	    }
	  } else {
	    if (d) printf("smaller.\n");
	  }
	  sizes[n] = size;

	  r++;

	}

      } else {
	/* Slot is empty */
	if (d) printf("Calloc  slot %d.\n",n);

	sizes[n] = rand() % maxblock + 1;
	slots[n] = (long *) calloc (sizes[n],sizeof(long));

	if (slots[n] == NULL) {
	  fprintf(stderr,"calloc: out of memory\n");
	  exit(1);
	}

	/* Fill block with 1,2,3,4,... */
	for (j=0; j<sizes[n]; j++) {
	  if (*(slots[n]+j)) {
	    printf("Calloc failed to zero memory\n");
	    exit(1);
	  }
	  *(slots[n]+j) = j;
	}

	c++;
      }

    }
    printf("%d\t%d\t%d\t%p\n",c,r,f,sbrk(0));
    c = 0; f = 0; r=0;
    cc++;
  }
}

