/* Main program that uses pipes to connect the three filters as follows:
 *
 * ./gen_numbers n | ./filter_pos_nums | ./calc_avg
 * 
 * Author: Daniel Rodriguez 
 * Date created: 2/1/2020
 *
 * Compile as follows: 
 * gcc -o pipes_gen_filter_avg pipes_gen_filter_avg.c -std=c99 -Wall -lm
 * */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


int main(int argc, char **argv)
{
    int pfd_one[2];
    int pfd_two[2];

    /* Create both pipes */
    if(pipe (pfd_one) ==  -1 || pipe(pfd_two) == -1) {
        perror ("pipe");
        exit (EXIT_FAILURE);
    }

    switch(fork()) {
        case -1:
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0:                 /* Child code one */
            /*close read end to both pipes*/
            if(close (pfd_one[0]) == -1 || close(pfd_two[0]) == -1){
                perror ("close 1");
                exit (EXIT_FAILURE);
            }

            /* Duplicate stdout on write end of pipe; close the duplicated 
             * descriptor. */
            if(pfd_one[1] != STDOUT_FILENO) {
                if(dup2 (pfd_one[1], STDOUT_FILENO) == -1) {
                    perror ("dup2 1");
                    exit (EXIT_FAILURE);
                }
            
                /* Close write to both pipes */
                if(close (pfd_one[1]) == -1 || close (pfd_two[1]) == -1) {
                    perror ("close 2");
                    exit (EXIT_FAILURE);
                }
            }
            
            /* Execute the first command. Write to pipe. */
            execl ("./gen_numbers", "./gen_numbers", argv[1], (char *) NULL);
            exit(EXIT_FAILURE);    /* Should not get here unless execlp returns an error */

        default:
            break; /* Parent falls through to create the next child */
    }

    switch(fork()) {
        case -1:
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0:                 /* Child code */
            /* Close write for pipe one and read for pipe two*/
            if(close(pfd_one[1]) == -1 || close(pfd_two[0]) == -1) {
                perror ("close 3");
                exit (EXIT_FAILURE);
            }

            

            /* Duplicate stdin on read end of pipe; close the duplicated 
             * descriptor. */
            if(pfd_one[0] != STDIN_FILENO){
                if(dup2 (pfd_one[0], STDIN_FILENO) == -1) {
                    perror ("dup2 2");
                    exit (EXIT_FAILURE);
                }

                if (close (pfd_one[0]) == -1){
                    perror ("close 4");
                    exit (EXIT_FAILURE);
                }
            }

            /* Send stdout to second pipe write end so third process can read */
            if(pfd_two[1] != STDOUT_FILENO){
                if(dup2 (pfd_two[1], STDOUT_FILENO) == -1) {
                    perror ("dup2 2");
                    exit (EXIT_FAILURE);
                }

                if(close(pfd_two[1]) == -1){
                    perror ("close 5");
                    exit (EXIT_FAILURE);
                }

            }
                
            /* Second command read from pipe */
            execl ("./filter_pos_nums", "./filter_pos_nums", (char *) NULL);
            exit(EXIT_FAILURE);

        default:
            break; /* Parent falls through. */
    }

    switch (fork ()) {
        case -1:
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0:                 /* Child code */
            /*Close write end on both pipes*/
            if(close (pfd_two[1]) == -1 || close(pfd_one[1]) == -1) {
                perror ("close 6");
                exit (EXIT_FAILURE);
            }

            /* Duplicate stdin on read end of pipe; close the duplicated 
             * descriptor. */
            if(pfd_two[0] != STDIN_FILENO){
                if (dup2 (pfd_two[0], STDIN_FILENO) == -1) {
                    perror ("dup2 2");
                    exit (EXIT_FAILURE);
                }

                if(close (pfd_two[0]) == -1 || close(pfd_one[0]) == -1){
                    perror ("close 7");
                    exit (EXIT_FAILURE);
                }
            }
                
            /* Read third command from second pipe */
            execl ("./calc_avg", "./calc_avg", (char *) NULL);
            exit(EXIT_FAILURE);

        default:
            break; /* Parent falls through. */
    }


    /* close all pipes */
    if(close (pfd_one[0]) == -1 || close(pfd_two[0]) == -1) {
        perror ("close 8");
        exit (EXIT_FAILURE);
    }
    
    if(close (pfd_one[1]) == -1 || close(pfd_two[1]) == -1) {
        perror ("close 9");
        exit (EXIT_FAILURE);
    }

    /*All waits for the three processes*/
    if(wait (NULL) == -1) {
        perror ("wait 1");
        exit (EXIT_FAILURE);
    }

    if(wait (NULL) == -1) {
        perror ("wait 2");
        exit (EXIT_FAILURE);
    }

    if(wait (NULL) == -1) {
        perror ("wait 3");
        exit (EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}