/* Skeleton code for the server side code. 
 * 
 * Compile as follows: gcc -o chat_server chat_server.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandasamy
 * Date created: January 28, 2020
 *
 * Student/team name: Dan and Isaac
 * Date created: FIXME  
 *
 */

#define _POSIX_C_SOURCE 2

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <limits.h>
#include "msg_structure.h"

#define FALSE 0
#define TRUE !FALSE

#define CTRLC  1

static sigjmp_buf env;


/* FIXME: Establish signal handler to handle CTRL+C signal and 
 * shut down gracefully. 
 */

typedef struct _LLNODE {
   // struct client_msg clientAttr;
   mqd_t fileD;
   char user_name[USER_NAME_LEN];
   int pid;
   struct _LLNODE * pNext;
} LLNODE, *PLLNODE;


PLLNODE appendList(PLLNODE head, int PID, char user[USER_NAME_LEN]){
   PLLNODE pNode;
   pNode = malloc(sizeof(LLNODE));

    char cliName[20] = "/client_";
    strcat(cliName,user);

    mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP;
    int flags = O_WRONLY | O_NONBLOCK;

    pNode -> fileD = mq_open (cliName, flags, perms, NULL);
    if (pNode-> fileD  == (mqd_t)-1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }
   
   //set node attr to passed in variables
   pNode -> pid = PID;
   strcpy(pNode -> user_name,user);
   //set pNext to head
   pNode -> pNext = head;
   //make pNode new head
   head = pNode;

   return head;//dont know if we need this
}

PLLNODE removeList(PLLNODE head, int PID_remove){
	PLLNODE pNode = head;
	PLLNODE pPrev = NULL;
	if(PID_remove == head->pid && head -> pNext != NULL){//if head of list holds correct pid
			head = pNode->pNext;
            mq_unlink(pNode->fileD);
            mq_close(pNode->fileD);
			free(pNode);
			return head;
		}else if(PID_remove == head->pid && head -> pNext == NULL){
            return NULL;
        }
		pPrev = head;
		pNode = head->pNext;

	while(pNode != NULL){
		if(PID_remove == pNode->pid){//pid matches a client pid attribute
			if(pNode->pNext != NULL){//if pNode is not tail
				pPrev->pNext = pNode->pNext;//pNext of pPrev is now equal to pNext of P node
				mq_unlink(pNode->fileD);
                mq_close(pNode->fileD);
                free(pNode);//get rid of pNode
				return head;
			}else{//if tail
				pPrev->pNext = NULL;
				return head;
			}
		}
		pNode = pNode->pNext;
		pPrev = pNode;	
	}
	return head;
} 

 static void custom_signal_handler (int signalNumber)
{
    siglongjmp (env, CTRLC);
}

int
main (int argc, char **argv)
{
    mqd_t server_fd;
    int flags, opt;
    mode_t perms;
    void *buffer;
    struct server_msg attrS, broadcast_msg, private_msg, *attrpS;
    struct client_msg attrC, received_msg, *attrpC;
    ssize_t nr;
    unsigned int priority = 0;

   //signal handler to catch CTRLC signal
    signal (SIGINT, custom_signal_handler);
    int ret;
    ret = sigsetjmp (env, TRUE);
    switch (ret) {
        case 0:
            /* Returned from explicit sigsetjmp call. */
            break;
        case CTRLC:
            printf("\nClosing Message Queue\n");
                 mq_unlink("/mq_DRodriguez");
                 mq_close("/mq_DRodriguez");
                exit (EXIT_SUCCESS);
            }

    attrpS = NULL;
    flags = O_RDONLY | O_CREAT;
    perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP;
    //open server mq
    server_fd = mq_open ("/mq_DRodriguez", flags, perms, attrpS);
    if (server_fd  == (mqd_t)-1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    char dummy;
    char priv_user[USER_NAME_LEN];
    PLLNODE head = NULL;

    while (1) {

        int control = 2;

        //receive each incoming client message
        if ( mq_receive(server_fd, (char *) &received_msg, 8192, priority) == -1 ) {
            perror ("mq_receive");
            exit (EXIT_FAILURE);
        }

        control = received_msg.control;

        if(control == 0) {
            printf("\nClient %s is disconnecting\n", received_msg.user_name);
            //remove client from list
            head = removeList(head, received_msg.client_pid);
        }

        else if(control == 1){
            printf("\nClient %s is connecting\n", received_msg.user_name);
            //add client to list
            head = appendList(head, received_msg.client_pid, received_msg.user_name);
        }
        else{
            if (received_msg.broadcast == 1) {
                //broadcast message code
                printf("\n%s: %s", received_msg.user_name, received_msg.msg);
                //open and send to each client mq

                strcpy(broadcast_msg.msg, received_msg.msg);//extract message
                strcpy(broadcast_msg.sender_name, received_msg.user_name);//extract sender name

                int S_Pflags = O_WRONLY | O_NONBLOCK;
                int S_Pperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP;

                PLLNODE client_node = head;
                while(client_node->pNext != NULL){
                    if(strcmp(broadcast_msg.sender_name, client_node->user_name) != 0){
                        // mqd_t S_CP_fd = mq_open (client_node->fileD, S_Pflags, S_Pperms, attrpS);//open receiving mq
                        mq_send(client_node->fileD, (const char *) &broadcast_msg, sizeof(broadcast_msg), priority);//send message to client 
                    }
                    client_node = client_node->pNext; 
            }
            }
            if (received_msg.broadcast == 0) {
                //private message code

                //extract receiving mq name
                strcpy(priv_user, received_msg.priv_user_name);
                char priv_mq_name[20] = "/client_";
                strcat(priv_mq_name, priv_user);
                int len = strlen(priv_mq_name);
                priv_mq_name[len-1] = 0;

                int S_Pflags = O_WRONLY | O_NONBLOCK;
                int S_Pperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP;
                strcpy(private_msg.msg, received_msg.msg);//extract message
                strcpy(private_msg.sender_name, received_msg.user_name);//extract sender name

                //open and send message to user specified by priv_user
                mqd_t S_CP_fd = mq_open (priv_mq_name, S_Pflags, S_Pperms, attrpS);//open receiving mq
                mq_send(S_CP_fd, (const char *) &private_msg, sizeof(private_msg), priority);//send message to client
            }
        }


    }


    exit (EXIT_SUCCESS);
}