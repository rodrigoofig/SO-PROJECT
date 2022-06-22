//Luís Carlos Lopes Loureiro Nº2018297934
//Rodrigo Santos Figueiredo Nº2020236687
#include "functions.h"


int main(int argc, char *argv[])
{

    pid_t systemManager;

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    
    systemManager = fork();
    
        
    if(argc == 2){
        if (systemManager == 0){
            systemManager_f(argv[1]);

            exit(0);

        }
    }
    else{
        printf("give a valid file name\n");
    }    
        

    wait(NULL);
    return 0;

}