//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687

#include "functions.h"

void Monitor_f(){
	printf("=> PROCESS MONITOR CREATED\n");
    escreve_log("PROCESS MONITOR CREATED\n");

	while(1){
		char mess[100];
		sem_wait(sem_monitor);
		mem->state = 2;
		for(int i=0;i<mem->edge_servers_num;i++){
			strcpy(mess, mem->edge_servers[i].name);
			strcat(mess, " STATE HAS CHANGED HIGH\n");
			escreve_log(mess);
			
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
			strcpy(mess, mem->edge_servers[i].name);
			strcat(mess, " STATE HAS CHANGED TO NORMAL\n");
			escreve_log(mess);
			memset(mess, 0 , strlen(mess));
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
