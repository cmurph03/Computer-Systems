# Computer-Systems
The labs assigned for my computer systems class.

Lab 1: File Compression
  Learning Objectives:
    1. Use the open(), close(), read(), and write() system calls to do file I/O
    2. Implement a simple lossless data compression method called run-length encoding.
    3. Perform proper Linux/Unix style error checking on all functions that may return an error 

  File: rle.c

Lab 2: Shell
  Learning Objectives:
    1. Spawn new processes with the fork() and execvp() system calls
    2.Wait for child processes with the wait() system call
    3. Perform complex input parsing in C
    4. Pipe data between processes
    5. Perform basic signal handling 

  Notes:
   Program execution commands have the form: prog_n [args] ( ... prog_3 [args] ( prog_2 [args] ( prog_1 [args]
    
  File: slush.c


Lab 3: Brute Force Password Cracking
  Learning Objectives:
    1. Use crypt() and crypt_r() to guess password hashes
    2. Iterate over aribtrary length strings with characters from 'a' to 'z'
    3. Create and wait for threads with pthread_create() and pthread_join()
    4. Divide work between multiple parallel threads 
    
  File: crack.c

Lab 4: Malloc
  Learning Objectives:
    1. Write your own memory allocation routines - malloc() , free(), calloc(), and realloc()
    2. Use the sbrk() system call to request memory from the operating system
    3. Track allocated chunks of memory with a linked list
    4. Compile and link other programs to your own allocator 

  Notes: 
     Requirements include that all calls to sbrk() are for the page size rather than just the size needed for malloc.

  Files: malloc.c, churn.c and churn2.c for testing

  

    
