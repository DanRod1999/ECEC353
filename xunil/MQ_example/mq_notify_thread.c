 /* Program illustrates how to receive notification on a message queue using 
 * a thread function.
 *
 * Compile as follows: gcc -o mq_notify_thread mq_notify_thread.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandsamy
 * Date created: January 22, 2020
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
#include <pthread.h>

static void setup_notification (mqd_t *mqdp);

static void 
thread_func (union sigval sv)
{
    ssize_t nr;
    mqd_t *mqdp;
    void *buffer;
    struct mq_attr attr;

    mqdp = sv.sival_ptr;

    if (mq_getattr (*mqdp, &attr) == -1) {
        perror ("mq_getattr");
        exit (EXIT_FAILURE);
    }

    buffer = malloc (attr.mq_msgsize);
    if (buffer == NULL) {
        perror ("malloc");
        exit (EXIT_FAILURE);
    }


    /* Reenable notification */
    setup_notification (mqdp);

    /* Drain the queue empty */
    while ((nr = mq_receive (*mqdp, buffer, attr.mq_msgsize, NULL)) >= 0)
        printf ("Read %ld bytes \n", (long) nr);

    free (buffer);

    return;
}

static void 
setup_notification (mqd_t *mqdp)
{
    struct sigevent sev;

    sev.sigev_notify = SIGEV_THREAD;            /* Notify via thread */
    sev.sigev_notify_function = thread_func;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = mqdp;           /* Argument to thread_func() */

    if (mq_notify (*mqdp, &sev) == -1) {
        perror ("mq_notify");
        exit (EXIT_FAILURE);
    }

    return;
}

int
main (int argc, char **argv)
{
    mqd_t mqd;

    /* Print usage information */
    if (argc != 2 || strcmp (argv[1], "--help") == 0) {
        printf ("%s mq-name\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    /* Open the message queue in non-blocking mode. So that after receiving 
     * a notificatiom we can completely drain the queue without blocking. 
     */
    mqd = mq_open (argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    /* Setup the notification system */
    setup_notification (&mqd);

    /* Simulate processing */
    int i = 0;
    while (1) {
        printf ("Processing with i = %d \n", i++);
        sleep (1);
    }

    exit (EXIT_SUCCESS);
}
