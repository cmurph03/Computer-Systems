// Caroline Murphy and Brandon Terselic
// September 25, 2023
// This is the slush program for Lab 2


#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

void ctrlCHandler(int sigNum){
        printf("\nControl-C Ignored\n");
}

int main() {
        //Variables for printing directory
        char home[256];
        strcpy(home, getenv("HOME"));
        char currentDir[256];
       //Variables for parsing input
        char buffer[256];
        const char delim[2] = "(";
        const char delim2[2] = " ";
        int max_args = 15;
        int max_argv_size = max_args + 2;
        char* cmd;
        char* my_argv[max_argv_size];
        int command_len = 0;
        char* input_list[256];

    // While loop to create the command line	
   while(1) {
        getcwd(currentDir, sizeof(currentDir));
        printf("slush|%s> ", currentDir);
        signal(2, ctrlCHandler); 
	command_len = 0;

        if(fgets(buffer, sizeof(buffer), stdin) == 0) return 0;
        // strip the newline character from input string
        buffer[strcspn(buffer,"\n")]=0;
        // divide each instruction set into an array
        char* token = strtok(buffer, delim);
        input_list[0] = token;
        
	int i = 1;
        while (token != NULL) {
                token = strtok(NULL, delim);
                input_list[i] = token;
                i++;
                command_len++;
        }

        char* inner_token;
        //Set up pipes
        int pipe1[2];
        int pipe2[2];
       if (pipe(pipe1) == -1) perror("Issue executing pipe");
       if (pipe(pipe2) == -1) perror("Issue executing pipe");
        int pid;
        int pid2;
        int pid3;
        
	 

	for (int i = command_len-1; i>-1; i--) {
            if (input_list[i] == NULL) {
		printf("Invalid command\n");
	   	exit(1);
	   }
	   
	    inner_token = strtok(input_list[i],delim2);
            cmd = inner_token;
            // my_argv[0] = bin command
            my_argv[0] = cmd;
            int j = 1;
            // fill up the my_argv array
            while (inner_token != NULL) {
                inner_token = strtok(NULL, delim2);
                my_argv[j] = inner_token;
                j++;
              }
	 if (command_len == 1) {
	    int checkCD = strcmp(cmd,"cd");	 
    	    if (checkCD == 0){
	    	chdir(my_argv[1]);
		break;
	    }
	}
        int ret;
        // first command
        if (i == command_len-1) {
            pid = fork();
            if (pid == 0) {
                if (command_len > 1) {
                    dup2(pipe1[1],STDOUT_FILENO);
                }
                close(pipe1[0]);
                close(pipe1[1]);
		ret = execvp(cmd, my_argv);
                if (ret == -1){
		    perror("Error executing first command");
		    exit(1);
		}
            }
	    else if (pid == -1){
		    perror ("Error executing fork");
		    exit(1);
	    }
        }
        // middle commands
        else if (i>0 && i<command_len-1) {
           pid2 = fork();
           if (pid2 == 0) {
                   dup2(pipe1[0],STDIN_FILENO);
                   dup2(pipe2[1],STDOUT_FILENO);
                   close(pipe1[0]);
                   close(pipe1[1]);
                   close(pipe2[0]);
                   close(pipe2[1]);
		   ret = execvp(cmd,my_argv);
		   if (ret == -1) {
	                perror("Error executing middle commands");
			exit(1);
		   }

           }
	   else if (pid2 == -1) { 
		   perror ("Error executing middle fork");
		   exit(1);
	   }
        }
        // last commands
        else if (i == 0) {
            pid3 = fork();
            if (pid3 == 0) {
                    if (command_len > 2) {
                    dup2(pipe2[0],STDIN_FILENO);
                    }
                    else {
                    dup2(pipe1[0],STDIN_FILENO);
                    }
                    close(pipe1[0]);
                    close(pipe1[1]);
                    close(pipe2[0]);
                    close(pipe2[1]);
	            ret = execvp(cmd,my_argv);
                    if (ret == -1) { 
			perror("Error executing final command");
			exit(1);
		    }
            }
	    else if (pid3 == -1){
		    perror ("Error executing final fork");
		    exit(1);
	    }
          }
  
	}
		
	
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            waitpid(pid,NULL,0);
            waitpid(pid2,NULL,0);
            waitpid(pid3,NULL,0); 
   
   }
   	signal(2,ctrlCHandler);
	return 0;
}

