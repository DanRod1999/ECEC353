/* Program that accepts integers from STDIN until EOF and prints out the average. 
 *
 * Author: Naga Kandasamy
 * Date created: July 13, 2018
 * Date modifeid: January 23, 2020
 *
 * Compile as follows: gcc -o calc_avg calc_avg.c -std=c99 -Wall
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
    int sum = 0;
    int n = 0;
    while (fscanf (stdin, "%d", &my_num) != EOF) {
        n++;
        sum += my_num;
    }

    if (n > 0)
        fprintf (stdout, "%f \n", (float) sum/n);
    else
        fprintf (stdout, "%s \n", "Average cannot be calculated");

    exit (EXIT_SUCCESS);
}

