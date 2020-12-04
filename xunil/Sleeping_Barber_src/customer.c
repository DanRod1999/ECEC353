/* Skeleton code for the customer program. 
 *
 * Compile as follows: gcc -o customer customer.c -std=c99 -Wall -lpthread -lm 
 *
 * Author: Naga Kandasamy
 * Date: February 7, 2020
 * Student/team: Daniel Rodriguez
 * Date: 2/22/2020
 * 
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#define MIN_TIME 2
#define MAX_TIME 10
#define WAITING_ROOM_SIZE 8

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
    
    sem_t *barber_seat = sem_open (argv[3], 0);
    sem_t *done_with_customer = sem_open (argv[5], 0);
    sem_t *waiting_room = sem_open (argv[2], 0);
    sem_t *barber_bed = sem_open (argv[4], 0);

    printf ("Customer %d: Walking to barber shop\n", customer_number);
    walk (MIN_TIME, MAX_TIME);

    /* FIXME: Get hair cut by barber and go home. */
    int number = customer_number;
   
    /* Simulate going to the barber shop */
    printf ("Customer %d: Arrived at the barber shop \n", number);
    if (sem_trywait(waiting_room) != 0) {
        printf("Waiting room is full, customer %d is leaving\n", customer_number);
        exit(EXIT_SUCCESS);
    }
  

    /* Wait to get into the barber shop */
    
    printf ("Customer %d: Entering waiting room \n", number);
    sem_wait (barber_seat);            /* Wait for the barber to become free */
    sem_post (waiting_room);           /* Let people waiting outside the shop know */
    sem_post (barber_bed);             /* Wake up barber */
    sem_wait (done_with_customer);     /* Wait until hair is cut */
    sem_post (barber_seat);            /* Give up seat */

    printf ("Customer %d: Going home \n", number);    


    printf ("Customer %d: all done\n", customer_number);

    exit (EXIT_SUCCESS);
}
