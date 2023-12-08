#include <unistd.h>
#include <sys/types.h>
# include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


// Function to Compress a file
int encode(int file_in, int file_out, int comp_len) {
	int counter=1;
	int file_read=1;
	int file_write=1;
	char current_pattern[comp_len];
	char prev_pattern[comp_len];

//read input file
  file_read = read(file_in, current_pattern, comp_len);
  if (file_read == -1) {
	perror("Read Error");
  	return(-1);
   } 

  // Set initial pattern to avoid comparison errors in the loop
  strcpy(prev_pattern,current_pattern);
  while( file_read > 0) { 
	// Check pattern
	file_read = read(file_in,current_pattern,comp_len);
	if(file_read == -1) {
		perror("Read Error");
		return(-1);
	}	
	if( (strcmp(current_pattern, prev_pattern) == 0) && (counter<255)) { 
		counter++;
	}
	else {
		write(file_out, &counter, 1);
   		file_write = write(file_out, prev_pattern, comp_len);
		if (file_write == -1) {
			perror ("Write Error");
			return(-1);
		}
		
		counter = 1;
	}

    	strcpy(prev_pattern,current_pattern);	
   }
  return(0);
  }



// Function to Decompress a File
int decode (int file_in, int file_out, int comp_len) {
   int file_read=1;
   int file_write=1;
   int repeats=0;
   char pattern[comp_len];
   
// Determine number of repeats for the first part of the  pattern 
   file_read = read(file_in, &repeats, 1);
   if (file_read == -1) {
	perror("Read Error");
  	return(-1);
       }
// Read file and write to output
   while( file_read > 0) {
    
    // Determine the pattern
       file_read = read(file_in,pattern,comp_len);
	if(file_read == -1) {
		perror("Read Error");
		return(-1);
	}
    // Write to the file		
	for(int i=0; i<repeats; i++) {
		file_write = write(file_out, pattern, comp_len);
		if (file_write == -1) {
			perror("Write Error");
			return(-1);
		}
	}	
// Determine number of repeats for the pattern 
       file_read = read(file_in, &repeats, 1);
       if (file_read == -1) {
		perror("Read Error");
  		return(-1);
       }
    }
   return(0);
}
int main(int argc, char* argv[]) {
	// Set Variables
	int input_file, output_file ;
	int comp_len = atoi(argv[3]);
	int mode = atoi(argv[4]);
	
	// Check for command line  error
	if (argc < 5) {
		printf("Too few command line arguments.\n");
		return(-1);
	}
	else if (argc > 5) {
		printf("Too many command line arguments.\n");
		return(-1);
	}
	if (comp_len < 1) {
		printf("Compression length cannot be less than 1.\n");
		return(-1);
	}
	if ((mode >1) || (mode <0)) {
		printf("The mode must be 0 or 1.\n");
		printf("The input was instead %i\n",mode);
		return(-1);
	}
	// open input file
	input_file = open(argv[1], O_RDONLY);
	if (input_file == -1) {
		perror("Input Error");
		return(-1);
	}

	//open output file and set read/write permission
	output_file = open(argv[2], O_CREAT|O_TRUNC|O_WRONLY);
	chmod(argv[2],S_IRWXU);

	  // Compress File
	  if (mode == 0) {
		encode(input_file, output_file, comp_len);
	  }
	  // Decompress File
	  else if (mode == 1) {
		  decode(input_file, output_file, comp_len);
	  }
	close(input_file);
	close(output_file);
	return 0;
}
