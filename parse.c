#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/*a function that parses through the line
  and sets the stages attributes*/
void parseCommand(struct cmd *stage, char **bufPointer, 
				  char **bufPointerO, int *stageNum, 
				  int pipeOffset){
	char *token = NULL;
	char *input = NULL;
	char *output = NULL;
	char delim[2] = " \0";
	int count = 0;
	int flag = 1;
	/*get the token*/
	token = strtok(*bufPointer, delim);
	while((token != NULL) && (token < (*bufPointer + pipeOffset))){
		/*deep pipeline?*/
		if(*stageNum >= MAXCMDSPIPE){
			fprintf(stderr, "pipeline too deep\n");
			exit(3);
		}
		if(!strcmp(token, "<")){
			/*proper input redirection?*/
			if(input != NULL){
				fprintf(stderr, "%s: bad input redirection\n",
				        *bufPointerO);
				exit(3);
			}
			/*clear token*/
			token = strtok(NULL, delim);
			input = token;
			flag = 0;

		} else if(!strcmp(token, ">")){
			/*proper output redirection?*/
			if(output != NULL){
				fprintf(stderr, "%s: bad output redirection\n",
					    *bufPointerO);
				exit(3);
			}
			/*clear token*/
			token = strtok(NULL, delim);
			output = token;
			flag = 0;
		} else {
			/*does the pipeline have an empty stage?*/
			if(!strcmp(token, "|")){
				fprintf(stderr, "invalid null command\n");
				exit(3);
			}
			/*too many args?*/
			if(count >= MAXARGSCMD){
				fprintf(stderr, "%s: too many arguments",
				        *bufPointerO);
				exit(3);
			}
			if(flag == 1){
				/*set argv then clear token*/
				stage -> argv[count] = token;
				token = strtok(NULL, delim);
				count++;
			}else{
				stage -> argv[count] = token;
				token = strtok(NULL, delim);
			}
		}

	}

	/*set all of the stage attributes*/
	stage -> cmd = *bufPointerO;
	stage -> cmdLen = pipeOffset;
	stage -> output = output;
	stage -> input = input;
	stage -> argc = count;

	/*add to offsets and stageNum*/
	*bufPointerO = *bufPointerO + (pipeOffset + 3);
	*bufPointer = *bufPointer + (pipeOffset + 2);
	(*stageNum)++;

}
/*a function that returns the pipeOffset and -1
  if there aren't any delimiters*/
int getOffset(char *cur, char *delim){
	int offset = 0;
	int len = strlen(delim);

	for(;*cur; cur++){
		if(*cur == delim[0]){
			if(!strncmp(cur, delim, len)){
				return offset;
			}
		}
		offset++;
	}
	return -1;
}
/*a function that returns the number of stages and
  passes the line to parseCommand for processing*/
int parseLine(struct cmd *stages, char *bufPointer,
			  char *bufPointerO){
	int stageNum = 0;
	int flag = 1;
	char * delim = " | \0";
	/*infinate loop*/
	while(flag == 1){
		int pipeOffset = 0;
		int opFlag = 0;
		pipeOffset = getOffset(bufPointer, delim);
		/*if there are no more delimiters it is the
		  last operation and pipeOffset == -1*/
		if(pipeOffset < 0){
			pipeOffset = strlen(bufPointer);
			opFlag = 1;
		}
		/*check it it's a space or null character*/
		if(*bufPointer == '\0'){
			bufPointer++;
			continue;
		}
		/*pass the line to parseCommand for processing*/
		parseCommand(&stages[stageNum], &bufPointer, &bufPointerO,
					 &stageNum, pipeOffset);
		/*if it's the last operation break out of the loop*/
		if(opFlag){
			break;
		}
	}
	return stageNum;
}

/*a function to print all the commands from dumpCmds.
  Contains all the proper formatting*/
void printCmds(struct cmd *stage, int count, int opFlag){
	int i = 0;

	printf("\n");
	printf("--------\n");

	if(count){
		printf("Stage %i: \" ", count);
	}else{
		printf("Stage %i: \"", count);
	}
	printf("%.*s", stage -> cmdLen, stage -> cmd);
	if(opFlag){
		printf(" \"\n");
	}else{
		printf("\"\n");
	}
	printf("--------\n");
	printf("%10s: %s\n", "input", stage->input);
	printf("%10s: %s\n", "output", stage->output);
	printf("%10s: %i\n", "argc", stage->argc);
	printf("%10s: ", "argv");

	while(i < stage->argc -1){
		printf("\"%s\",",stage -> argv[i]);
		i++;
	}
	printf("\"%s\"\n", stage->argv[stage->argc - 1]);

}
/*over-arching print function that gets the input and
  output messages and passes them to printCmds*/
void dumpCmds(struct cmd *stage, int stageNum, int count){
	char in[256] = {"\0"};
	char out[256] = {"\0"};

	if(count){
		if(stage -> input != NULL){
			fprintf(stderr, "ambiguous input\n");
			exit(3);
		}
		sprintf(in, "%s %i", "pipe from stage", count - 1);
		stage -> input = in;
	}else{
		if(stage -> input == NULL){
			strcpy(in, "original stdin");
			stage -> input = in;
		}
	}

	if(count != stageNum - 1){
		if(stage -> output != NULL){
			fprintf(stderr, "ambiguous output\n");
			exit(3);
		}
		sprintf(out, "%s %i", "pipe to stage", count + 1);
		stage -> output = out;
	}else{
		if(stage -> output == NULL){
			strcpy(out, "original stdout");
			stage -> output = out;
		}
	}
	printCmds(stage, count, stageNum - 1 - count);
}



int main(int argc, char **argv){
	struct cmd stages[MAXCMDSPIPE];
	/*Original Command Buffer*/
	char bufO[CLINELEN] = {'\0'};
	char * bufPointerO = (char *)&bufO;
	/*Second Buff Init*/
	char buf[CLINELEN] = {'\0'};
	char * bufPointer = (char *)&buf;
	
	int stageNum = 0;
	int count = 0;

	memset(stages, '\0', MAXCMDSPIPE * sizeof(*stages));

	printf("line: ");
	scanf("%[^\n\r]", buf);
	/*check the line length*/
	if(strlen(buf) > CLINELEN){
		fprintf(stderr, "command too long");
		exit(3);
	/*check for null commands*/
	} else if(strlen(buf) == 0){
		fprintf(stderr, "invalid null command");
		exit(3);
	/*check for empty command*/
	}else if(strlen(buf) == 1 && buf[0] == ' '){
		fprintf(stderr, "Error: no command input");
		exit(3);
	}
	/*copy the buffer into the original command buffer*/
	strcpy(bufO, buf);

	stageNum = parseLine(stages, bufPointer, bufPointerO);
	/*creating each stage and dumping it*/
	for(count = 0; count < stageNum; count++){
		struct cmd *stage = &stages[count];

		dumpCmds(stage, stageNum, count);
	}
	return 0;
}