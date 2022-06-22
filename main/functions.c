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



void* vCPUs_f(void * arg){
	sleep(1);
	vcpu_struct vcpu = *((vcpu_struct *)arg);
	int speed = vcpu.param;
	char * server_name = vcpu.name;
	char speed_char[10];
	sprintf(speed_char,"%d",speed);
	char log_output[200];
	strcpy(log_output,"vCPU WITH SPEED ");
	strcat(log_output,speed_char);
	strcat(log_output, " FROM SERVER ");
	strcat(log_output, server_name);
	strcat(log_output," created\n");
	escreve_log(log_output);
	printf("=> %s",log_output);

	while(1){
		pthread_mutex_lock(&mutex2);
		while(mem->edge_servers[vcpu.i].thread != vcpu.id || mem->edge_servers[vcpu.i].thread == 0){
			pthread_cond_wait(&cond2,&mutex2);
			//printf("thread %d from %d\n",mem->edge_servers[vcpu.i].thread,vcpu.i);
			//printf("vcpu id %d from %d\n",vcpu.id,vcpu.i);
		}
		double time_used;
		if(vcpu.id == 1){
			time_used = (double) (instructions1/(speed*1000));
			//printf("IM 1 GOING TO EXECUTE\n");
		}
		else{
			if(vcpu.id == 2){
				time_used = (double) (instructions2/(speed*1000));
				//printf("IM 2 GOING TO EXECUTE\n");
			}
		}
		pthread_mutex_unlock(&mutex2);
		for(int i=1;i<=3;i++){
			sleep(time_used/3);
			printf("=> vCPU %d FROM %s WORKING ON TASK...[%d%%] Complete\n",vcpu.id,server_name,i*33);
		}
		mem->edge_servers[vcpu.i].thread = 0;
		if(vcpu.id == 1){
			printf("=> %s: TASK %s COMPLETED\n",server_name,task1);
		}
		if(vcpu.id == 2){
			printf("=> %s: TASK %s COMPLETED\n",server_name,task2);
		}
		if(vcpu.id == 1 && ((mem->edge_servers[vcpu.i].param1 > mem->edge_servers[vcpu.i].param2 && mem->state == 2) || (mem->edge_servers[vcpu.i].param1 < mem->edge_servers[vcpu.i].param2 && mem->state == 1))){
			mem->edge_servers[vcpu.i].thread1 = 1;
			sem_post(sem_disp);
		}
		if(vcpu.id == 2 && ((mem->edge_servers[vcpu.i].param2 > mem->edge_servers[vcpu.i].param1 && mem->state == 2) || (mem->edge_servers[vcpu.i].param2 < mem->edge_servers[vcpu.i].param1 && mem->state == 1))){
			mem->edge_servers[vcpu.i].thread2 = 1;
			sem_post(sem_disp);
		}
		mem->edge_servers[vcpu.i].tasks_completed++;
		pthread_cond_signal(cond);
	}
	pthread_exit(NULL);
	return NULL;

}


void * maintenance_aux(void * index){
	int idx = *((int*)index);

	msg_queue my_msg;

	while(1){
		msgrcv(mqid, &my_msg, sizeof(msg_queue), idx + 1, 0);
		printf("=> %s: MESSAGE RECEIVED. HAS TO STOP\n",mem->edge_servers[idx].name);
		mem->edge_servers[idx].performance = 0;
		sem_wait(sem_disp);
		while(((mem->edge_servers[idx].thread1 == 0 && mem->edge_servers[idx].thread2 == 0) && mem->state==1) || ((mem->edge_servers[idx].thread1 == 0 || mem->edge_servers[idx].thread2 == 0) && mem->state==2)){
			
		}
		printf("hihi\n");
		mem->edge_servers[idx].thread1 = 0;
		mem->edge_servers[idx].thread2 = 0;
		my_msg.mtype = mem->edge_servers_num + 1;
		strcpy(my_msg.server_activated,mem->edge_servers[idx].name);
		msgsnd(mqid, &my_msg, sizeof(msg_queue), 0);
		msgrcv(mqid, &my_msg, sizeof(msg_queue), idx + 1,  0);
		mem->edge_servers[idx].maintenance++;
		if(mem->state == 1){
			mem->edge_servers[idx].performance = 1;
			if(mem->edge_servers[idx].param1 < mem->edge_servers[idx].param2){
				mem->edge_servers[idx].thread1 = 1;
			}
			else{
				mem->edge_servers[idx].thread2 = 1;
			}
		}
		else{
			mem->edge_servers[idx].performance = 2;
			mem->edge_servers[idx].thread1 = 1;
			mem->edge_servers[idx].thread2 = 1;
		}
		sem_post(sem_disp);
	}
	

}

