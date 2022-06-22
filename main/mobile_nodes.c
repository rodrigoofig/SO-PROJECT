//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687

#include "functions.h"

int main(int argc, char * argv[])
{


    /* create the FIFO (named pipe) */
    fd = open(PIPE_NAME, O_WRONLY);
   
    int param1 = atoi(argv[1]);
    int param2 = atoi(argv[2]);
    int param3 = atoi(argv[3]);
    int param4 = atoi(argv[4]);
    if (fork() == 0)
    {


        char send[200];
        char id[10];
        char p3[10];
        char p4[10];
        if(argc  == 5){
            for (int i = 0; i < param1; i++) {
                int my_id = getpid();
                sprintf(id, "%d", my_id * 1000 + i);
                strcpy(send, id);
                strcat(send, ":");
                sprintf(p3, "%d", param3);
                strcat(send, p3);
                strcat(send, ":");
                sprintf(p4, "%d", param4);
                strcat(send, p4);

                write(fd, send, sizeof(send));
                usleep(param2*1000);

            }
        }
        else if (argc<5){
            printf("Not enough arguments\n");
        }
        else{
            printf("too many arguments\n");
        }



        exit(0);
    }

    wait(NULL);
    close(fd);
    return 0;
}