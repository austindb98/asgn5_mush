struct stage{
    char cmd[512];
    char fullstage[512];
    char in[512];
    char out[512];
    int argc;
    char argv[10][512];
};
int parsecommand(char **tokens, struct stage **stages,
        int pos, int stage, int pipe);