void edge_server_f(char *nome,int param1, int param2,int ufd[]){

	pthread_mutex_init(&mutex2, NULL);
	pthread_cond_init(&cond2,NULL);

	sem_init(&sem_vcpu,1,0);

	int index = 0;
	for(int i=0;i<mem->edge_servers_num;i++){
		if(strcmp(mem->edge_servers[i].name,nome) == 0){
			index = i;
			break;
		}
	}

	mem->edge_servers[index].thread = 0;
	printf("=> %s READY\n",nome);
	char log_output[200];
	strcpy(log_output,nome);
	strcat(log_output," READY\n");
    escreve_log(log_output);

	char buf[100];

	close(ufd[1]);

	//Criar as threads vCPUs

	pthread_t vCPUs[2];
	pthread_t main_aux;
	vcpu_struct vcpu1;
	vcpu_struct vcpu2;
	vcpu1.param = param1;
	vcpu1.i = index;
	vcpu1.id = 1;
	strcpy(vcpu1.name,nome);
	strcpy(vcpu2.name,nome);
	vcpu2.param = param2;
	vcpu2.i = index;
	vcpu2.id = 2;

	pthread_create(&vCPUs[0],NULL,vCPUs_f,&vcpu1);
	pthread_create(&vCPUs[0],NULL,vCPUs_f,&vcpu2);
	pthread_create(&main_aux,NULL,maintenance_aux,&index);

	while(read(ufd[0],&buf,sizeof(buf)) > 0){
		char *piece = strtok(buf,"/");
		piece = strtok(NULL,"/");
		char aux[5];
		strcpy(aux,piece);
		piece = strtok(NULL,"\0");
		printf("=> RECEIVED BY %s: vCPU %s WILL EXECUTE TASK %s WITH %s MIL INSTRUCTIONS\n",nome, aux, piece, buf);
		if(atoi(aux) == 1){
			instructions1 = atoi(buf);
			mem->edge_servers[index].thread = 1;
			//printf("YOU CAN GO %d\n",mem->edge_servers[index].thread);
			mem->edge_servers[index].thread1 = 0;
			strcpy(task1,piece);
		}
		else{
			if(atoi(aux) == 2){
				instructions2 = atoi(buf);
				mem->edge_servers[index].thread = 2;
				//printf("YOU CAN GO %d\n",mem->edge_servers[index].thread);
				mem->edge_servers[index].thread2 = 0;
				strcpy(task2,piece);
			}
		}
		pthread_cond_broadcast(&cond2);
		sem_post(sem_ready);
	}
	printf("BYE4");
	for(int i=0;i<2;i++){
		pthread_join(vCPUs[i],NULL);
	}
	pthread_join(main_aux,NULL);
}


void removetask(){
    int timenow;
    int index;
	int tasks_removed = 1;
	while(tasks_removed != 0){
		tasks_removed = 0;
		for(int i = 0; i < tasks.num_tasks ; i++){
			time(&end);
			timenow = end - start; 
			if(timenow > tasks.task_array[i].time_to_be_completed){
				index = i;
				for(int j = index; j + 1 < mem->queue_pos && i < tasks.num_tasks ; j++){
					tasks.task_array[j] = tasks.task_array[j+1];
				}
				tasks_removed++;
				tasks.num_tasks--;
				mem->deleted_tasks++;
			}
		}
	}
    
}

