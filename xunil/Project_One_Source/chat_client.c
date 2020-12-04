/* Skeleton code for the client side code. 
 *
 * Compile as follows: gcc -o chat_client chat_client.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandsamy
 * Date created: January 28, 2020
 * Date modified:
 *
 * Student/team name: FIXME
 * Date created: FIXME 
 *
*/

#define _POSIX_C_SOURCE 2 // For getopt()

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "msg_structure.h"

void 
print_main_menu (void)
{
    printf ("\n'B'roadcast message\n");
    printf ("'P'rivate message\n");
    printf ("'E'xit\n");
    return;
}

int 
main (int argc, char **argv)
{
    char user_name[USER_NAME_LEN];
    mqd_t mq_dmr;
    struct client_msg attr, *attrp;
    unsigned int priority = 0;
    // char *messageIn[10];

    if (argc != 2) {
        printf ("Usage: %s user-name\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    strcpy (user_name, argv[1]); /* Get the client user name */

    /* FIXME: Connect to server */
    printf ("User %s connecting to server\n", user_name);
    mq_dmr = mq_open ("/mq_DRodriguez", O_RDWR, S_IRUSR | S_IWUSR, attrp);
        if (mq_dmr == (mqd_t)-1) {
            perror ("mq_open");
            exit (EXIT_FAILURE);
        }

    /* Operational menu for client */
    char option, dummy;
    while (1) {
        print_main_menu ();
        option = getchar ();

        switch (option) {
            case 'B':
                /* FIXME: Send message to server to be broadcast */
                attrp->broadcast = 1;
                dummy = getchar ();
                fgets(attrp->msg,MESSAGE_LEN,stdin);
                if (mq_send (mq_dmr, (const char *)attrp, sizeof(attrp), priority) == -1) {
                    perror ("mq_send");
                    exit (EXIT_FAILURE);
                }
                break;

            case 'P':
                /* FIXME: Get name of private user and send the private 
                 * message to server to be sent to private user */
                attrp->broadcast = 0;
                break;

            case 'E':
                printf ("Chat client exiting\n");
                /* FIXME: Send message to server that we are exiting */
                exit (EXIT_SUCCESS);

            default:
                printf ("Unknown option\n");
                break;
                
        }
        /* Read dummy character to consume the \n left behind in STDIN */
    }

    exit (EXIT_SUCCESS);
}
