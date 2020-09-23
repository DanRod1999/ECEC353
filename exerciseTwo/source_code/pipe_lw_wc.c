/* This program shows how to use pipes to connect filters between 
 * multiple processes. 
 *
 * The program implements a filter between the ls and wc commands: ls | wc
 *
 * The code shows how a process's standard output, which in this example is 
 * ls, is bound to the write end of the pipe, and the other process's standard 
 * input is bound to the read end of the pipe.
 *
 * The end result is that the output from ls is directed into the pipe and the standard 
 * input of wc is taken from the read end of the pipe.
 *
 * Compile as follows: gcc -o pipe_lw_wc pipe_ls_wc.c -std=c99 -Wall
 *
 * Source: M. Kerrisk, The Linux Programming Interface. 
 *
 * Author: Naga Kandasamy
 * Date created: July 4, 2018
 * Date modified: January 18, 2020
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


int 
main (int argc, char **argv)
{
    int pfd[2];

    if (pipe (pfd) ==  -1) {
        perror ("pipe");
        exit (EXIT_FAILURE);
    }

    switch (fork ()) {
        case -1:
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0:                 /* Child code */
            if (close (pfd[0]) == -1) {
                perror ("close 1");
                exit (EXIT_FAILURE);
            }

            /* Duplicate stdout on write end of pipe; close the duplicated 
             * descriptor. */
            if (pfd[1] != STDOUT_FILENO) {
                if (dup2 (pfd[1], STDOUT_FILENO) == -1) {
                    perror ("dup2 1");
                    exit (EXIT_FAILURE);
                }
            
                if (close (pfd[1]) == -1) {
                    perror ("close 2");
                    exit (EXIT_FAILURE);
                }
            }
            
            /* Execute the first command. Write to pipe. */
            execl ("./gen_numbers", "./gen_numbers", argv[1], (char *) NULL);
            exit (EXIT_FAILURE);    /* Should not get here unless execlp returns an error */

        default:
            break; /* Parent falls through to create the next child */
    }

    switch (fork ()) {
        case -1:
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0:                 /* Child code */
            if (close (pfd[1]) == -1) {
                perror ("close 3");
                exit (EXIT_FAILURE);
            }

            /* Duplicate stdin on read end of pipe; close the duplicated 
             * descriptor. */
            if (pfd[0] != STDIN_FILENO){
                if (dup2 (pfd[0], STDIN_FILENO) == -1) {
                    perror ("dup2 2");
                    exit (EXIT_FAILURE);
                }

                if (close (pfd[0]) == -1){
                    perror ("close 4");
                    exit (EXIT_FAILURE);
                }
            }
                
            /* Execute the second command. Reads from pipe. */
            execl ("./filter_pos_nums", "./filter_pos_nums", (char *) NULL);
            exit (EXIT_FAILURE);

        default:
            break; /* Parent falls through. */
    }


    /* Parent closes unused file descriptors for pipe and waits for the 
     * children to terminate. */
    if (close (pfd[0]) == -1) {
        perror ("close 5");
        exit (EXIT_FAILURE);
    }
    
    if (close (pfd[1]) == -1) {
        perror ("close 6");
        exit (EXIT_FAILURE);
    }

    if (wait (NULL) == -1) {
        perror ("wait 1");
        exit (EXIT_FAILURE);
    }

    if (wait (NULL) == -1) {
        perror ("wait 2");
        exit (EXIT_FAILURE);
    }
    
    exit (EXIT_SUCCESS);
}