/*
* 
*
*
*
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define BUF_LEN 256

int 
main (int argc, char **argv)
{
    int pid;
    /*Pipe 1*/
    int fd[2];    /* Array to hold the read and write file descriptors for the pipe. */
    /*Pipe 2*/
    int fd_2[2];
    int n; 
    char buffer[BUF_LEN];
    int status;

    while(fgets(buffer,BUF_LEN,stdin) !=  NULL){

    
    /* Create the pipe data structure. */
    if (pipe(fd) < 0 || pipe(fd_2) < 0) {
        perror ("pipe");
        exit (EXIT_FAILURE);
    }

    /* Fork the parent process. */
    if ((pid = fork ()) < 0) {
        perror ("fork");
        exit (EXIT_FAILURE);
    }

    /* Parent code */
    if (pid > 0) {
        /* Close the reading end of pipe 1*/
        close(fd[0]);
        /* Close the writing end of pipe 2*/
        close(fd_2[1]);

        // printf ("PARENT: Writing %d bytes to the pipe: \n", (int) strlen (buffer));
        /* Write the buffer contents to the pipe */
        write (fd[1], buffer, strlen (buffer));
    }

    /* Child code */
    else {
        /* Close writing end of pipe 1*/
        close (fd[1]);
        /* Close reading end of pipe 2*/
        close(fd_2[0]);

        n = read (fd[0], buffer, BUF_LEN);                                          /* Read n bytes from the pipe */
        buffer[n] = '\0';                                                           /* Terminate the string */

        /*Convert word to upper case*/
        for(int i = 0; i < strlen(buffer); i++){
            buffer[i] = toupper(buffer[i]);
        }
        write (fd_2[1], buffer, strlen (buffer));
        exit (EXIT_SUCCESS);                                                        /* Child exits */
    }
  
    /* Parent code */
    close(fd_2[1]);//close write to pipe 2
    pid = waitpid (pid, &status, 0);                                                /* Wait for child to terminate */
    n = read (fd_2[0], buffer, BUF_LEN);
    printf("%s\n", buffer);
    // printf ("PARENT: Child has terminated. \n");
    memset(buffer, 0, BUF_LEN);

    }
    exit (EXIT_SUCCESS);
}