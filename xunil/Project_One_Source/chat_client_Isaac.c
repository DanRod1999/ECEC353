/* Skeleton code for the client side code.
 *
 * Compile as follows: gcc -o chat_client chat_client_Isaac.c -std=c99 -Wall -lrt
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

    if (argc != 2) {
        printf ("Usage: %s user-name\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    strcpy (user_name, argv[1]); /* Get the client user name */


    /* FIXME: Connect to server */
    printf ("User %s connecting to server\n", user_name);

    int flags;
    mode_t perms;
    mqd_t client_fd;
    struct server_msg attrS, privS, *attrpS, recieved_msg;
    struct client_msg attrC, connectC, privC, *attrpC, exitC;
    ssize_t nr;
    unsigned int priority = 0;

    int PID = getpid();
    // char pid[8];
    // sprintf(pid, "%d", PID);
    char cliName[20] = "/client_";
    strcat(cliName,user_name);


    attrpC = NULL;
    attrC.client_pid = PID;
    strcpy(attrC.user_name, user_name);

    flags = O_RDONLY | O_CREAT | O_NONBLOCK;
    perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP;

    client_fd = mq_open (cliName, flags, perms, attrpC);
    if (client_fd  == (mqd_t)-1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    char priv_user_name[USER_NAME_LEN];
    char msg[MESSAGE_LEN + 1];
    char option, dummy, dummy2, dummy3;
    int flags_to_server = O_WRONLY;
    mqd_t C_S_fd;
    mqd_t C_S_send;
    mqd_t C_S_send_priv;

    C_S_fd = mq_open("/mq_DRodriguez", flags_to_server, perms, attrpC);
        if (C_S_fd == (mqd_t) - 1) {
            perror("mq_open");
            exit(EXIT_FAILURE);
        }

    /* set up startup control message */
    connectC.control = 1;
    connectC.client_pid = PID;
    strcpy(connectC.user_name, user_name);
    mqd_t startup_msg_fd;

    startup_msg_fd = mq_send(C_S_fd, (const char *) &connectC, sizeof(connectC), priority);
    if (startup_msg_fd == (mqd_t) - 1) {
        perror ("mq_send");
        exit (EXIT_FAILURE);
    }


    char priv_user[USER_NAME_LEN];

    /* Operational menu for client */
    while (1) {

        while(mq_receive(client_fd,(char *)&recieved_msg, 8192, priority)>= 0){
            printf("%s: %s:\n",recieved_msg.sender_name,recieved_msg.msg);
        }

        print_main_menu ();
        option = getchar ();

        switch (option) {
            case 'B':
                attrC.broadcast = 1;
                attrC.control = 2;
                dummy = getchar();

                fgets(msg, MESSAGE_LEN, stdin);
                strcpy(attrC.msg, msg);

                C_S_send = mq_send(C_S_fd, (const char *) &attrC, sizeof(attrC), priority);
                if (C_S_send == (mqd_t) - 1) {
                    perror ("mq_send");
                    exit (EXIT_FAILURE);
                }

                break;

            case 'P':
                attrC.broadcast = 0;
                attrC.control = 2;
                printf("Enter the username of the message recipient:\n");
                dummy2 = getchar();

                fgets(priv_user, USER_NAME_LEN, stdin);
                strcpy(privC.priv_user_name, priv_user);
                privC.control = attrC.control;

                printf("Enter your message to %s \n", priv_user);

                fgets(msg, MESSAGE_LEN, stdin);
                strcpy(privC.msg, msg);

                C_S_send_priv = mq_send(C_S_fd, (const char *) &privC, sizeof(privC), priority);
                if (C_S_send_priv == (mqd_t) - 1) {
                    perror ("mq_send");
                    exit (EXIT_FAILURE);
                }
                break;

            case 'E':
                printf ("Chat client exiting\n");
                exitC.control = 0;
                exitC.client_pid = PID;
                strcpy(exitC.user_name, user_name);
                if(mq_send(C_S_fd, (const char *) &exitC, sizeof(exitC), priority) == -1) {
                    perror ("mq_send");
                    exit(EXIT_FAILURE);
                }

                exit (EXIT_SUCCESS);

            default:
                printf ("Unknown option\n");
                break;

        }
        /* Read dummy character to consume the \n left behind in STDIN */
        //dummy = getchar ();
    }

    exit (EXIT_SUCCESS);
}