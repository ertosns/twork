#include <stdio.h>

#define END				"\x1b[0m"
#define ANSI_EC_BOLD 	"\x1b[1m"
#define ANSI_EC_BLINK 	"\x1b[5m"
#define ANSI_EC_RED		"\x1b[31;1m"
#define ANSI_EC_YELLOW  "\x1b[33m"
#define ANSI_EC_BLUE	"\x1b[34;1m"
#define DEBUG

int dbgmode;
void error(char * err);
void highlight(char * imp);
void terminal(char *msg);
void db(char *msg);
