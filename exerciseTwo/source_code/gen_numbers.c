/* Program that generates random integers between -5 and +5 and prints out to STDOUT. 
 * The number of integers to be generated is specified via the command line.
 *
 * Author: Naga Kandasamy
 * Date created: July 13, 2018
 * Date modified: January 23, 2020
 *
 * Compile as follows: gcc -o gen_numbers gen_numbers.c -std=c99 -Wall -lm
 * */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

int 
main (int argc, char **argv)
{
    if (argc != 2 || strcmp (argv[1], "--help") == 0) {
        printf ("Usage: %s num-integers \n", argv[0]);
        exit (EXIT_FAILURE);
    }

    int num_int = atoi (argv[1]);

    /* Set the seed for the random number generator for repeatability of 
     * experiments. Note: DO NOT CHANGE THIS NUMBER
     * */
    srand (2018);

    int rand_num;
    for (int i = 0; i < num_int; i++) {
        rand_num = floor ((rand ()/(float) RAND_MAX - 0.5) * 10);
        fprintf (stdout, "%d \n", rand_num);
    }

    exit (EXIT_SUCCESS);
}

