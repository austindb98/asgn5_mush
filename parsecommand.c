#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsecommand.h"

/*
Empty string in in/out = stdin/stdout
Otherwise, write to/read from named file
*/

int parsecommand(char **tokens, struct stage **stages,
        int pos, int stage, int pipefromprev) {

    struct stage *curstage = calloc(1,sizeof(struct stage));
    curstage->argv = calloc(11,sizeof(char *));

    curstage->argv[0] = calloc(512,1);
    curstage->argv[11] = NULL;

    if(!tokens[0]) {
        return 1;
    }

    strcpy(curstage->cmd, tokens[pos]);
    strcpy(curstage->argv[0], tokens[pos]);
    curstage->argc = 1;
    curstage->pipein = pipefromprev;
    curstage->pipeout = 0;
    pos++;

    if(stage > 9) {
        fprintf(stderr, "pipeline too deep\n");
        return -1;
    }

    if(strpbrk(curstage->cmd, "|<>")) {
        fprintf(stderr,"invalid null command\n");
        return -2;
    }

    if(pipefromprev) {
        sprintf(curstage->in,"pipe from stage %d", stage-1);
    }

    while(tokens[pos]) {
        if(*(tokens[pos]) == '|') {
            if(strlen(tokens[pos]) > 1) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            if(curstage->out[0]) {
                fprintf(stderr,"%s: ambiguous output\n", curstage->cmd);
                return -3;
            }

            curstage->pipeout = 1;
            stages[stage] = curstage;
            pos++;

            if(!curstage->in[0]) {
                strcpy(curstage->in, "");
            }

            if(!curstage->out[0]) {
                strcpy(curstage->out, "");
            }

            pos = parsecommand(tokens,stages,pos,stage+1,1);
            if(pos < 0) {
                return pos;
            }

        } else if(*(tokens[pos]) == '>') {
            if(strlen(tokens[pos]) > 1) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            if(curstage->out[0]) {
                fprintf(stderr,"%s: bad output redirection\n", curstage->cmd);
                return -4;
            }


            if(strpbrk(tokens[++pos], "|<>")) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            strcpy(curstage->out, tokens[pos++]);

        } else if(*(tokens[pos]) == '<') {
            if(strlen(tokens[pos]) > 1) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            if(curstage->in[0]) {
                fprintf(stderr,"%s: bad input redirection\n", curstage->cmd);
                return -4;
            } else if(pipefromprev) {
                fprintf(stderr,"%s: ambiguous input\n", curstage->cmd);
                return -3;
            }

            if(strpbrk(tokens[++pos], "|<>")) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            strcpy(curstage->in, tokens[pos++]);

        } else {
            curstage->argv[curstage->argc] = calloc(512,1);
            strcpy(curstage->argv[curstage->argc], tokens[pos]);
            curstage->argc += 1;

            if(curstage->argc > 10) {
                fprintf(stderr, "%s: too many arguments\n", curstage->cmd);
                return -1;
            }
            pos++;
        }
    }

    if(!curstage->in[0]) {
        strcpy(curstage->in, "");
    }

    if(!curstage->out[0]) {
        strcpy(curstage->out, "");
    }

    stages[stage] = curstage;
    if(stage != 0) {
        return pos;
    } else {
        return 0;
    }
}
