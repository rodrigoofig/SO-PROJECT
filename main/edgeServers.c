//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687

#include "functions.h"


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
	strcat(log_output," CREATED\n");
	escreve_log(log_output);
	printf("=> %s",log_output);

	while(1){
		char mess[100];
		
		pthread_mutex_lock(&mutex2);
		while(mem->edge_servers[vcpu.i].thread != vcpu.id || mem->edge_servers[vcpu.i].thread == 0){
			pthread_cond_wait(&cond2,&mutex2);
		
		}
		double time_used;
		if(vcpu.id == 1){
			time_used = (double) (instructions1/(speed*1000));
		
		}
		else{
			if(vcpu.id == 2){
				time_used = (double) (instructions2/(speed*1000));
				
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
			strcat(mess, server_name);
			strcat(mess, ": TASK ");
			strcat(mess, task1);
			strcat(mess, " COMPLETED\n");
			escreve_log(mess);
		}
		if(vcpu.id == 2){
			printf("=> %s: TASK %s COMPLETED\n",server_name,task2);
			strcat(mess, server_name);
			strcat(mess, ": TASK ");
			strcat(mess, task2);
			strcat(mess, " COMPLETED\n");
			escreve_log(mess);
		}
		if(vcpu.id == 1 && ((mem->edge_servers[vcpu.i].param1 > mem->edge_servers[vcpu.i].param2 && mem->state == 2) || (mem->edge_servers[vcpu.i].param1 < mem->edge_servers[vcpu.i].param2 && mem->state == 1))){
			
			mem->edge_servers[vcpu.i].thread1 = 1;
			if(mem->edge_servers[vcpu.i].performance == 0){
				pthread_cond_signal(&cond4);
			}
			else{
				sem_post(sem_disp);
			}
		}
		if(vcpu.id == 2 && ((mem->edge_servers[vcpu.i].param2 > mem->edge_servers[vcpu.i].param1 && mem->state == 2) || (mem->edge_servers[vcpu.i].param2 < mem->edge_servers[vcpu.i].param1 && mem->state == 1))){
			mem->edge_servers[vcpu.i].thread2 = 1;
			if(mem->edge_servers[vcpu.i].performance == 0){
				pthread_cond_signal(&cond4);
			}
			else{
				sem_post(sem_disp);
			}
		}
		mem->edge_servers[vcpu.i].tasks_completed++;
		sem_post(sem_vcpu);
	}
	pthread_exit(NULL);
	return NULL;

}


void edge_server_f(char *nome,int param1, int param2,int ufd[]){

	pthread_mutex_init(&mutex2, NULL);
	pthread_cond_init(&cond2,NULL);

	pthread_mutex_init(&mutex4, NULL);
	pthread_cond_init(&cond4,NULL);

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

	while(read(ufd[0],&buf,sizeof(buf)) != 1){
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
	for(int i=0;i<2;i++){
		pthread_join(vCPUs[i],NULL);
	}
	pthread_join(main_aux,NULL);
}
