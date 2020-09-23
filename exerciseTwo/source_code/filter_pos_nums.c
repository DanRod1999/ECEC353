/* Program that accepts integers from STDIN, filters out the positive ones, and 
 * prints them out to STDOUT. 
 *
 * Author: Naga Kandasamy
 * Date created: July 13, 2018
 * Date modified: January 23, 2020
 *
 * Compile as follows: gcc -o filter_pos_nums filter_pos_nums.c -std=c99 -Wall
 * */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

int 
main (int argc, char ** argv)
{
    int my_num;
    while (fscanf (stdin, "%d", &my_num) != EOF) {
        if (my_num > 0)
            fprintf (stdout, "%d \n", my_num);
    }

    exit (EXIT_SUCCESS);
}

