/* Program creates a message queue using parameters specified from the command 
 * line. 
 *
 * Compile as follows: gcc -o create_mq creat_mq.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandsamy
 * Date created: july 12, 2018
 * Date modified: January 22. 2020
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
#include <errno.h>

static void 
usage_error (const char *program_name)
{
    fprintf (stderr, "Usage: %s [-cx] [-m maxmsg] [-s msgsize] mq-name [octal-perms] \n", program_name);

    fprintf (stderr, " -c               Create queue (O_CREAT) \n");
    fprintf (stderr, " -m maxmsg        Set maximum number of messages \n");
    fprintf (stderr, " -s msgsize       Set maximum message size \n");
    fprintf (stderr, " -x               Create exclusively (O_EXCL) \n");

    exit (EXIT_FAILURE);
}

int 
main (int argc, char **argv)
{
    int flags, opt;
    mode_t perms;
    mqd_t mqd;
    struct mq_attr attr, *attrp;

    /* Set the default message queue attributes. */
    attrp = NULL;
    attr.mq_maxmsg = 10;    /* Maximum number of messages on queue */
    attr.mq_msgsize = 2048; /* Maximum message size in bytes */
    flags = O_RDWR;         /* Create or open the queue for reading and writing */

    /* Parse command-line arguments and set the message queue attributes as specified */
    while ((opt = getopt (argc, argv, "cm:s:x")) != -1) {
        switch (opt) {
            case 'c': 
                flags |= O_CREAT;   /* Create the MQ if it doesn't already exist */
                break;

            case 'm':
                attr.mq_maxmsg = atoi (optarg);
                attrp = &attr;
                break;

            case 's':
                attr.mq_msgsize = atoi (optarg);
                attrp = &attr;
                break;

            case 'x':
                flags |= O_EXCL;    /* With O_CREAT, create the queue exclusively */
                break;

            default: 
                usage_error (argv[0]);
        }
    }

    /* In case of incorrect cmd-line agruments, show usage to user. */
    if (optind >= argc)
        usage_error (argv[0]);
    
    perms = S_IRUSR | S_IWUSR;  /* rw------- permissions on the queue */

    mqd = mq_open (argv[optind], flags, perms, attrp);
    if (mqd == (mqd_t)-1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    exit (EXIT_SUCCESS);
}