#include "parsecommand.h"
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
                free(stages[i]->argv[j]);
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
  wait(NULL);
  printf("\n");
}

int main(int argc, char *argv[]) {

    FILE *commands;
    struct stage **stages;
    /*signal initializations*/
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    if(argc == 1) {
        commands = stdin;
    } else {
        commands = fopen(argv[1],"r");
    }

    while(!feof(commands)) {
        if(isatty(fileno(commands)) && isatty(STDOUT_FILENO)) {
            printf("8-P ");
            fflush(stdout);
        }

        stages = readline(commands);
        if(stages) {
            printstages(stages);
        } else {
            printf("error parsing command or end of file\n");
        }
    }

}
