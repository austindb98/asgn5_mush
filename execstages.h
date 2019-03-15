#ifndef EXEC
#define EXEC
#include "parsecommand.h"
#include "mush.h"

int cd(struct stage *cdstage);
int execstages(struct stage **stages);
#endif
