/* This code illustrates the use of simple signal handlers with the intention of 
 * catching some selected signals. 
 *
 * Signals are one of the mechanisms used by the kernel to communicate with the processes in user space. 
 * Signals can also be used as a rudimentary form of inter-process communication.
 *
 * Author: Naga Kandasamy
 * Date created: December 22, 2008
 * Date modified: January 14, 2020
 *
 * */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

static void custom_signal_handler (int);

int 
main (int argc, char **argv)
{
    /* The signals of interest will be caught by the same handler. */

    /* Set up the custom signal handler to catch the arithmetic exception. */
    signal (SIGFPE, custom_signal_handler);

    /* Set up signal handler to catch the Control+C signal. */
    signal (SIGINT, custom_signal_handler);

    /* Set up signal handler to catch the Control+C signal. */
    signal (SIGQUIT, custom_signal_handler);

    /* Set up signal handlers to catch user-defined signals SIGUSR1 and SIGUSR2. */
    signal (SIGUSR1, custom_signal_handler); 
    signal (SIGUSR2, custom_signal_handler);
  
    // int a = 10/0; // Should cause a divide-by-zero exception. Any decent compiler should also warn you!
    for (;;)
        pause ();
}

/* Important note: printf() is not reentrant and should not be used in
 * a signal handler.  It is only used here for the sake
 * of easily demonstrating signal handlers.  Production
 * code should never do this.  Instead, the signal handler
 * should set a flag on the heap or data segment that is
 * checked periodically outside of the signal handler.
 *
 * There are other non-reentrant library functions and system calls as well. 
 * Make sure that your signal handler never uses these functions.
*/

static void 
custom_signal_handler (int signalNumber)
{
    switch (signalNumber){
        case SIGFPE: 
             printf ("Caught an arithmetic exception! \n");
             printf ("Going through an orderly shut down process. \n");
             exit (EXIT_SUCCESS);
         
        case SIGINT:
             signal (SIGINT, custom_signal_handler); /* Restablish the signal handler for the next occurrence */
             printf ("Ouch...Caught the Control+C signal. \n");
             break;

         case SIGQUIT: 
             printf ("Caught the Quit signal. \n");
             printf ("Going through an orderly shutdown process. \n");
             exit (EXIT_SUCCESS);

         case SIGUSR1:
             signal (SIGUSR1, custom_signal_handler); 
             printf ("Received Signal SIGUSR1 \n");
             break;

         case SIGUSR2: 
             signal (SIGUSR1, custom_signal_handler); 
             printf ("Received Signal SIGUSR1 \n");
             break;

         default: 
             break;

    }
}

