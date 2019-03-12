#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parseline.h"

int parsecommand(char **tokens, struct stage **stages,
        int pos, int stage, int pipe) {
    struct stage *curstage = calloc(1,sizeof(struct stage));

    strcpy(curstage->cmd, tokens[pos]);
    strcpy(curstage->argv[0], tokens[pos]);
    strcpy(curstage->fullstage, tokens[pos]);
    curstage->argc = 1;
    pos++;

    if(stage > 9) {
        fprintf(stderr, "pipeline too deep\n");
        exit(EXIT_FAILURE);
    }

    if(strpbrk(curstage->cmd, "|<>")) {
        fprintf(stderr,"invalid null command\n");
        exit(EXIT_FAILURE);
    }

    if(pipe) {
        sprintf(curstage->in,"pipe from stage %d", stage-1);
    }

    while(tokens[pos]) {
        if(*(tokens[pos]) == '|') {
            if(curstage->out[0]) {
                fprintf(stderr,"%s: ambiguous output\n", curstage->cmd);
                exit(EXIT_FAILURE);
            }

            sprintf(curstage->out, "pipe to stage %d", stage + 1);
            stages[stage] = curstage;
            pos++;

            if(!curstage->in[0]) {
                strcpy(curstage->in, "original stdin");
            }

            if(!curstage->out[0]) {
                strcpy(curstage->out, "original stdout");
            }

            pos = parsecommand(tokens,stages,pos,stage+1,1);

        } else if(*(tokens[pos]) == '>') {
            if(curstage->out[0]) {
                fprintf(stderr,"%s: bad output redirection\n", curstage->cmd);
                exit(EXIT_FAILURE);
            }

            strcat(curstage->fullstage, " ");
            strcat(curstage->fullstage, tokens[pos++]);
            strcat(curstage->fullstage, " ");

            if(strpbrk(tokens[pos], "|<>")) {
                fprintf(stderr,"invalid null command\n");
                exit(EXIT_FAILURE);
            }

            strcat(curstage->fullstage, tokens[pos]);
            strcpy(curstage->out, tokens[pos++]);

        } else if(*(tokens[pos]) == '<') {
            if(curstage->in[0]) {
                fprintf(stderr,"%s: bad input redirection\n", curstage->cmd);
                exit(EXIT_FAILURE);
            } else if(pipe) {
                fprintf(stderr,"%s: ambiguous input\n", curstage->cmd);
                exit(EXIT_FAILURE);
            }

            strcat(curstage->fullstage, " ");
            strcat(curstage->fullstage, tokens[pos++]);
            strcat(curstage->fullstage, " ");

            if(strpbrk(tokens[pos], "|<>")) {
                fprintf(stderr,"invalid null command\n");
                exit(EXIT_FAILURE);
            }

            strcat(curstage->fullstage, tokens[pos]);
            strcpy(curstage->in, tokens[pos++]);

        } else {
            strcpy(curstage->argv[curstage->argc], tokens[pos]);
            curstage->argc += 1;

            if(curstage->argc > 10) {
                fprintf(stderr, "%s: too many arguments\n", curstage->cmd);
                exit(EXIT_FAILURE);
            }

            strcat(curstage->fullstage, " ");
            strcat(curstage->fullstage, tokens[pos]);
            pos++;
        }
    }

    if(!curstage->in[0]) {
        printf("in prev: %s\n", curstage->in);
        strcpy(curstage->in, "original stdin");
    }

    if(!curstage->out[0]) {
        strcpy(curstage->out, "original stdout");
    }

    stages[stage] = curstage;
    return pos;
}
