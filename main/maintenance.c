//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687

#include "functions.h"

void MaintenanceManager_f(){
	printf("=> PROCESS MAINTENANCE MANAGER CREATED\n");
    escreve_log("PROCESS MAINTENANCE MANAGER CREATED\n");
	while(1){		
		sleep(rand()%5 + 1);
		msg_queue my_msg;
		my_msg.timeStop = rand()%5 +1 ;
		my_msg.mtype = rand()%mem->edge_servers_num + 1;
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

void * maintenance_aux(void * index){
	int idx = *((int*)index);

	msg_queue my_msg;

	while(1){
		char mess[100];
		msgrcv(mqid, &my_msg, sizeof(msg_queue), idx + 1, 0);
		printf("=> %s: MESSAGE RECEIVED. HAS TO STOP\n",mem->edge_servers[idx].name);
		
		strcpy(mess,  mem->edge_servers[idx].name);
		strcat(mess, " STATE HAS CHANGED TO STOPPED\n");
		escreve_log(mess);
		memset(mess, 0 , strlen(mess));

		mem->edge_servers[idx].performance = 0;
		pthread_mutex_lock(&mutex4);
		while(((mem->edge_servers[idx].thread1 == 0 && mem->edge_servers[idx].thread2 == 0) && mem->state==1) || ((mem->edge_servers[idx].thread1 == 0 || mem->edge_servers[idx].thread2 == 0) && mem->state==2)){
			pthread_cond_wait(&cond4,&mutex4);
		}

		mem->edge_servers[idx].thread1 = 0;
		mem->edge_servers[idx].thread2 = 0;
		pthread_mutex_unlock(&mutex4);
		my_msg.mtype = mem->edge_servers_num + 1;
		strcpy(my_msg.server_activated,mem->edge_servers[idx].name);
		msgsnd(mqid, &my_msg, sizeof(msg_queue), 0);
		msgrcv(mqid, &my_msg, sizeof(msg_queue), idx + 1,  0);
		mem->edge_servers[idx].maintenance++;
		if(mem->state == 1){
			mem->edge_servers[idx].performance = 1;
			if(mem->edge_servers[idx].param1 < mem->edge_servers[idx].param2){
				mem->edge_servers[idx].thread1 = 1;
				strcpy(mess,  mem->edge_servers[idx].name);						
				strcat(mess, " STATE HAS CHANGED TO NORMAL\n");
				escreve_log(mess);
				memset(mess, 0 , strlen(mess));
			}
			else{
				mem->edge_servers[idx].thread2 = 1;
				strcpy(mess,  mem->edge_servers[idx].name);						
				strcat(mess, " STATE HAS CHANGED TO HIGH\n");
				escreve_log(mess);
				memset(mess, 0 , strlen(mess));
			}
		}
		else{
			mem->edge_servers[idx].performance = 2;
			mem->edge_servers[idx].thread1 = 1;
			mem->edge_servers[idx].thread2 = 1;
		}
		sem_post(sem_disp);
		if(mem->state == 2){
			sem_post(sem_disp);
		}
		sem_post(sem_vcpu);
	}
	

}