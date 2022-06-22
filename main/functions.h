#ifndef FUNC

#define FUNC





#include <stdio.h>

#include <unistd.h>

#include <sys/types.h>

#include <signal.h>

#include <unistd.h>

#include <pthread.h>

#include <time.h>

#include <string.h>

#include <stdlib.h>

#include <fcntl.h>

#include <error.h>

#include <errno.h>

#include <semaphore.h>

#include <sys/ipc.h>

#include <sys/msg.h>

#include <sys/shm.h>

#include <ctype.h>

#include <sys/stat.h>

#include <sys/wait.h>

#include <stdbool.h>

#include <fcntl.h>

#include <sys/stat.h>

#include <sys/types.h>

#include <unistd.h>





#define PIPE_NAME    "TASK_PIPE"

#define MAX_BUF 1024

typedef struct
{
  /* Message type */
  long mtype;

  int timeStop;                 
  char server_activated[50];
  char index;                     
} msg_queue;


typedef struct {

    int param;

    char name[50];

    int i;

    int id;

} vcpu_struct;

typedef struct {

    int num_pipes;

    int **pipes;

} pipe_struct;

typedef struct{

    int performance;

    char name[50];

    int param1;

    int param2;

    int tasks_completed;
    
    int maintenance;

    int thread; 

    int thread1; 

    int thread2; 

} edge_server;



typedef struct {

    int ID;

    int num_instrucoes;

    int max_time;

    int time_to_be_completed;

} task;



typedef struct {

    int num_tasks;

    task task_array[];

} taskstruct;

int mqid;

taskstruct tasks;

pid_t TaskManager, MaintenanceManager, Monitor;

pipe_struct pipes;

sem_t *sem_vcpu;

sem_t *sem_task;

sem_t *sem_ready;

sem_t *sem_sched;

sem_t *sem_log;

sem_t *sem_disp;

sem_t *sem_monitor;

sem_t *sem_monitor2;

sem_t *sem_end;


typedef struct

{

    sem_t sem_vcpu;

    int state;

    int response_totalTime;

    int deleted_tasks;

    int queue_pos,max_wait,edge_servers_num;

    sem_t sem_task;

    sem_t sem_log;

    sem_t sem_sched;

    sem_t sem_disp;

    sem_t sem_ready;

    sem_t sem_monitor;

    sem_t sem_monitor2;

    sem_t sem_end;

    edge_server edge_servers[];

} mem_structure;



mem_structure *mem;



pthread_mutex_t mutex4;
pthread_cond_t cond4;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
pthread_cond_t cond2;
pthread_cond_t cond3;

int fd;

char task1[20];

char task2[20];

int instructions1;

int instructions2;

int shmid;

time_t current_time;

struct tm * time_info;

char timeString[10];
time_t start;
time_t end;

void leftshift();

void escreve_log(char string[250]);

int fileread(char config_name[200],FILE *fp);

void escreve_log(char string[250]);

void* vCPUs_f(void * arg);

void edge_server_f(char *nome,int param1, int param2,int ufd[]);

void* scheduler_f();

void* dispatcher_f(void *arg);

void TaskManager_f();

void MaintenanceManager_f();

void Monitor_f();

void init();
int check();

void exit_offloading();
void sigint(int signum);
void stats(int signum);
void systemManager_f(char config_name[200]);
void removetask();
void* aux_f(void* arg);
void sortarray();
void * maintenance_aux(void * index);
void stats_task();

#endif //FUNC