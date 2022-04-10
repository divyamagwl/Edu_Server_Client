#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>

#include "configs.h"

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
} server_msg_t;

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
                break;
            case 2:
                strcpy(out_msg.query_type, DELETE_COURSE);
                printf("Enter course name: ");
                scanf("%[^\n]%*c",query_msg);
                break;
            case 3:
                strcpy(out_msg.query_type, ADD_TEACHER);
                printf("Enter teacher name: ");
                scanf("%[^\n]%*c",query_msg);
                break;
            case 4:
                strcpy(out_msg.query_type, DELETE_TEACHER);
                printf("Enter teacher name: ");
                scanf("%[^\n]%*c",query_msg);
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

        strcpy(out_msg.query_details, query_msg);

        if (mq_send(qd_srv, (char *)&out_msg, sizeof(out_msg), 0) == -1)
        {
            perror("Client MsgQ: Not able to send message to the queue /server_msgq");
            continue;
        }

        printf("\nMessage sent successfully: %s %s\n", out_msg.query_type, out_msg.query_details);

        server_msg_t in_msg;
        if (mq_receive(qd_client, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        {
            perror("Client MsgQ: mq_receive from server");
            exit(1);
        }

        printf(BLUE);
        printf("Status code received from the server = %s\n", in_msg.status_code);

        int status = atoi(in_msg.status_code);
        if(status == SUCCESS) {
            printf(GREEN);
            printf("SUCCESS\n");
        }
        else {
            printf(RED);
            switch(status)
            {
                case COURSE_DUPLICATE:
                    printf("One of the course already exists\n");
                    break;
                case COURSE_NOT_EXISTS:
                    printf("Course does not exists\n");
                    break;
                case MAX_COURSE_FULL:
                    printf("Maximum number of courses reached\n");
                    break;
                case MIN_COURSE_REQD:
                    printf("Cannot delete more courses. Minimum number of courses reached\n");
                    break;
                case TEACHER_DUPLICATE:
                    printf("One of the teacher already exists\n");
                    break;
                case TEACHER_NOT_EXISTS:
                    printf("Teacher does not exists\n");
                    break;
                case MAX_TEACHER_FULL:
                    printf("Maximum number of teachers reached\n");
                    break;
                case MIN_TEACHER_REQD:
                    printf("Cannot delete more teachers. Minimum number of teachers reached\n");
                    break;
            }
        }
        printf(RESET);
    }

    printf("Client %d: bye\n", getpid());

    exit(0);
}
