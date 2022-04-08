#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

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

static client_msg_t client_msg;

#define SERVER_QUEUE_NAME "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)

int main(int argc, char **argv)
{
    mqd_t qd_srv, qd_client; // Server and Client Msg queue descriptors
    int num = 1;

    printf("Server MsgQ: Welcome!!!\n");

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                          &attr)) == -1)
    {
        perror("Server MsgQ: mq_open (qd_srv)");
        exit(1);
    }

    client_msg_t in_msg;
    int val_client;
    while (1)
    {
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_srv, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        {
            perror("Server msgq: mq_receive");
            exit(1);
        }

        val_client = atoi(in_msg.msg_val);
        printf("%d: Server MsgQ: message received.\n", num);
        printf("Client msg q name = %s\n", in_msg.client_q);
        printf("Client msg val = %d\n", val_client);
        num++;

        server_msg_t out_msg;
        strcpy(out_msg.msg_type, "Server msg"); // strcpy(destPtr, srcPtr)
        sprintf(out_msg.msg_val, "%d", val_client + 10);

        // Open the client queue using the client queue name received
        if ((qd_client = mq_open(in_msg.client_q, O_WRONLY)) == 1)
        {
            perror("Server MsgQ: Not able to open the client queue");
            continue;
        }

        // Send back the value received + 10 to the client's queue
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send(qd_client, (char *)&out_msg, sizeof(out_msg), 0) == -1)
        {
            perror("Server MsgQ: Not able to send message to the client queue");
            continue;
        }

    } // end of while(1)

} // end of main()
