#define CLINELEN 512
#define MAXCMDSPIPE 10
#define MAXARGSCMD 10

struct cmd {
	char *cmd;
	int cmdLen;
	char *input;
	char *output;
	int argc;
	char *argv[MAXCMDSPIPE * MAXARGSCMD];
};