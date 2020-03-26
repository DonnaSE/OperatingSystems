/*
 Name: Raghad Safauldeen
 ID: 1001417235
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments



// function to handle the signal (handle Ctrl-c and Ctrl-z) and return nothing
static void handle_signal (int sig )
{
 // printf ("Caught signal %d\n", sig );
}

int p=1; // number of pids
struct sigaction act;
int main(int argc, char *argv[])
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
 
  //  Zero out the sigaction struct  
  memset (&act, '\0', sizeof(act));
  // Install the handler and check the return value.
  // Set the handler to use the function handle_signal()  
  act.sa_handler = &handle_signal;

  // SIGTSTP handle ctrl-z  
  // SIGINT  handle ctrl-c
  if((sigaction(SIGTSTP, &act, NULL)<0) ||(sigaction(SIGINT , &act, NULL)<0))			 
  {
    perror ("sigaction: ");
	return 1;
  }
  // make 2D array for the history
  char tokenArrayHistory[MAX_NUM_ARGUMENTS][255];
  int i;
  int listPid[15];
   
  for( i = 0; i < MAX_NUM_ARGUMENTS; i++ )
  memset( tokenArrayHistory[i], 0, 255 );
	
	  while( 1 )
	  {
		// Print out the msh prompt
		printf ("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

        if(strcmp(cmd_str, " ")==0) 
            continue;
        
		/* Parse input */
		char *token[MAX_NUM_ARGUMENTS];

		int   token_count = 0;                                 
															   
		// Pointer to point to the token
		// parsed by strsep
		char *arg_ptr;                                         
															   
		char *working_str  = strdup( cmd_str );                

		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;

		// Tokenize the input stringswith whitespace used as the delimiter
		while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
				  (token_count<MAX_NUM_ARGUMENTS))
		{
		  token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
		  if( strlen( token[token_count] ) == 0 )
		  {
			token[token_count] = NULL;
		  }
			token_count++;
		}

		// Now print the tokenized input as a debug check
		// \TODO Remove this code and replace with your shell functionality

		int token_index  = 0;
		for( token_index = 0; token_index < token_count; token_index ++ ) 
		{
		 // printf("token[%d] = %s\n", token_index, token[token_index] );  
		}

		free( working_root );	  
	  
		// if token is empty do nothing 
		 if(token[0]==NULL)
		   continue;
	  
		// compare the token with exit and quit if so, exit the program
		else if((strcmp(token[0], "exit")==0)||(strcmp(token[0], "quit")==0))
			exit(EXIT_SUCCESS);
		
		else if( strcmp( token[0], "listpids" ) == 0 )
		{
			int j;
			for (j=0; j< 15;j++)
			{
				if( listPid[j] != 0 )
				printf("%d %d \n", j, listPid[j]);  
			}   
		}
		else if(token[0] != NULL)
		{		
			static int j = 0;
			 // save token[0] in an array
			 
			 if ((char)token[0][0] == '!' )
		     {
				int index = atoi((char*)(token[0]+1));
		        strncpy( token[0], (tokenArrayHistory[index]), 
		                 strlen( (tokenArrayHistory[index]) )-1);
		    }   
			else 
			{
				strcpy( tokenArrayHistory[j++], cmd_str );
			    tokenArrayHistory[j-1][strlen(tokenArrayHistory[j-1])] = '\0';			     
			}
			
			if( j >= MAX_NUM_ARGUMENTS ) j = 0;
			
			 if( strcmp( token[0], "history" ) == 0 )
			 {
				 int k=0;
				 
			     // print the history array	
	  			 for (k=0; k< j; k++)
	     		 {
				    printf("%d: %s", k, tokenArrayHistory[k]);
		    	 }
		    	 
		    	 continue;
		    	 
			 }
			 
			 if( strcmp( token[0], "cd" ) == 0 )
			 {
				 chdir( token[1] );
				 continue;
			 }
			 // suspend the process with bg
			 if(strcmp(token[0],"bg")==0)
			 {
				 continue;
			 }

			 
			 // make fork(0) 
			 pid_t pid = fork();
			 
			  // get the pid number and save it in array			    
			 static int p=0;
			 if(p<15)
			 {
			   listPid[p] = pid;
			   p++;
			 }
			 else 
			 {
				 p = 0;
			 }
 
		
			 int status;
			 // first is the child
			 if(pid == 0)
			{
				  char *PATH[] = {"/usr/local/bin","/usr/bin","/bin"};
				   // loop to check the path
				   int i;
				   char command[MAX_COMMAND_SIZE]; // the max command size is 255
				   int size = sizeof(PATH)/sizeof(PATH[0]);
				   
				   // for loop to make a path 
				   for(i=0; i<size; i++)
				   {
					   strcpy(command, PATH[i]);
					   strcat(command, "/");
					   strcat(command, token[0]);
					   execl(command,token[0], token[1],token[2],token[3], 
					   token[4],token[5],token[6],token[7],token[8],
					   token[9],token[10], NULL);
				   }
				
				  // compare the token with cd and if it is not equels,
				  // print the phrase below
				  if (strcmp(token[0],"cd")!=0) 
				  {
					  printf("%s: Command not found.\n\n", token[0]); 
					  fflush(NULL);
					  exit(EXIT_SUCCESS);
				  }
			}  
		   else if (pid > 0) // parent
		   {
				 // When fork() returns a positive number, we are in the parent
				 // process and the return value is the PID of the newly created
				 // child process.

			
				// Force the parent process to wait until the child process exits
				waitpid(pid, &status,0);
				fflush( NULL );
				
				// if the user input was only cd then the path will 
				// go up toward the root
				if (strcmp(token[0], "cd")==0 && !(token[1]))
					chdir(".."); 
				 // if the user input was cd then directory or folder 
				 // then it has to open the directory by chdir
				else if (strcmp(token[0], "cd")==0 && (token[1])) 
					chdir(token[1]);    
		   }
				   
		   else if( pid == -1 )
		   {
				// When fork() returns -1, an error happened.
				perror("fork failed: ");
				exit( EXIT_FAILURE );
		   }  
	    }
	 } 
    return 0;
}
