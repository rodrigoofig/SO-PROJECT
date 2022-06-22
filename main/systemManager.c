//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687

#include "functions.h"

void escreve_log(char string[250]){
	sem_wait(sem_log);
	FILE *fpointer;
	fpointer=fopen("log.txt","a");
	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, 8, "%H:%M", time_info);
	fprintf(fpointer,"%s %s",timeString,string);
	fclose(fpointer);
	sem_post(sem_log);

}

int fileread(char config_name[200],FILE *fp){

	char str[250];
	fgets(str,sizeof(str),fp);

	if(str == NULL){

		printf("=> ERROR IN FILE CONFIG\n");

		return 1;

	}
	mem->state = 1;
	mem->deleted_tasks = 0;
	mem->response_totalTime = 0;

	mem->queue_pos = atoi(str);

	fgets(str,sizeof(str),fp);
	if(str == NULL){
		printf("=> ERROR IN FILE CONFIG\n");
		return 1;

	}

	mem->max_wait = atoi(str);

	fgets(str,sizeof(str),fp);
	if(str == NULL){
		printf("=> ERROR IN FILE CONFIG\n");
		return 1;
	}
	mem->edge_servers_num = atoi(str);
	for(int i=0;i<mem->edge_servers_num;i++){
		edge_server edge;
		fgets(str,sizeof(str),fp);
		char *sep = strtok(str,",");
		if(sep == NULL){
			printf("=> ERROR IN FILE CONFIG\n");
			return 1;
		}
		strcpy(edge.name,sep);
		sep = strtok(NULL,",");
		if(sep == NULL){
			printf("=> ERROR IN FILE CONFIG\n");
			return 1;
		}
		edge.param1 = atoi(sep);
		sep = strtok(NULL,"\n");
		edge.param2 = atoi(sep);
		if(sep == NULL){
			printf("=> ERROR IN FILE CONFIG\n");
			return 1;
		}
		edge.performance = 1;
		if(edge.param1 <= edge.param2){
			edge.thread1 = 1;
			edge.thread2 = 0;
		}
		else{
			edge.thread1 = 0;
			edge.thread2 = 1;
		}
		edge.thread = 0;
		edge.maintenance = 0;
		edge.tasks_completed = 0;
		mem->edge_servers[i] = edge;
	}
	return 0;

}



void init(){
	sem_init(&mem->sem_task, 1, 1);
  	sem_task = &mem->sem_task;
	sem_init(&mem->sem_sched, 1, 0);
  	sem_sched = &mem->sem_sched;
	sem_init(&mem->sem_log, 1, 1);
  	sem_log = &mem->sem_log;
	sem_init(&mem->sem_disp, 1, mem->edge_servers_num);
  	sem_disp = &mem->sem_disp;
	sem_init(&mem->sem_monitor, 1, 0);
  	sem_monitor = &mem->sem_monitor;
	sem_init(&mem->sem_monitor2, 1, 0);
  	sem_monitor2 = &mem->sem_monitor2;
	sem_init(&mem->sem_ready, 1, 0);
  	sem_ready = &mem->sem_ready;
	sem_init(&mem->sem_end, 1, 0);
  	sem_end = &mem->sem_end;
	sem_init(&mem->sem_vcpu, 1, 0);
  	sem_vcpu = &mem->sem_vcpu;
}



void exit_offloading(){
	sem_destroy(sem_task);
	sem_destroy(sem_log);
	sem_destroy(sem_sched);
	sem_destroy(sem_disp);
	sem_destroy(sem_monitor);
	sem_destroy(sem_monitor2);
	sem_destroy(sem_ready);
	sem_destroy(sem_end);
	sem_destroy(sem_vcpu);
  	shmctl(shmid, IPC_RMID, NULL);
	msgctl(mqid, IPC_RMID, 0);
}



void systemManager_f(char config_name[200]){
	FILE *fp;
	fp = fopen(config_name,"r");

	if(fp == NULL){
		printf("=> FILE DOES NOT EXIST\n");
		return;
	}
	//Criar memória partilhada

	shmid = shmget(IPC_PRIVATE, sizeof(mem_structure), IPC_CREAT | 0700);
	if (shmid < 0) {
		perror("Erro ao criar segmento de memoria partilhada.\n");
		exit(0);
	}

	mem = (mem_structure*)shmat(shmid, NULL, 0);
	if (mem < 0) {
		perror("Error ao fazer attach");
		exit(0);
	}

	//ler o ficheiro config
	if(fileread(config_name,fp)){
		exit_offloading();
		return;
	};

	init();

	printf("--------------------------------OFFLOAD SIMULATOR STARTING-----------------------------\n");
    escreve_log("OFFLOAD SIMULATOR STARTING\n");
    printf("=> SHARED MEMORY CREATED\n");
    escreve_log("SHARED MEMORY CREATED\n");

	if(mem->edge_servers_num < 2){
		printf("=> NOT ENOUGH EDGE SERVERS\n");
    	escreve_log("NOT ENOUGH EDGE SERVERS\n");
		exit_offloading();
		return;

	}


    //Criacao do named pipe

	if ((mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0600) < 0) && (errno != EEXIST)) {
		perror("Cannot create pipe: ");
		exit(0);
	}


	//Criacao da message queue

	mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777);
	if (mqid < 0)
	{
		perror("Creating message queue");
		exit(0);
	}
	printf("=> MESSAGE QUEUE CREATED\n");
	escreve_log("MESSAGE QUEUE CREATED\n");
	//Criar processos Task Manager, MaintenanceManager e Monitor

	TaskManager = fork();

	if(TaskManager == 0){
		TaskManager_f();
		exit(0);
	}

	MaintenanceManager = fork();
	if(MaintenanceManager == 0){
		MaintenanceManager_f();
		exit(0);
	}

	Monitor = fork();
	if(Monitor == 0){
		Monitor_f();
		exit(0);

	}


	signal(SIGINT,sigint);
	signal(SIGTSTP,stats);

	wait(NULL);

	kill(Monitor,SIGKILL);
	kill(MaintenanceManager,SIGKILL);
	stats(1);
	char mess[100] = "SIMULATOR CLOSING";
	escreve_log(mess);
	printf("=> SIMULATOR CLOSING...\n");
	exit_offloading();
	sleep(2);
}