void sortarray(){
	removetask();
    task aux;
    for(int i = 1; i < tasks.num_tasks;i++){
        while(i > 0 && tasks.task_array[i-1].time_to_be_completed > tasks.task_array[i].time_to_be_completed){
            aux = tasks.task_array[i-1];
            tasks.task_array[i-1] = tasks.task_array[i];
            tasks.task_array[i] = aux;
            i--;
        }
    }
}

void* scheduler_f(){
	
	printf("=> THREAD SCHEDULER CREATED\n");
	escreve_log("THREAD SCHEDULER CREATED\n");
	printf("=> SCHEDULER: WAITING FOR TASKS TO BE ADDED\n");
	while(1){
		sem_wait(sem_sched);
		sortarray();
		printf("=> SCHEDULER: EVALUATION COMPLETED\n");
		printf("NUM TASKS: %d\n",tasks.num_tasks);
		//sem_post(sem_task);
		for(int i=0;i<tasks.num_tasks;i++){
			printf("ID: %d\n",tasks.task_array[i].ID);
		}
		pthread_cond_signal(&cond3);

	}
	pthread_exit(NULL);
	
	return NULL;
}

void leftshift(){
    for(int i = 0; i < tasks.num_tasks - 1 ; i++){
        tasks.task_array[i] = tasks.task_array[i+1];
    }
    tasks.num_tasks--;
}


void* dispatcher_f(void *arg){
	pipe_struct ufd = *((pipe_struct *)arg);

	printf("=> THREAD DISPATCHER STARTED\n");
	escreve_log("THREAD DISPATCHER CREATED\n");
	
	for(int i=0;i<mem->edge_servers_num;i++){
		close(ufd.pipes[i][0]);
	}

	int timenow;
	while(1){
		while(tasks.num_tasks == 0){
			pthread_cond_wait(&cond3,&mutex3);
		}
		sem_wait(sem_disp);
		int max = tasks.task_array[0].time_to_be_completed;
		int instr = tasks.task_array[0].num_instrucoes;
		char taskid[100];
		sprintf(taskid,"%d",tasks.task_array[0].ID);
		int condicao = 0;
		int index = 0;
		int breakpoint = 1;
		char vcpu[3];
		strcpy(vcpu,"/2");
		time(&end);
		timenow = end - start;
		for(int i=0;i<mem->edge_servers_num;i++){
			int param1 = mem->edge_servers[i].param1*1000;
			int param2 = mem->edge_servers[i].param2*1000;
			double time_in_vcpu1 = (double) instr/param1;
			double time_in_vcpu2 = (double) instr/param2;
			if((max <= time_in_vcpu1 + timenow || mem->edge_servers[i].thread1 == 0) && (max <= time_in_vcpu2 + timenow || mem->edge_servers[i].thread2 == 0)){
				condicao++;
			}
			else{
				if(breakpoint){
					index = i;
					breakpoint = 0;
					if((max > time_in_vcpu1 + timenow) && (mem->edge_servers[i].thread1 == 1)){
						strcpy(vcpu, "/1");
					}
				}
			}
		}
		if(condicao < mem->edge_servers_num){ //um deles pode executar
			char text[100];
			sprintf(text, "%d", instr); 
			strcat(text,vcpu);
			strcat(text,"/");
			strcat(text,taskid);
			printf("=> DISPATCHER: TASK %d SENT TO %s\n",tasks.task_array[0].ID,mem->edge_servers[index].name);
			write(ufd.pipes[index][1],&text,sizeof(text));
			leftshift();
			sem_wait(sem_ready);
			if((double) tasks.num_tasks/mem->queue_pos <= 0.2 && mem->state == 2){
				printf("hi\n");
				sem_post(sem_monitor2);
			}
		}
		else{
			printf("=> DISPATCHER: TASK %d DELETED\n",tasks.task_array[0].ID);
			leftshift();
			mem->deleted_tasks++;
			sem_post(sem_disp);
		}
	}

	pthread_exit(NULL);
	return NULL;
}


int check(){
	int cond = 0;
	for(int i=0;i<mem->edge_servers_num;i++){
		if((mem->edge_servers[i].thread1 == 1 || mem->edge_servers[i].thread2 == 1) && mem->state == 1){
			cond++;
		}
	}
	if(cond == mem->edge_servers_num){
		return 0;
	}
	else{
		return 1;
	}
}

