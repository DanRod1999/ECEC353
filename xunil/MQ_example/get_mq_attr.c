/* Program retrieves attributes for a previously created 
 * message queue.
 *
 * Compile as follows: gcc -o get_mq_attr get_mq_attr.c -std=c99 -Wall -lrt 
 *
 * Author: Naga Kandsamy
 * Date created: july 12, 2018
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

int 
main (int argc, char **argv)
{
    mqd_t mqd;
    struct mq_attr attr;

    if (argc != 2 || strcmp (argv[1], "--help") == 0) {
        printf ("Usage: %s mq-name \n", argv[0]);
        exit (EXIT_FAILURE);
    }

    mqd = mq_open (argv[1], O_RDONLY);
    if (mqd == (mqd_t) -1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    if (mq_getattr (mqd, &attr) == -1) {
        perror ("mq_getattr");
        exit (EXIT_FAILURE);
    }

    printf ("Maximum # of messages on queue:    %ld \n", attr.mq_maxmsg);
    printf ("Maximum message size:              %ld \n", attr.mq_msgsize);
    printf ("# of messages currently in queue:  %ld \n", attr.mq_curmsgs);

    exit (EXIT_SUCCESS);
}
