#define MSG_VAL_LEN 16
#define CLIENT_Q_NAME_LEN 16
#define QUERY_TYPE_LEN 16
#define MSG_TYPE_LEN 16

#define COURSE_NAME_LEN 16
#define TEACHER_NAME_LEN 16

#define SERVER_QUEUE_NAME "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)


#define ADD_COURSE "ADD_COURSE"
#define DELETE_COURSE "DELETE_COURSE"
#define ADD_TEACHER "ADD_TEACHER"
#define DELETE_TEACHER "DELETE_TEACHER"


#define SUCCESS 200
#define TEACHER_EXISTS 401
#define MAX_TEACHER_FULL 402