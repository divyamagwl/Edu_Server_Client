#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

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
    char msg_val[MSG_VAL_LEN];
} server_msg_t;


int MIN_COURSES = 10;
int MAX_COURSES = 15;
int MIN_TEACHERS = 5;
int MAX_TEACHERS = 10;

struct Teacher {
    char name[TEACHER_NAME_LEN];
} teachers[10];

struct Course {
    char name[COURSE_NAME_LEN];
    char teacher[TEACHER_NAME_LEN];
} courses[15];


void init_config(int configurable) {
    /*
    The initial configuration of the Edu Server with respect to the minimum and maximum values are configurable.
    MIN_COURSES 10, MAX_COURSES 15, MIN_TEACHERS 5, MAX_TEACHERS 10

    On Edu Server invocations, these above values can be given as configurable parameters
    if the given values do not fall in the above limits, the above values will be chosen by the Server
    */

    int min_courses, max_courses, min_teachers, max_teachers;
    
    printf("Please set these initial configurations of the server:\n");

    if(configurable) {
    printf("MIN COURSES (min 10): ");
    scanf("%d", &min_courses);
    printf("MAX COURSES (max 15): ");
    scanf("%d", &max_courses);
    printf("MIN TEACHERS (min 5): ");
    scanf("%d", &min_teachers);
    printf("MAX COURSES (max 10): ");
    scanf(" %d%*c", &max_teachers); // Space is required before %d : https://stackoverflow.com/questions/26085237/simple-c-code-not-working

    if(min_courses <= max_courses) {
        if(min_courses >= MIN_COURSES && min_courses <= MAX_COURSES) {
            MIN_COURSES = min_courses;
        }
        if(max_courses >= MIN_COURSES && max_courses <= MAX_COURSES) {
            MAX_COURSES = max_courses;
        }
    }

    if(min_teachers <= max_teachers) {
        if(min_teachers >= MIN_TEACHERS && min_teachers <= MAX_TEACHERS) {
            MIN_TEACHERS = min_teachers;
        }
        if(max_teachers >= MIN_TEACHERS && max_teachers <= MAX_TEACHERS) {
            MAX_TEACHERS = max_teachers;
        }
    }
    }

    for(int i = 0; i < MAX_COURSES; i++) {
        char name[COURSE_NAME_LEN] = "NULL";
        char teacher[TEACHER_NAME_LEN] = "NULL";
        struct Course c;
        strncpy(c.name, name, sizeof(c.name));
        strncpy(c.teacher, teacher, sizeof(c.teacher));
        courses[i] = c;
    }

    for(int i = 0; i < MAX_TEACHERS; i++) {
        char name[TEACHER_NAME_LEN] = "NULL";
        struct Teacher t;
        strncpy(t.name, name, sizeof(t.name));
        teachers[i] = t;
    }

    printf("Initial server configuration set!\n");
    return;
}

int add_teacher(char* name) {

    // Checking if the teacher already exists
    for(int i = 0; i < MAX_TEACHERS; i++) {
        struct Teacher t = teachers[i];
        if(strcmp(name, t.name) == 0) {
            return STATUS_TEACHER_EXISTS;
        }
    }

    // Adding teacher to the first place where name is NULL
    for(int i = 0; i < MAX_TEACHERS; i++) {
        struct Teacher *t = malloc(sizeof(struct Teacher));
        t = &teachers[i];
        if(strcmp("NULL", t->name) == 0) {
            strcpy(t->name, name);
            return STATUS_SUCCESS;
        }
    }

    return STATUS_MAX_TEACHER_FULL;
}


int main(int argc, char **argv)
{
    mqd_t qd_srv, qd_client; // Server and Client Msg queue descriptors

    printf("Welcome to the Education Server!!!\n");
    init_config(0); // Initial server configuration
    
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
    int status;

    while (1)
    {
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_srv, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        {
            perror("Server msgq: mq_receive");
            exit(1);
        }

        char client_query_type[QUERY_TYPE_LEN];
        char client_query_details[MSG_VAL_LEN];

        strcpy(client_query_type, in_msg.query_type);
        strcpy(client_query_details, in_msg.query_details);

        printf("Client msg q name = %s\n", in_msg.client_q);
        printf("Query type = %s Details = %s.\n", client_query_type, client_query_details);

        if(strcmp(client_query_type, ADD_COURSE) == 0) {
            printf("Add karo course\n");
        }
        else if(strcmp(client_query_type, DELETE_COURSE) == 0) {
            printf("Delete karo course\n");
        }
        else if(strcmp(client_query_type, ADD_TEACHER) == 0) {
            status = add_teacher(client_query_details);

            for(int i = 0; i < MAX_TEACHERS; i++) {
                struct Teacher t = teachers[i];
                printf("Teacher name = %s", t.name);
            }
        }
        else if(strcmp(client_query_type, DELETE_TEACHER) == 0) {
            printf("Delete karo teacher\n");
        }

        server_msg_t out_msg;
        strcpy(out_msg.msg_type, "Server msg"); 
        sprintf(out_msg.status_code, "%d", status);
        strcpy(out_msg.msg_val, "All good"); 

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
