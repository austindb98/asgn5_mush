#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parseline.h"

/* Error codes
-1: Length of input/command
-2: invalid command
-3: ambiguous input/output
-4: bad redirection
 */

 /*
 Empty string in in/out = stdin/stdout
 Otherwise, write to/read from named file
 */

int parsecommand(char **tokens, struct stage **stages,
        int pos, int stage, int pipefromprev) {
    struct stage *curstage = calloc(1,sizeof(struct stage));

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
            if(curstage->out[0]) {
                fprintf(stderr,"%s: bad output redirection\n", curstage->cmd);
                return -4;
            }


            if(strpbrk(tokens[pos], "|<>")) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            strcpy(curstage->out, tokens[pos++]);

        } else if(*(tokens[pos]) == '<') {
            if(curstage->in[0]) {
                fprintf(stderr,"%s: bad input redirection\n", curstage->cmd);
                return -4;
            } else if(pipefromprev) {
                fprintf(stderr,"%s: ambiguous input\n", curstage->cmd);
                return -3;
            }

            if(strpbrk(tokens[pos], "|<>")) {
                fprintf(stderr,"invalid null command\n");
                return -2;
            }

            strcpy(curstage->in, tokens[pos++]);

        } else {
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
    return pos;
}
