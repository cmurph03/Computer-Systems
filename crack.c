#include <stdio.h>
#include <crypt.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

/*  This is a program to brute force crack a password hash (target) by iterating over all combinations
 *  of a string of a given length (keysize). The number of threads can also be adjusted to
 *  decrease the time needed to crack the password
 *  The format is ./crack <threads> <keysize> <target> (optional)<extended range>
 *  where the optional extended range is 1 to have a password include numbers, symbols, and capital letters.
 */

struct thread_data {
	char t_target[20];
	char t_pw[8];
	char t_salt[3];
	int t_pw_len;
	char check_end[2];
};

bool found;
int pw_range;
int adjust_start;
int adjust_end;


// Function to iterate over the possible passwords. Does one at a time and starts from 1 to keysize
void* iteratePW(void* param){
	struct thread_data* thread_ptr = (struct thread_data*) param;

	struct crypt_data data;
	// Create and assign local variables to access the struct variables
	char* target;
        char* pw;
        char* salt;
	int pw_len;
	char* hash;
	char* end;

	target = thread_ptr->t_target;
	pw = thread_ptr->t_pw;
	salt = thread_ptr->t_salt;
	pw_len = thread_ptr->t_pw_len;
	end = thread_ptr->check_end;


    // pw[0] set in main func	
    while(pw[0] < end[0]) {
	// increment the rightmost index
        pw[pw_len - 1]++;	
	
	// loop to update next digit and wrap when end_range is reached
	int cur_index = pw_len -1;
	while(cur_index >0 && pw[cur_index] >(122+adjust_end)){
	    pw[cur_index] = 97 - adjust_start;
	    pw[cur_index-1]++;
	    cur_index --;
	    
	}
//	printf("%s\n",pw);
	// check the pw
	hash = crypt_r(pw, salt, &data);
	if (hash == NULL) {
		perror("Crypt() failed");
	}
	if(strcmp(target, hash) == 0) {
		printf("The password is %s\n", pw);
		found = true;
		pw[0] = end[0];
	}

	// Exit the other threads once the password is found
	if(found == true) {
		pw[0] = end[0];
	}
     }
}



int main(int argc, char* argv[]) {
	// Check for command line  error
	if (argc < 4) {
		printf("Too few command line arguments.\n");
		printf("Usage: <thread number> <keysize> <target> (optional)<extended range>\n");
		return(-1);
	}
	else if (argc > 5) {
		printf("Too many command line arguments.\n");
		printf("Usage: <thread number> <keysize> <target> (optional)<extended range>\n");
		return(-1);
	}
	// Set Variables
	char* target = argv[3];
	char salt[3];
	char password[8];
	char hash[20];
	int threads = atoi(argv[1]);
	int keysize = atoi(argv[2]);
	char *pw = calloc(keysize+1, sizeof *pw);
	if (argc == 5) {
		if (atoi(argv[4]) == 1) {
			pw_range = 93;
			adjust_start = 64;
			adjust_end = 4;
			// Set the intial password
			for(int i=0; i<keysize; i++) {
			    pw[i] = '!';
			}		
			pw[keysize+1] = '\0';

		}
		else {
			printf("Set the optional parameter to 1 to allow the extended password set\n");
			return 0;
		}

	}
	else if (argc == 4) {
		pw_range = 26;
		adjust_start = 0;
		adjust_end = 0;
		// Set the intial password
	 	for(int i=0; i<keysize; i++) {
		    pw[i] = 'a';
		}
		pw[keysize+1] = '\0';
	}

	int thread_size = pw_range/threads;
	int thread_mod = pw_range%threads;

	if (keysize > 8) {
		printf("Maximum key size is 8\n");
		return(-1);
	}

	// identify the salt
	for(int i=0; i<2; i++) {
	    salt[i] = target[i];
	}
	salt[3] = '\0';

	// Set up thread structures
	struct thread_data data[threads];
	pthread_t threadID[threads];
        struct thread_data *thread_ptr = data;

	char end[2];
	int swp = 97 - adjust_start;
	found = false;
	// for loop to create the threads
	for(int j=0; j<threads; j++) {	
	   // set the first one to begin_range with an end based on if there is an uneven division
	   if (j == 0 && thread_mod > 0) {
		   swp = 97 - adjust_start+thread_size+1;
		   end[0] = swp;
		   thread_mod--;
	   }
	   else if (j == 0 && thread_mod == 0){
		   swp = 97 - adjust_start + thread_size;
		   end[0] = swp; 

	   
	   }
 	   if (thread_mod > 0 && j>0) {
		   pw[0] = swp;
		   swp += thread_size+1;
		   end[0] = swp;
		   thread_mod--;

	   }
	   else if (thread_mod == 0 && j>0) {
		  pw[0] = swp;
		  swp += thread_size;
		  end[0] = swp;
	   }
	   end[1] = '\0';
   	   // Identify values for threads
           strcpy(thread_ptr->t_target, target);
           strcpy(thread_ptr->t_salt, salt);
           thread_ptr->t_pw_len = keysize;
	   strcpy(thread_ptr->t_pw, pw);
	   strcpy(thread_ptr->check_end, end);
	   // Iterate values and create threads
	   if(pthread_create(&threadID[j],NULL,iteratePW,(void *)thread_ptr) !=0) {
		  perror("Error creating thread");
	     	  return 0;
	       }
	   thread_ptr++;

	   // exit the loop just in case once the password is found
	   if (found) {
		   break;	
	    }	 
	}		
	for(int i=0; i<threads; i++){	
	    if (pthread_join(threadID[i],NULL) != 0) {
		printf("pthread_join() error\n");
   	    }
	 } 		
		
        
	return 0;
}




