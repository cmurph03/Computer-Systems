/* Caroline Murphy
 * caroline.murphy.1@health.slu.edu
 * CSCI2510
 * Lab 4
 *
 * The purpose of this lab is to code a version of malloc, free, calloc, and realloc from scratch
 */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

// A macro to word align the size
#define ALIGN(x) (((x) + (8-1)) & ~(8-1))

// A macro to determine the page size of the current system
#define PAGE_SIZE sysconf(_SC_PAGESIZE)


struct node {
	int block_size;
	int free;
	struct node* next;
};
struct node *head; 

// A function to align a given size to a multiple of the page size for when more memory than a signle page is needed
size_t page_align(size_t size) {
	return (size+PAGE_SIZE-1) & ~(PAGE_SIZE-1);
}

// first free block policy for memory allocation
struct node *find_free_block(size_t size) {
	struct node *current = head;
	// loop through nodes to fine one that is free and of the correct size
	while (current != NULL) {
	    if(current->free && current->block_size > (size + sizeof(struct node))) {
	    	    return (void*)current;
	    }
	    current = current->next;
	}	
	return NULL;
}

// Find the last node in the linked list when a new page needs to be added
struct node *find_last_node() {
	struct node *current = head;
	struct node *next;
	while(current){
		next = current->next;
		if(next == NULL) {
			return (void*)current;
		}
		current = current->next;
	}
	return NULL;

}

// Used for troubleshooting the code along with gdb
int print_list() {
	struct node *current = head;
	struct node *next;
	while(current){
		next = current->next;
		if(next == NULL) {
		printf("Node %p of set size %i with free %i and next %p\n",(void*)(current), current->block_size, current->free, (void*)current->next);
			break;
		}
		printf("Node %p of set size %i with actual size %i free %i and next %p\n",(void*)(current), current->block_size, (void*)current->next - (void*)(current), current->free, (void*)current->next);
		if (((void*)current->next - (void*)(current+1)) < 1) break;
		current = current->next;
	}
	return 0;
}

void *malloc(size_t size) {
	
	// Instantiate variables
	size_t adj_size = ALIGN(size +sizeof(struct node));
	void *block;
	struct node *next_node;
	struct node *cur_node;
	int free_space;

	//printf("Allocating memory for size %i\n", adj_size);
	//print_list();
	if(size<=0) {
		return NULL;
	}
	// Create the initial linked list
	if((void*) head == NULL) {
	    //printf("allocating pages: %d\n", page_align(adj_size) );
	    block = sbrk(page_align(adj_size));
     	    if (block == (void*) -1) {
			errno = ENOMEM;
			return NULL;
		}
	    cur_node = block;
	    free_space = page_align(adj_size) - adj_size - sizeof(struct node);
	    cur_node->block_size = adj_size;
	    cur_node->free = 0;
	    next_node = (void*)cur_node + adj_size;
	    next_node->block_size = free_space;
	    next_node->free = 1;
	    next_node->next = NULL;
	    cur_node->next = next_node;
	    head = cur_node;
	    //printf("The returned node is %p size %i and next %p then %p\n", (void*)(cur_node+1), cur_node->block_size, (void*)cur_node->next,(void*)next_node->next);
	    return (void*)(cur_node +1);

	}



	// Search for a free/available block of memory
	cur_node = find_free_block(adj_size);
	// if a free space is found
	if(cur_node) {
	    //printf("Found a free block of memory\n");
	    //printf("%p of size %i with %i\n", (void*)cur_node, cur_node->block_size, cur_node->free);
	    free_space = cur_node->block_size - adj_size;

	    if( free_space >= 2*sizeof(struct node)){

	 	cur_node->block_size = adj_size;
		cur_node->free = 0;
		next_node = (void*)cur_node + adj_size;
		next_node->block_size = free_space;
		next_node->free = 1;
		
		next_node->next = cur_node->next;
		cur_node->next = next_node;

	    } else {

		cur_node->free = 0;
		//printf("Not large enough to split");
   	    }


	   //printf("The returned node is %p size %i and next %p then %p\n", (void*)(cur_node+1), cur_node->block_size, (void*)cur_node->next,(void*)next_node->next);
	    	return (void*)(cur_node +1);

	}
	//printf("Need to allocate more memory\n");
	block = sbrk(page_align(adj_size));
	if (block == (void*) -1) {
		errno = ENOMEM;
		return NULL;
	}
	cur_node = find_last_node();
	struct node *new_block = block;
	cur_node->next = new_block;
	free_space = page_align(adj_size) - adj_size - sizeof(struct node);
	new_block->block_size = adj_size;
	new_block->free = 0;
	
	next_node = (void*)new_block + adj_size;
	next_node->block_size = free_space;
	next_node->free = 1;
	next_node->next = NULL;
	new_block->next = next_node;
	    //printf("The returned node is %p size %i and next %p of size %i then %p\n", (void*)(new_block+1), new_block->block_size, (void*)next_node,next_node->block_size, (void*)next_node->next);
	return (void*)(new_block +1);
}

