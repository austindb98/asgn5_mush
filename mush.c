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

    size_t *n = malloc(sizeof(int));
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

    return stages;
}

void printstages(struct stage **stages) {
    int i;
    for(i = 0; stages[i]; i++) {
        printf("--------\n");
        printf("Stage %d: %s\n", i, stages[i]->cmd);
        printf("--------\n");
        printf("     input: %s\n", stages[i]->in);
        printf("    output: %s\n", stages[i]->out);
        printf("      argc: %d\n", stages[i]->argc);
        printf("      argv: \"%s\"", stages[i]->argv[0]);
        if(stages[i]->argc > 1) {
            int j;
            for(j = 1; j < stages[i]->argc; j++) {
                printf(", \"%s\"", stages[i]->argv[j]);
            }
            printf("\n");
        }
        printf("\n");
        free(stages[i]);
    }
    free(stages);
}

void int_handler(int signum){
    /*wait around*/
    write(STDERR_FILENO,"\nSIGINT received\n\n",18);
    while(wait(NULL) > 0) {
        write(STDERR_FILENO,"Waiting for children\n",22);
    }

    if(isatty(fileno(stdin)) && isatty(fileno(stdout))) {
        write(STDOUT_FILENO,"8-P ",4);
        fflush(stdout);
    }
}
void blockSignals() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("SIGBLOCK");
        exit(3);
    }
}

void unblockSignals() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        perror("SIGUNBLOCK");
        exit(3);
    }
}

int main(int argc, char *argv[]) {

    FILE *commands;
    struct stage **stages;
    /*signal initializations*/
    struct sigaction sa;
    sigset_t mask, oldmask;

    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);

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
            if(execstages(stages) == 100) {
                fprintf(stderr,"Exiting from main\n");
                break;
            }
        } else {
            printf("error parsing command or end of file\n");
        }

        while(wait(NULL) > 0) {
            write(STDERR_FILENO,"Waiting for children\n",22);
        }
    }
    exit(0);
}
