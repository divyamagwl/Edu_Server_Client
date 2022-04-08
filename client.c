// Lab11 MsgQ Client process
// Compilation of this file
// gcc -o msgqclt lab11_client.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>

#define MSG_VAL_LEN 16
// For the client queue message
#define CLIENT_Q_NAME_LEN 16

// For the server queue message
#define MSG_TYPE_LEN 16

typedef struct
{
    char client_q[CLIENT_Q_NAME_LEN];
    char msg_val[MSG_VAL_LEN];
} client_msg_t;

typedef struct
{
    char msg_type[MSG_TYPE_LEN];
    char msg_val[MSG_VAL_LEN];
} server_msg_t;

#define SERVER_QUEUE_NAME "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)

int main(int argc, char **argv)
{
    char client_queue_name[64];
    mqd_t qd_srv, qd_client; // Server and Client Msg queue descriptors
    int num = 1;

    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1)
    {
        perror("Client MsgQ: mq_open (qd_srv)");
        exit(1);
    }

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    client_msg_t out_msg;
    // create the client queue for receiving messages from the server
    sprintf(out_msg.client_q, "/clientQ-%d", getpid());

    if ((qd_client = mq_open(out_msg.client_q, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                             &attr)) == -1)
    {
        perror("Client msgq: mq_open (qd_client)");
        exit(1);
    }

    printf("Keep sending msgs to the /server_msgq and wait for the reply ...\n");

    while (1)
    {
        sprintf(out_msg.msg_val, "%d", num);
        // send message to my_msgq_rx queue
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send(qd_srv, (char *)&out_msg, sizeof(out_msg), 0) == -1)
        {
            perror("Client MsgQ: Not able to send message to the queue /server_msgq");
            continue;
        }

        printf("%d: Client MsgQ: Message sent successfully\n", num);

        sleep(5); // sleep for 5 seconds
        num++;

        server_msg_t in_msg;
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_client, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        {
            perror("Client MsgQ: mq_receive from server");
            exit(1);
        }

        int val_server = atoi(in_msg.msg_val);
        printf("Client MsgQ: Msg received from the server with val = %d\n", val_server);
    }

    printf("Client MsgQ: bye\n");

    exit(0);
}
