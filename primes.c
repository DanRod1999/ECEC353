 /* Skeleton code from primes.
  * Author: Naga Kandasamy
  * Date created: June 28, 2018
  * Date updated: January 16, 2020 
  * Build your code as follows: gcc -o primes primes.c -std=c99 -Wall
  * */

/*
Daniel Rodriguez
Ecec353
Exercise 1
Using Set jump and Signal handlers in order to exit code that is being run
*/
#define _POSIX_C_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <limits.h>


#define FALSE 0
#define TRUE !FALSE

#define CTRLC  1
#define CTRLQ  2

unsigned long int num_found; /* Number of prime numbers found */
unsigned long int circle_buff[5];


static sigjmp_buf env;

int is_prime (unsigned int num)
{
    unsigned long int i;

    if (num < 2) {
        return FALSE;
    }
    else if (num == 2) {
        return TRUE;
    }
    else if (num % 2 == 0) {
        return FALSE;
    }
    else {
        for (i = 3; (i*i) <= num; i += 2) {
            if (num % i == 0) {
                return FALSE;
            }
        }
        return TRUE;
    }
}

//Report function prints all the elements
//in the circle buffer in the correct order
//from smallest to largest
void report ()
{
    //Use for loop and mod in order to find the correct starting point of the circle buffer
    printf("\n");
    for(int i = 0; i<5; i++){
        if(((num_found)%5+ i) >= 5){
            printf("%lu\n",circle_buff[((num_found)%5+ i) - 5]);
        }
        else{
            printf("%lu\n",circle_buff[(num_found)%5 +i]);
        }

    }
}

//Handler one takes care of SIGINT and ctrl c
static void custom_signal_handler_one (int signalNumber)
{
    siglongjmp (env, CTRLC);
}

/*Handler two takes care of SIGQUIT and ctrl \      */
static void custom_signal_handler_two (int signalNumber)
{
    siglongjmp (env, CTRLQ);
    //jumps code to  to switch case in main where report is ran and code is exited
}

int main (int argc, char** argv)
{
    /* Set up signal handler to catch the Control+C signal. */
    signal (SIGINT, custom_signal_handler_one);

    /* Set up signal handler to catch the Control \ signal. */
    signal (SIGQUIT, custom_signal_handler_two);

    unsigned long int num;
    num_found = 0;

    int ret;
    ret = sigsetjmp (env, TRUE);
    switch (ret) {
        case 0:
            /* Returned from explicit sigsetjmp call. */
            break;

        case CTRLC:
            report();
            exit (EXIT_SUCCESS);

        case CTRLQ:
            report();
            exit(EXIT_SUCCESS);
    }
    
    //given code
    printf ("Beginning search for primes between 1 and %lu. \n", LONG_MAX);
    for (num = 1; num < LONG_MAX; num++) {
        if (is_prime (num)) {

            //circle buffer that saves the last five prime numbers found in an array
            circle_buff[num_found % 5] = num;

            num_found++;
            printf ("%lu \n", num);
        }
    }


    exit (EXIT_SUCCESS);
}
