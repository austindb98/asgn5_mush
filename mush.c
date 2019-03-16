#include "parsecommand.h"
#include "execstages.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAXLEN 256

struct stage **readline(FILE *insource) {
    struct stage **stages = calloc(sizeof(struct stage *), 11);
    char **tokens = calloc(sizeof(char *), 513);

    char **lineptr;
    lineptr = (char **)malloc(sizeof(char *));
    *lineptr = NULL;

    size_t *n = malloc(sizeof(size_t));
    *n = 0;

    char *line;
    char *token;
    int i;

    getline(lineptr, n, insource);

    if(*n > MAXLEN) {
        fprintf(stderr,"line too long\n");
        return NULL;
    }

    line = *lineptr;

    token = strtok(line, " \n");
    for(i = 0; token; i++) {
        tokens[i] = calloc(1,512);
        strcpy(tokens[i], token);
        token = strtok(NULL, " \n");
    }
    free(line);

    if(!tokens[0]) {
        return NULL;
    }
    int err = parsecommand(tokens, stages, 0, 0, 0);
    if(err) {
        return NULL;
    }
    for(i = 0; i < 512; i++) {
        if(tokens[i]) {
            free(tokens[i]);
        }
    }
    free(tokens);
    free(lineptr);
    free(n);
    return stages;
}

void freestages(struct stage **stages) {
    int i;
    for(i = 0; stages[i]; i++) {
        int j;
        for(j = 0; j < stages[i]->argc; j++) {
            free(stages[i]->argv[j]);
        }
        free(stages[i]->argv);
        free(stages[i]);
    }
    free(stages);
}

/*a function to block interrupt signals*/
void blocksignals() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    /*create the sigprocmask to block signals*/
    if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("SIGBLOCK");
        exit(3);
    }
}
/*a function to unblock interrupt signals*/
void unblocksignals() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    /*create the sigprocmask to unblock signals*/
    if(sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        perror("SIGUNBLOCK");
        exit(3);
    }
}

void int_handler(int signum) {
    /*wait around*/
    ;
}

int main(int argc, char *argv[]) {
    FILE *commands;
    struct stage **stages;
    struct sigaction sa;

    /*initialize signal variables*/
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    unblocksignals();

    if(argc == 1) {
        commands = stdin;
    } else {
        commands = fopen(argv[1],"r");
    }

    while(!feof(commands)) {
        if(isatty(fileno(stdin)) && isatty(fileno(stdout))) {
            printf("8-P ");
            fflush(stdout);
        }

        stages = readline(commands);

        if(stages) {
            execstages(stages);
            freestages(stages);
        } else if(ferror(commands)) {
            clearerr(commands);
        }



        while(wait(NULL) > 0) {
            //write(STDERR_FILENO,"Waiting for children\n",21);
        }
    }
    exit(0);
}