// Used in the free function to combined neighboring blocks of available memory into one node
void merge() {
	struct node *curr, *prev;
	curr = head;
	while ( curr != NULL && curr->next != NULL) {
		if((curr->free) && (curr->next->free)) {
			curr->block_size += (curr->next->block_size);
			curr->next = curr->next->next;
		}
		else {
		    	prev = curr;
			curr = curr->next;
		}
	}
}

void free(void *ptr) {
	if(!ptr) return;
	struct node *cur_node = (struct node*)ptr-1;
	cur_node->free = 1;
	merge();
}

void *realloc(void *ptr, size_t size) {
	if (ptr == NULL) {
		return malloc(size);
	}

	struct node *cur_node = (struct node*)ptr -1;
	int copy_size;
	void *new_ptr = malloc(size);
	if (new_ptr == NULL) {
		return NULL;
	}
	if (cur_node->block_size < size){
		// resize the space
		copy_size = cur_node->block_size - sizeof(struct node);
	}
	else {
	    copy_size = size;
	}

	memcpy(new_ptr, ptr, copy_size);
	free(ptr);
	return new_ptr;
}

int check_overflow(int a, int b) {
	if (a > 0 && b > 0 && (a*b > INT_MAX)) {
		return -1;
	}
	if (a < 0 && b < 0 && (a*b > INT_MAX)) {
		return -1;
	}
	return 0;
}

void *calloc(size_t num, size_t size) {
	if (check_overflow(num,size) == -1) {
		errno = ENOMEM;
		return NULL;
	}
	size_t space = num*size;
	if (space == 0) return NULL;
	void *ptr = malloc(space);
	memset(ptr, 0, space);
	return ptr;
}
/*
int main()
{
 
    int NUMSLOTS = 40;
    int maxblock = 1000;
    char *slots[NUMSLOTS];
    size_t sizes[NUMSLOTS];
    int i, n;
  // Initialize
  for (i = 0; i<NUMSLOTS; i++) slots[i] = NULL;
  srand(time(NULL));

  int count = 0;
 while(count<50){
    n = rand() % NUMSLOTS;
    sizes[n] = rand() % maxblock + 1;
    printf("Node list before calling malloc: \n");
    print_list();
    slots[n] = (char *) mymalloc (sizes[n]);
    printf("Called malloc on size %i with return %p\n", sizes[n],(void*)slots[n]);
    printf("Node list after calling malloc:\n");
    print_list();

    for (int j=0; j<sizes[n]; j++) {
	      *(slots[n]+j) = (char) j;
	
      }
    count++;
    printf("Memory filled, count: %d\n", count);
 }
*/

/*
    // This pointer will hold the
    // base address of the block created
    int *ptr, *ptr1, *ptr2, *ptr3, *ptr4, *ptr5, *ptr6,*ptr7;
    int n, i;
 
    ptr = mymalloc(1000);
    if (ptr == NULL) printf("Memory for pointer 0 not allocated\n");
    ptr1 = mymalloc(500);
    if (ptr1 == NULL) printf("Memory for pointer 1 not allocated\n");
    ptr2 = mymalloc(200);
    if (ptr2 == NULL) printf("Memory for pointer 2 not allocated\n");
    ptr3 = mymalloc(300);
    if (ptr3 == NULL) printf("Memory for pointer 3 not allocated\n");
    ptr4 = mymalloc(700);
    if (ptr4 == NULL) printf("Memory for pointer 4 not allocated\n");
    ptr5 = mymalloc(6000);
    if (ptr5 == NULL) printf("Memory for pointer 5 not allocated\n");
    ptr6 = mymalloc(1100);
    if (ptr6 == NULL) printf("Memory for pointer 6 not allocated\n");
    ptr7 = mymalloc(1500);
    if (ptr7 == NULL) printf("Memory for pointer 7 not allocated\n");

   
     printf("Before calling free:\n");
     print_list();
     myfree(ptr2);

    printf("The current linked list after calling free is:\n");
    print_list();
    ptr2 = mymalloc(100); 
    if (ptr2 == NULL) printf("Memory for pointer 2 not allocated\n");
    printf("The freed and reallocated memory for pointer 2 is %p\n",(void*)ptr2);
   */ 
//    return 0;
//}


