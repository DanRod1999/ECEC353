/* This code illustrates how to use semaphores to solve the sleeping barber problem. 
 *
 * Author: Naga Kandasamy
 * Date created: August 27, 2018
 * Date modified: Feruary 9, 2020
 *
 * Compile as follows: gcc -o sleeping_barber sleeping_barber.c -std=c99 -Wall -lpthread -lm
 *
 * */

#define _REENTRANT
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define MIN_TIME 4
#define MAX_TIME 8

#define MAX_NUM_CUSTOMERS 50

void *customer (void *num); /* Prototype for customer thread */
void *barber (void *); /* Prototype of barber thread */
int UD (int, int); /* Random number generator */


/* Definition of semaphores */
sem_t waiting_room;             /* Signal that the waiting room can accommodate  customers */
sem_t barber_seat;              /* Signal to ensure exclusive access to the barber seat */
sem_t done_with_customer;       /* Signals the customer that the barber is done with him/her */
sem_t barber_bed;               /* Signal to wake up the barber */

int done_with_all_customers = FALSE; /* Flag indicating the barber can go home */

int 
main (int argc, char **argv)
{
    if (argc < 3) {
        printf ("Usage: %s num-customers num-chairs \n", argv[0]);
        exit (EXIT_FAILURE);
    }

    int num_customers = atoi (argv[1]); /* Number of customers */
    int num_waiting_chairs = atoi (argv[2]); /* Number of waiting chairs in the barber shop */
  
    srand((long)time(NULL)); /* Initialize randomizer */
    if (num_customers > MAX_NUM_CUSTOMERS) {
        printf ("Number of customers exceeds the maximum capacity of the barber \n");
        printf ("Resetting the number of customers to %d \n", (int) MAX_NUM_CUSTOMERS);
        num_customers = MAX_NUM_CUSTOMERS;
    }
  
    /* Initialize the semaphores */
    sem_init (&waiting_room, 0, num_waiting_chairs);
    sem_init (&barber_seat, 0, 1);
    sem_init (&done_with_customer, 0, 0);
    sem_init (&barber_bed, 0, 0);

    pthread_t btid; /* ID for the barber thread */
    pthread_t tid[MAX_NUM_CUSTOMERS]; /* IDs for customer threads */

    /* Create barber thread */
    pthread_create (&btid, 0, barber, 0); 
  
    /* Create customer threads and give each an ID */
    int customerID[MAX_NUM_CUSTOMERS];
    int i;
    for (i = 0; i < num_customers; i++) {
        customerID[i] = i;
        pthread_create (&tid[i], 0, customer, &customerID[i]);
    }
  
    for (i = 0; i < num_customers; i++)
        pthread_join (tid[i], 0);

    done_with_all_customers = TRUE;
    sem_post (&barber_bed); /* Wake up barber */
  
    pthread_join (btid, 0);

    exit (EXIT_SUCCESS); /* That's all folks */
}

void *
barber (void *junk) 
{
    while (!done_with_all_customers) { /* Customers remain to be serviced */
        printf ("Barber: Sleeping \n");
        sem_wait (&barber_bed);

        if (!done_with_all_customers) {
            printf ("Barber: Cutting hair \n");
            
            int wait_time = UD (MIN_TIME, MAX_TIME); /* Simulate cutting hair */
            sleep (wait_time);
      
            sem_post (&done_with_customer); /* Indicate that chair is free */
    
        }
        else 
            printf ("Barber: Going home \n");
    }

    pthread_exit (NULL);
}

void *
customer (void *customerNumber)
{
    int number = *(int *) customerNumber;
   
    /* Simulate going to the barber shop */
    printf ("Customer %d: Leaving for the barber shop \n", number);
    int wait_time = UD (MIN_TIME, MAX_TIME);     
    sleep (wait_time);
  
    printf ("Customer %d: Arrived at the barber shop \n", number);
    sem_wait (&waiting_room);           /* Wait to get into the barber shop */
    
    printf ("Customer %d: Entering waiting room \n", number);
    sem_wait (&barber_seat);            /* Wait for the barber to become free */
    sem_post (&waiting_room);           /* Let people waiting outside the shop know */
    sem_post (&barber_bed);             /* Wake up barber */
    sem_wait (&done_with_customer);     /* Wait until hair is cut */
    sem_post (&barber_seat);            /* Give up seat */

    printf ("Customer %d: Going home \n", number);

    pthread_exit (NULL);
}

/* Returns a random number between min and max */
int 
UD (int min, int max)
{
    return ((int) floor ((double) (min + (max - min + 1) * ((float) rand()/(float) RAND_MAX))));
}



