/* This code illustrates a simple signal handler that catches the alarm signal on a periodic basis 
 *
 * Author: Naga Kandasamy
 * Date created: 6/27/2018
 *
 * */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

static void alarm_handler (void); /* Signal handler to catch the ALRM signal */
static void ctrl_c_handler (void); /* Signal handler to catch CTRL+C signal */

int int_flag = FALSE;

int 
main (int argc, char **argv)
{
    int alarm_interval = 2; /* Set our alarm to ring every two seconds */
  
    /* Set up our signal handler to catch SIGALRM */
    if (signal (SIGALRM, alarm_handler) == SIG_ERR){ 
        printf ("Cannot catch SIGALRM \n");
        exit (0);
    }

    /* Set up another signal handler to catch SIGINT */
    if (signal (SIGINT, ctrl_c_handler) == SIG_ERR){ 
        printf ("Cannot catch SIGINT \n");
        exit (0);
    }

    while (TRUE){
        alarm (alarm_interval); /* Set our alarm to ring at the specified interval */ 
        pause (); /* Suspend our process here until a signal is caught */
        if (int_flag == TRUE){
            printf ("SIGINT received. Shutting down. \n");
	
            /* Some clean-up code here */
	
            exit (0); 
        }
        /* Simulate some processing after the occurence of the alarm */
    }
}

/* The user-defined signal handler for SIGALRM */
static void 
alarm_handler (void)
{
    printf ("Alarm caught. \n");
    signal (SIGALRM, alarm_handler); /* Restablish handler for next occurrence */
    return;
}

/* The user-defined signal handler for SIGINT */
static void 
ctrl_c_handler (void)
{
    int_flag = 1;
    return;
}
