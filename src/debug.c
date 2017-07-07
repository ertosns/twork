#include "debug.h"

void error(char * err)
{
	printf("%s %s %s\n", ANSI_EC_RED, err, END);
}

void highlight(char * imp)
{
	printf("%s %s %s\n", ANSI_EC_YELLOW, imp, END);
}

void terminal(char *msg)
{
	printf("%s %s %s\n", ANSI_EC_BLUE, msg, END);
}

void db(char *msg)
{
	if(dbgmode)
		printf("%s %s %s\n", ANSI_EC_YELLOW, msg, END);
}
