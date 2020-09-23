#ifndef _MSG_STRUCTURE_H_
#define _MSG_STRUCTURE_H_

#define USER_NAME_LEN 32
#define MESSAGE_LEN 256

/* Structure of the client ---> server message */
struct client_msg {
    int client_pid;                     /* Process ID of the client contacting the server */
    char user_name[USER_NAME_LEN];      /* User name or handle of the client chat user */
    int broadcast;                       /* 1 if message must be broadcasted, 0 if preivate */
    char priv_user_name[USER_NAME_LEN]; /* Intended party in case of private message */
    char msg[MESSAGE_LEN];              /* Message */
    int control;                        /* Flag indicating control msg, say connect or disconnect */
};

/* Structure of the server ---> client message */
struct server_msg {
    char sender_name[USER_NAME_LEN];    /* User name of the originating chat user */
    char msg[MESSAGE_LEN];              /* Message */
};

#endif