void* aux_f(void* arg){
	sem_wait(sem_end);
	close(fd);
	pthread_exit(NULL);
	return NULL;
}

void TaskManager_f(){
	pid_t edges[mem->edge_servers_num];
	printf("=> PROCESS TASK MANAGER CREATED\n");
    escreve_log("PROCESS TASK MANAGER CREATED\n");
	sleep(1);

	pthread_mutex_init(&mutex3, NULL);
	pthread_cond_init(&cond3,NULL);

	pipe_struct pipes;
	pipes.num_pipes = mem->edge_servers_num;
    pipes.pipes = malloc(sizeof(int *) * 2);
    for (int i = 0; i < pipes.num_pipes; i++) {
        pipes.pipes[i] = malloc(sizeof(int) * 2);
        pipe(pipes.pipes[i]);
    }

	for(int i=0;i<mem->edge_servers_num;i++){
		edges[i] = fork();
		if(edges[i] == 0){
			edge_server_f(mem->edge_servers[i].name,mem->edge_servers[i].param1,mem->edge_servers[i].param2,pipes.pipes[i]);
			exit(0);
		}
	}

	pthread_t scheduler;
	pthread_create(&scheduler,NULL,scheduler_f,NULL);
	pthread_t dispatcher;
	pthread_create(&dispatcher,NULL,dispatcher_f,&pipes);
	pthread_t aux;
	pthread_create(&aux,NULL,aux_f,NULL);
	//---------------------------------------------------------------------
	//lê comandos do named pipe
	char buf[MAX_BUF];
	tasks.num_tasks = 0;
	fd = open(PIPE_NAME, O_RDONLY);
	time(&start);
	while(1){
		memset(buf,0,strlen(buf));
		if(read(fd, buf, MAX_BUF) == -1){
			//função que da print das tarefas que nao foram executadas.
			pthread_cancel(scheduler);
			pthread_cancel(dispatcher);
			while(check()){
				pthread_cond_wait(cond,mutex);
			}
			for(int i=0;i<mem->edge_servers_num;i++){
				kill(edges[i],SIGKILL);
			}
			break;
		}
		if(strlen(buf) > 3){
				//sem_wait(sem_task);
				printf("=> RECEIVED COMMAND FROM NAMED PIPE: %s\n=> PROCESSING REQUEST...\n", buf);
				if(tasks.num_tasks < mem->queue_pos){
					char aux[100];
					task task;
					char *tok = strtok(buf, ":\0");
					task.ID = atoi(tok);
					tok = strtok(NULL, ":\0");
					task.num_instrucoes = atoi(tok);
					tok = strtok(NULL, ":\0");
					time(&end);
					task.time_to_be_completed = atoi(tok) + end - start;
					task.max_time = atoi(tok);
					tasks.task_array[tasks.num_tasks] = task;
					sprintf(aux, "%d", task.ID);
					printf("=> TASK [%s] ADDED TO THE TASKS QUEUE\n",aux);
					tasks.num_tasks++;
					if((double) tasks.num_tasks/mem->queue_pos >= 0.8 && mem->state == 1){
						sem_post(sem_monitor);
					}
					sem_post(sem_sched);
				}
				else{
					printf("%d %d\n",tasks.num_tasks,mem->queue_pos);
					char mess[100];
					strcpy(mess, "=> QUEUE IS FULL\nTASK[");
					char *tok = strtok(buf, ":\0");
					strcat(mess,tok);
					strcat(mess, "] DELETED\n");
					escreve_log(mess);
					printf("%s",mess);
					memset(mess, 0 , strlen(mess));
					mem->deleted_tasks++;
			}
		}
	}
	//-------------------------------------------------------------------

	pthread_join(dispatcher,NULL);
	pthread_join(scheduler,NULL);


	for(int i=0;i<mem->edge_servers_num;i++){
		wait(NULL);
	}

}



