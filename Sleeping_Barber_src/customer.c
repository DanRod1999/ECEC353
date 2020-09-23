/* Skeleton code for the customer program. 
 *
 * Compile as follows: gcc -o customer customer.c -std=c99 -Wall -lpthread -lm 
 *
 * Author: Naga Kandasamy
 * Date: February 7, 2020
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>

#define MIN_TIME 2
#define MAX_TIME 10

/* Simulate walking to barber shop, by sleeping for some time between [min. max] in seconds */
void
walk (int min, int max) 
{
    sleep ((int) floor((double) (min + ((max - min + 1) * ((float) rand ()/(float) RAND_MAX)))));
    return;
}

int 
main (int argc, char **argv)
{
    srand (time (NULL)); 

    int customer_number = atoi (argv[1]);
    /* FIXME: unpack the semaphore names from the rest of the arguments */
    
    printf ("Customer %d: Walking to barber shop\n", customer_number);
    walk (MIN_TIME, MAX_TIME);

    /* FIXME: Get hair cut by barber and go home. */


    printf ("Customer %d: all done\n", customer_number);

    exit (EXIT_SUCCESS);
}
