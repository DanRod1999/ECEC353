/* Program removes a message from a previously created message queue. 
 *
 * Author: Naga Kandsamy
 * Date created: July 12, 2018
 * Date modified: January 22, 2020
 *
 * Compile as follows: gcc -o recv_msg recv_msg.c -std=c99 -Wall -lrt
 *
 * Source: M. Kerrisk, The Linux Programming Interface. 
 *  */

#define _POSIX_C_SOURCE 2

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


void
usage_error (char *program_name)
{
    fprintf (stderr, "Usage: %s [-n] mq-name \n", program_name);
    fprintf (stderr, " -n           Use O_NOBLOCK flag \n");
    exit (EXIT_FAILURE);
}


int 
main (int argc, char **argv)
{
    int flags, opt;
    mqd_t mqd;
    unsigned int priority;
    void *buffer;
    struct mq_attr attr;
    ssize_t nr;

    flags = O_RDONLY;
    while ((opt = getopt (argc, argv, "n")) != -1) {
        switch (opt){
            case 'n': 
                flags |= O_NONBLOCK;
                break;

            default:
                usage_error (argv[0]);
        }
    }
    
    if (optind >= argc)
        usage_error (argv[0]);

    /* Open the MQ for O_RDONLY operation */
    mqd = mq_open (argv[optind], flags);
    if (mqd == (mqd_t) -1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    /* Get the attributes of the MQ */
    if (mq_getattr (mqd, &attr) == -1) {
        perror ("mq_getattr");
        exit (EXIT_FAILURE);
    }

    /* Allocate local buffer to store the received message from the MQ */
    buffer = malloc (sizeof(attr.mq_msgsize));
    if (buffer == NULL) {
        perror ("malloc");
        exit (EXIT_FAILURE);
    }

    nr = mq_receive (mqd, buffer, attr.mq_msgsize, &priority);
    if (nr == -1) {
        perror ("mq_receive");
        exit (EXIT_FAILURE);
    }

    printf ("Read %ld bytes; priority = %u \n", (long) nr, priority);
    if (write (STDOUT_FILENO, buffer, nr) == -1) {
        perror ("write");
        exit (EXIT_FAILURE);
    }

    exit (EXIT_SUCCESS);
}
