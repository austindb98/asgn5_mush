#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "parsecommand.h"
#include "mush.h"

void closepipes(int pipelist[][2], int numstages) {
    int i;
    for(i = 0; i < numstages; i++) {
        int j;
        for(j = 0; j < 2; j++) {
            close(pipelist[i][j]);
        }
    }
}

int cd(struct stage *cdstage) {
    if(cdstage->argc!=2) {
        /*If no directory specified*/
        fprintf(stderr,"usage: cd path/to/directory\n");
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
    /*Iterate and count stages, check for cd and exit*/
    for(numstages = 0; stages[numstages]; numstages++) {
        if(!strcmp(stages[numstages]->cmd,"cd")) {
            return cd(stages[numstages]);
        } else if(!strcmp(stages[numstages]->cmd,"exit")) {
            exit(0);
        }
    }

    int pipelist[numstages][2];

    int i = 0;
    /*create pipes*/
    for(i = 0; i < numstages; i++) {
        if(-1 == pipe(pipelist[i])) {
            perror("pipe");
            exit(-1);
        }
    }



    /*Block sigints during fork*/
    blocksignals();
    for(i = 0; stages[i]; i++) {
        pid_t pid;
        if((pid = fork()) == 0) {
            /*child*/

            /*Input*/
            if(stages[i]->pipein) {
                dup2(pipelist[i-1][0], STDIN_FILENO);
            } else if(*(stages[i]->in)) {
                int fd = open(stages[i]->in, O_RDONLY);
                dup2(fd, STDIN_FILENO);
            }

            /*Output*/
            if(stages[i]->pipeout) {
                dup2(pipelist[i][1], STDOUT_FILENO);
            } else if(*(stages[i]->out)) {
                int fd = open(stages[i]->out, O_WRONLY|O_CREAT|O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
            }

            closepipes(pipelist,numstages);
            unblocksignals();
            execvp(stages[i]->cmd,(char *const *)stages[i]->argv);
            fprintf(stderr, "%d ", errno);
            perror(stages[i]->cmd);
            exit(errno);
        } else if(pid < 0) {
            closepipes(pipelist,numstages);
            unblocksignals();
            perror("fork");
            exit(errno);
        }

    }
    unblocksignals();
    closepipes(pipelist,numstages);
    return 0;
}
