//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687

#include "functions.h"


int check(){
	int cond = 0;
	for(int i=0;i<mem->edge_servers_num;i++){
		if(((mem->edge_servers[i].thread1 == 1 || mem->edge_servers[i].thread2 == 1) && mem->state == 1) || ((mem->edge_servers[i].thread1 == 1 && mem->edge_servers[i].thread2 == 1) && mem->state == 2)){
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

void stats(int signum){
	char mess[100] = "SIGNAL SIGTSTP RECEIVED\n";
	escreve_log(mess);
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
	printf("=> AVERAGE TIME IN THE QUEUE: %f\n", (double)mem->response_totalTime/count);
}


void sigint(int signum) {
	char mess[100] = "SIGNAL SIGINT RECEIVED\n";
	escreve_log(mess);
	printf("=> SIGNAL SIGINT RECEIVED\n=> SIMULATOR WAITING FOR LAST TASKS TO FINISH\n");
	sem_post(sem_end);
}