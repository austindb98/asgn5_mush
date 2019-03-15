#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parsecommand.h"

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
            return cd(stages[numstages]);
        } else if(!strcmp(stages[numstages]->cmd,"exit")) {
            fprintf(stderr,"Exiting from execstages\n");
            return 100;
        }
    }

    int pipelist[numstages][2];

    int i = 0;
    for(i = 0; i < numstages; i++) {
        pipe(pipelist[i]);
    }


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

        if(stages[i]->pipein) {
            dup2(pipelist[i-1][0],fileno(stdin));
        }
        if(stages[i]->pipeout) {
            dup2(pipelist[i][1],fileno(stdout));
        }
        execvp(stages[i]->cmd,(char *const *)stages[i]->argv);
        fprintf(stderr, "%d ", errno);
        perror(stages[i]->cmd);
        return -1;
    } else if(pid < 0) {
        perror("fork");
        return -1;
    } else {
        for(i = 0; i < numstages; i++) {
            int j;
            for(j = 0; j < 2; j++) {
                close(pipelist[i][j]);
            }
        }
        return 0;
    }
}
