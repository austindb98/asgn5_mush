#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <parsecommand.h>

int cd(struct stage *cdstage) {
    if(cdstage->argc!=2) {
        fprintf(stderr,"usage: cd path/to/directory\n");
        return -1;
    }

    if(!cdstage->argv[1]) {
        fprintf(stderr,"No directory specified\n");
        return -1;
    }

    if(chdir(cdstage->argv[1])) {
        perror("cd");
        return -1;
    } else {
        return 0;
    }
}


int execstages(struct stage **stages) {
    int numstages;
    for(numstages = 0; stages[numstages]; numstages++) {
        if(!strcmp(stages[numstages]->cmd,"cd")) {
            return cd(stages[numstages]);;
        } else if(!strcmp(stages[numstages]->cmd,"exit")) {
            exit(0);
        }
    }

    int pipelist[numstages][2] ;

    int i = 0;
    pid_t pid;

    for(i = 0; stages[i]; i++) {
        if(!(pid = fork())) {
            /*Fork child preserving i*/
            break;
        }
    }

    if(pid == 0) {
        /*child*/

        /*plumb ends of pipes if needed*/

        execvp(stages[i]->cmd,(char *const *)stages[i]->argv);
        perror(stages[i]->cmd);
        return -1;
    } else if(pid < 0) {
        perror("fork");
        return -1;
    } else {
        return 0;
    }
}
