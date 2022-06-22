//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687



#include "functions.h"




void TaskManager_f(){
	pid_t edges[mem->edge_servers_num];
	printf("=> PROCESS TASK MANAGER CREATED\n");
    escreve_log("PROCESS TASK MANAGER CREATED\n");
	sleep(1);

	pthread_mutex_init(&mutex3, NULL);
	pthread_cond_init(&cond3,NULL);

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
	while(read(fd, buf, MAX_BUF) != -1){
		if(strlen(buf) > 3){
			if(strcmp(buf,"EXIT\n") == 0){
                printf("=> TASK MANAGER: RECEIVED COMMAND: %s\n", buf);
                close(fd);
                break;
            }
            if(strcmp(buf,"STATS\n") == 0){
                printf("=> TASK MANAGER: RECEIVED COMMAND: %s\n", buf);
                stats(1);
            }
			else{
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
		memset(buf,0,strlen(buf));
	}
	//-------------------------------------------------------------------
	//função que da print das tarefas que nao foram executadas.
	pthread_cancel(scheduler);
	pthread_cancel(dispatcher);
	pthread_cancel(aux);
	while(tasks.num_tasks>0){
		printf("=>DELETING TASK: %d\n", tasks.task_array[0].ID);
		leftshift();
	}

	while(check()){
		sem_wait(sem_vcpu);
	}
	printf("=> EDGE SERVERS CLOSING...\n");
	for(int i=0;i<mem->edge_servers_num;i++){
		kill(edges[i],SIGKILL);
	}

	for(int i=0;i<mem->edge_servers_num;i++){
		wait(NULL);
	}
	printf("=> TASK MANAGER CLOSING...\n");	
}


void* scheduler_f(){
	
	printf("=> THREAD SCHEDULER CREATED\n");
	escreve_log("THREAD SCHEDULER CREATED\n");
	printf("=> SCHEDULER: WAITING FOR TASKS TO BE ADDED\n");
	while(1){
		sem_wait(sem_sched);
		sortarray();
		printf("=> SCHEDULER: EVALUATION COMPLETED\n");
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
			mem->response_totalTime += timenow- (tasks.task_array[0].time_to_be_completed - tasks.task_array[0].max_time);
			leftshift();
			sem_wait(sem_ready);
			if((double) tasks.num_tasks/mem->queue_pos <= 0.2 && mem->state == 2){
				
				sem_post(sem_monitor2);
			}
		}
		else{
			char mess[100];
			char inttochar[10];
			printf("=> DISPATCHER: TASK %d DELETED\n",tasks.task_array[0].ID);
			strcpy(mess, "=> DISPATCHER: TASK ");
			sprintf(inttochar, "%d",tasks.task_array[0].ID );
			strcat(mess, inttochar);
			strcat(mess, " DELETED\n");
			escreve_log(mess);

			leftshift();
			mem->deleted_tasks++;
			sem_post(sem_disp);
		}
	}

	pthread_exit(NULL);
	return NULL;
}

void removetask(){
    int timenow;
    int index;
	int tasks_removed = 1;
	char removed[100] = "SCHEDULER: TASK ";
	char aux[10];
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
				sprintf(aux, "%d",tasks.task_array[i].ID );
				strcat(removed, aux);
				strcat(removed, " DELETED FROM THE QUEUE");
				escreve_log(removed);
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

void stats_task(){
	printf("----------------- SHOWING STATS -------------------\n");
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
	printf("=> AVERAGE TIME IN THE QUEUE: %f\n", (double)mem->response_totalTime/count);
}
