/* Skeleton code for the server side code. 
 * 
 * Compile as follows: gcc -o chat_server chat_server.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandasamy
 * Date created: January 28, 2020
 *
 * Student/team name: FIXME
 * Date created: FIXME  
 *
 */

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "msg_structure.h"

/* FIXME: Establish signal handler to handle CTRL+C signal and 
 * shut down gracefully. 
 */

int 
main (int argc, char **argv) 
{
    while (1) {
       /* FIXME: Server code here */
    }
    exit (EXIT_SUCCESS);
}
