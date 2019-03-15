#ifndef PARSE
#define PARSE
struct stage {
    char cmd[512];
    int pipein;
    char in[512];
    int pipeout;
    char out[512];
    int argc;
    char argv[11][512];
};
int parsecommand(char **tokens, struct stage **stages,
        int pos, int stage, int pipe);
#endif
