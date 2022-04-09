#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

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

sem_t bin_sem;


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

int add_course(char* name) {
    // Checking if the course already exists
    for(int i = 0; i < MAX_COURSES; i++) {
        struct Course c = courses[i];
        if(strcmp(name, c.name) == 0) {
            return COURSE_DUPLICATE; // ERROR: Course with given name already exists
        }
    }

    // Adding course to the first place where name is NULL
    for(int i = 0; i < MAX_COURSES; i++) {
        struct Course *c = malloc(sizeof(struct Course));
        c = &courses[i];
        if(strcmp("NULL", c->name) == 0) {
            strcpy(c->name, name);

            // Assign teacher randomly from current teachers
            struct Teacher available_teachers[10];
            int current_teacher = 0;
            for(int i = 0; i < MAX_TEACHERS; i++) {
                struct Teacher *t = malloc(sizeof(struct Teacher));
                t = &teachers[i];
                if(strcmp("NULL", t->name) != 0) {
                    available_teachers[current_teacher] = *t;
                    current_teacher++;
                }
            }

            if(current_teacher != 0) {
                int r = rand() % current_teacher;
                struct Teacher t = available_teachers[r];
                strcpy(c->teacher, t.name);
                printf("%s %s\n", c->name, c->teacher);
            }
            return SUCCESS;
        }
    }

    return MAX_COURSE_FULL; // ERROR: Maximum courses reached
}

int delete_course(char* name) {
    // Checking if the course already exists or not
    for(int i = 0; i < MAX_COURSES; i++) {
        struct Course *c = malloc(sizeof(struct Course));
        c = &courses[i];
        if(strcmp(name, c->name) == 0) {
            strcpy(c->name, "NULL");
            strcpy(c->teacher, "NULL");
            return SUCCESS; 
        }
    }

    return COURSE_NOT_EXISTS; // ERROR: Course does not exist so can't be removed
}


int add_teacher(char* name) {
    // Checking if the teacher already exists
    for(int i = 0; i < MAX_TEACHERS; i++) {
        struct Teacher t = teachers[i];
        if(strcmp(name, t.name) == 0) {
            return TEACHER_DUPLICATE; // ERROR: Teacher with given name already exists
        }
    }

    // Adding teacher to the first place where name is NULL
    for(int i = 0; i < MAX_TEACHERS; i++) {
        struct Teacher *t = malloc(sizeof(struct Teacher));
        t = &teachers[i];
        if(strcmp("NULL", t->name) == 0) {
            strcpy(t->name, name);
            return SUCCESS;
        }
    }

    return MAX_TEACHER_FULL; // ERROR: Maximum teachers reached
}

int delete_teacher(char* name) {
    // Checking if the teacher already exists or not
    for(int i = 0; i < MAX_TEACHERS; i++) {
        struct Teacher *t = malloc(sizeof(struct Teacher));
        t = &teachers[i];
        if(strcmp(name, t->name) == 0) {

            for(int i = 0; i < MAX_COURSES; i++) {
                struct Course *c = malloc(sizeof(struct Course));
                c = &courses[i];
                if(strcmp(t->name, c->name) == 0) {
                    strcpy(c->teacher, "NULL");
                }
            }

            strcpy(t->name, "NULL");
            return SUCCESS; 
        }
    }

    return TEACHER_NOT_EXISTS; // ERROR: Maximum teachers reached
}

void* generate_report() {
    while(1) {
        sem_wait(&bin_sem);  // Thread gets blocked if bin_sem is not free
        // sem_init(&bin_sem, 0, 0); // make bin_sem not free      
        printf("\n------------REPORT STARTED----------------\n");

        printf("COURSES: \n");
        for(int i = 0; i < MAX_COURSES; i++) {
            struct Course c = courses[i];
            if(strcmp("NULL", c.name) != 0) {
                printf("Name: %s Teacher: %s\n", c.name, c.teacher);
            }
        }

        printf("TEACHERS: \n");
        for(int i = 0; i < MAX_TEACHERS; i++) {
            struct Teacher t = teachers[i];
            if(strcmp("NULL", t.name) != 0) {
                printf("Name: %s\n", t.name);
            }
        }

        printf("\n------------REPORT ENDED----------------\n");
        sem_post(&bin_sem);
        sleep(5);
    }
}


int main(int argc, char **argv)
{
    srand(time(NULL));   // Random number initialized, should only be called once.

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

    int res_sem, res;
    pthread_t thread;
    
    res_sem = sem_init(&bin_sem, 0, 1);
    if(res_sem != 0) {
        printf("Semaphore creation failure: %d\n", res_sem);
        exit(1);
    }

    if( (res = pthread_create( &thread, NULL, &generate_report, "Thread")) )  {
      printf("Thread creation failed: %d\n", res);
      exit(1);
   }

    client_msg_t in_msg;
    int status;

    while (1)
    {
        if (mq_receive(qd_srv, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        {
            perror("Server msgq: mq_receive");
            exit(1);
        }

        sem_wait(&bin_sem);

        char client_query_type[QUERY_TYPE_LEN];
        char client_query_details[MSG_VAL_LEN];

        strcpy(client_query_type, in_msg.query_type);
        strcpy(client_query_details, in_msg.query_details);

        printf("Client msg q name = %s\n", in_msg.client_q);
        printf("Query type = %s Details = %s.\n", client_query_type, client_query_details);

        if(strcmp(client_query_type, ADD_COURSE) == 0) {
            status = add_course(client_query_details);
        }
        else if(strcmp(client_query_type, DELETE_COURSE) == 0) {
            status = delete_course(client_query_details);
        }
        else if(strcmp(client_query_type, ADD_TEACHER) == 0) {
            status = add_teacher(client_query_details);
        }
        else if(strcmp(client_query_type, DELETE_TEACHER) == 0) {
            status = delete_teacher(client_query_details);
        }

        server_msg_t out_msg;
        strcpy(out_msg.msg_type, "Server msg");
        sprintf(out_msg.status_code, "%d", status);

        // Open the client queue using the client queue name received
        if ((qd_client = mq_open(in_msg.client_q, O_WRONLY)) == 1)
        {
            perror("Server MsgQ: Not able to open the client queue");
            continue;
        }

        if (mq_send(qd_client, (char *)&out_msg, sizeof(out_msg), 0) == -1)
        {
            perror("Server MsgQ: Not able to send message to the client queue");
            continue;
        }

        sem_post(&bin_sem);
    } // end of while(1)

} // end of main()
