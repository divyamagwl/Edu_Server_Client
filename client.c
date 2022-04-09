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
#define QUERY_TYPE_LEN 16

// For the server queue message
#define MSG_TYPE_LEN 16

typedef struct
{
    char client_q[CLIENT_Q_NAME_LEN];
    char query_type[QUERY_TYPE_LEN];
    char query_details[MSG_VAL_LEN];
} client_msg_t;

typedef struct
{
    char msg_type[MSG_TYPE_LEN];
    char status_code[MSG_VAL_LEN];
    char msg_val[MSG_VAL_LEN];
} server_msg_t;

#define SERVER_QUEUE_NAME "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)

#define ADD_COURSE "ADD_COURSE"
#define DELETE_COURSE "DELETE_COURSE"
#define ADD_TEACHER "ADD_TEACHER"
#define DELETE_TEACHER "DELETE_TEACHER"

void display_menu() {
	printf("\n1. Add a course");
	printf("\n2. Delete a course");
	printf("\n3. Add a teacher");
	printf("\n4. Delete a teacher");
	printf("\n5. Quit program");
}

int main(int argc, char **argv)
{
    printf("Welcome to the Education Client!!!\n");

    mqd_t qd_srv, qd_client; // Server and Client Msg queue descriptors

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

    int quit = 0;
    int choice, validChoice;
    char query_msg[MSG_VAL_LEN];
    display_menu();

    while(!quit) {
        validChoice = 1;
        printf("\nEnter your choice : ");
	    scanf("%d%*c", &choice);

        switch(choice)
	    {
            case 1:
                strcpy(out_msg.query_type, ADD_COURSE);
                printf("Enter course name: ");
                scanf("%[^\n]%*c",query_msg);
                strcpy(out_msg.query_details, query_msg);
                break;
            case 2:
                strcpy(out_msg.query_type, DELETE_COURSE);
                printf("Enter course name: ");
                scanf("%[^\n]%*c",query_msg);
                strcpy(out_msg.query_details, query_msg);
                break;
            case 3:
                strcpy(out_msg.query_type, ADD_TEACHER);
                printf("Enter teacher name: ");
                scanf("%[^\n]%*c",query_msg);
                strcpy(out_msg.query_details, query_msg);
                break;
            case 4:
                strcpy(out_msg.query_type, DELETE_TEACHER);
                printf("Enter teacher name: ");
                scanf("%[^\n]%*c",query_msg);
                strcpy(out_msg.query_details, query_msg);
                break;            
            case 5:
                quit = 1;
                break;
            default:
                printf("Please Enter a Valid Choice!!");
                validChoice = 0;
                break;
        }

        if(quit) break;
        if(!validChoice) continue;

        // Send message to my_msgq_rx queue
        if (mq_send(qd_srv, (char *)&out_msg, sizeof(out_msg), 0) == -1)
        {
            perror("Client MsgQ: Not able to send message to the queue /server_msgq");
            continue;
        }

        printf("Message sent successfully: %s %s\n", out_msg.query_type, out_msg.query_details);

        server_msg_t in_msg;
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_client, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        {
            perror("Client MsgQ: mq_receive from server");
            exit(1);
        }

        printf("Status code received from the server = %s with msg = %s.", in_msg.status_code, in_msg.msg_val);

    }

    printf("Client %d: bye\n", getpid());

    exit(0);
}
