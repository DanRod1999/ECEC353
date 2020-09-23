/* Program to launch the barber and customer threads. 
 *
 * Compile as follows: gcc -o launcher launcher.c -std=c99 -Wall -lpthread -lm
 *
 * Run as follows: ./launcher 
 *
 * Author: Naga Kandasamy
 * Date: February 7, 2020
 *
 * Student/team: FIXME
 * Date: FIXME
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MIN_CUSTOMERS 10
#define MAX_CUSTOMERS 20
#define WAITING_ROOM_SIZE 8

/* Returns a random integer value, uniformly distributed between [min, max] */ 
int
rand_int (int min, int max)
{
    return ((int) floor ((double) (min + ((max - min + 1) * ((float) rand ()/(float) RAND_MAX)))));
}
    
int 
main (int argc, char **argv)
{
    int i, pid, num_customers;
    char arg[20];

    /* FIXME: create the needed named semaphores and initialize them to the correct values */
    
    /* Create the barber process. */
    printf ("Launcher: creating barber proccess\n"); 
    pid = fork ();
    switch (pid) {
        case -1:
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0: /* Child process */
            /* Execute the barber process, passing the waiting room size as the argument. 
             * FIXME: Also, pass the names of the ncessary semaphores as additional command-line 
             * arguments. 
             */
            snprintf (arg, sizeof (arg), "%d", WAITING_ROOM_SIZE);
            execlp ("./barber", "barber", arg, (char *) NULL);
            perror ("exec"); /* If exec succeeds, we should get here */
            exit (EXIT_FAILURE);
        
        default:
            break;
    }

    /* Create the customer processes */
    srand (time (NULL));
    num_customers = rand_int (MIN_CUSTOMERS, MAX_CUSTOMERS);
    printf ("Launcher: creating %d customers\n", num_customers);

    for (i = 0; i < num_customers; i++){
        pid = fork ();
        switch (pid) {
            case -1:
                perror ("fork");
                exit (EXIT_FAILURE);

            case 0: /* Child code */
                /* Pass the customer number as the first argument.
                 * FIXME: pass the names of the necessary semaphores as additional 
                 * command-line arguments.
                 */
                snprintf (arg, sizeof (arg), "%d", i);
                execlp ("./customer", "customer", arg, (char *) NULL);
                perror ("exec");
                exit (EXIT_FAILURE);

            default:
                break;
        }
    }

    /* Wait for the barber and customer processes to finish */
    for (i = 0; i < (num_customers + 1); i++)
        wait (NULL);

    /* FIXME: Unlink all the semaphores */

    exit (EXIT_SUCCESS);
}
