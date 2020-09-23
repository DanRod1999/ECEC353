/* Program adds a message to a previously created message queue. 
 *
 * Compile as follows: gcc -o send_msg send_msg.c -std=c99 -Wall -lrt

 * Author: Naga Kandasamy
 * Date created: July 12, 2018
 * Date modified: January 22, 2020
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
    fprintf (stderr, "Usage: %s [-n] mq-name msg [priority] \n", program_name);
    fprintf (stderr, " -n           Use O_NOBLOCK flag \n");
    exit (EXIT_FAILURE);
}


int 
main (int argc, char **argv)
{
    int flags, opt;
    mqd_t mqd;
    unsigned int priority;

    flags = O_WRONLY;
    while ((opt = getopt (argc, argv, "n")) != -1) {
        switch (opt){
            case 'n': 
                flags |= O_NONBLOCK;
                break;

            default:
                usage_error (argv[0]);
        }
    }
    
    if (optind + 1 >= argc)
        usage_error (argv[0]);

    /* Open the specified MQ for O_WRONLY operation */
    mqd = mq_open (argv[optind], flags);
    if (mqd == (mqd_t) -1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    /* Specify the priority of the message to be placed in the MQ */
    priority = (argc > optind + 2) ? atoi (argv[optind + 2]) : 0;
    
    if (mq_send (mqd, argv[optind + 1], strlen (argv[optind + 1]), priority) == -1) {
        perror ("mq_send");
        exit (EXIT_FAILURE);
    }

    exit (EXIT_SUCCESS);

}