void MaintenanceManager_f(){
	printf("=> PROCESS MAINTENANCE MANAGER CREATED\n");
    escreve_log("PROCESS MAINTENANCE MANAGER CREATED\n");



	while(1){
		sleep(15);
		msg_queue my_msg;
		my_msg.timeStop = 15;
		my_msg.mtype = rand()%4 + 1;
		int id = my_msg.mtype;
		msgsnd(mqid, &my_msg, sizeof(msg_queue), 0);
		printf("=> MAINTENANCE: SENT A MESSAGE TO %s\n",mem->edge_servers[my_msg.mtype - 1].name);
		msgrcv(mqid, &my_msg, sizeof(msg_queue), mem->edge_servers_num+1,  0);
		printf("=> MAINTENANCE STARTED\n");
		sleep(my_msg.timeStop);
		printf("=> MAINTENANCE FINISHED\n");
		my_msg.mtype = id;
		msgsnd(mqid, &my_msg, sizeof(msg_queue), 0);
	}

}

void Monitor_f(){
	printf("=> PROCESS MONITOR CREATED\n");
    escreve_log("PROCESS MONITOR CREATED\n");

	while(1){
		sem_wait(sem_monitor);
		mem->state = 2;
		for(int i=0;i<mem->edge_servers_num;i++){
			if(mem->edge_servers[i].performance == 1){
				mem->edge_servers[i].performance = 2;
				if(mem->edge_servers[i].param1 < mem->edge_servers[i].param2){
					mem->edge_servers[i].thread2 = 1;
				}
				else{
					mem->edge_servers[i].thread1 = 1;
				}
			}
		}
		printf("=> MONITOR: PERFORMANCE INCREASED TO: HIGH PERFORMANCE\n");
		for(int i=0;i<mem->edge_servers_num;i++){
			sem_post(sem_disp);
		}
		
		
		sem_wait(sem_monitor2);
		mem->state = 1;
		for(int i=0;i<mem->edge_servers_num;i++){
			if(mem->edge_servers[i].performance == 2){
				mem->edge_servers[i].performance = 1;
				if(mem->edge_servers[i].param1 < mem->edge_servers[i].param2){
					mem->edge_servers[i].thread2 = 0;
				}
				else{
					mem->edge_servers[i].thread1 = 0;
				}
			}
		}
		printf("=> MONITOR: PERFORMANCE DECREASED TO: NORMAL\n");
	}
	
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
	pthread_mutex_init(&mem->mutex, NULL);
	mutex = &mem->mutex;
	pthread_cond_init(&mem->cond,NULL);
	cond = &mem->cond;
}





void exit_offloading(){
	sem_destroy(sem_task);
	sem_destroy(sem_log);
	sem_destroy(sem_sched);
  	shmctl(shmid, IPC_RMID, NULL);
	msgctl(mqid, IPC_RMID, 0);
}

void stats(int signum){
	printf("----------------- SHOWING STATS -----------------\n");
	int count = 0;
	for(int i=0;i<mem->edge_servers_num;i++){
		count += mem->edge_servers[i].tasks_completed;
		printf("---------------------  %s  ------------------\n",mem->edge_servers[i].name);
		printf("---     TOTAL NUMBER OF TASKS COMPLETED: %d      ---\n",mem->edge_servers[i].tasks_completed);
		printf("---  TOTAL NUMBER OF MAINTENANCES COMPLETED: %d  ---\n",mem->edge_servers[i].maintenance);
		printf("---------------------------------------------------\n\n");
	}
	printf("=> TOTAL NUMBER OF COMPLETED TASKS: %d\n",count);
	printf("=> TOTAL NUMBER OF TASKS DELETED: %d\n",mem->deleted_tasks);
}


void sigint(int signum) {
	printf("=> SIGNAL SIGINT RECEIVED\n=> SIMULATOR WAITING FOR LAST TASKS TO FINISH\n");
	sem_post(sem_end);
	kill(Monitor,SIGKILL);
	kill(MaintenanceManager,SIGKILL);
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


	//signal(SIGINT,sigint);
	signal(SIGTSTP,stats);

	for(int i=0;i<3;i++){
		wait(NULL);

	}
	exit_offloading();
	stats(1);
	printf("=> SIMULATOR CLOSING\n");
}