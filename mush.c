#include "parseline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 256

struct stage **stageline(FILE *insource) {
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

        if(*n > MAXLEN) {
            fprintf(stderr,"line too long\n");
        }

        line = *lineptr;

        token = strtok(line, " \n");
        for(i = 0; token; i++) {
            tokens[i] = calloc(1,512);
            strcpy(tokens[i], token);
            token = strtok(NULL, " \n");
        }
        free(line);

        parsecommand(tokens, stages, 0, 0, 0);
        for(i = 0; i < 512; i++) {
            if(tokens[i]) {
                free(tokens[i]);
            }
        }
        free(tokens);

        return stages;
}

int main(int argc, char *argv[]) {

    FILE *commands;
    struct stage **stages;

    if(argc == 2) {
        commands = fopen(argv[1], "r");
    }

    if(argc == 1) {
        commands = stdin;
    } else {
        commands = fopen(argv[1],"r");
    }

    while(!feof(commands)) {
        stages = stageline(commands);
    }

}
