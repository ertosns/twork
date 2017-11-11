#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "alarm.h"

#ifndef CLOCK
#include "clock.h"
#endif


const char LIGHTS_LIGHT = 1;
const char LIGHTS_ON = 2;
const char LIGHTS_OFF=  0;
int LIGHT_LEVEL;

String ALERT_DIR;
int port = 4321;

int initalarm() {
    DIR *dir;
    struct dirent *ent;
    String dirname = TWORK_ALERT;

    if (!dirname) {
        error("ALERT DIR isn't valid in env_var ALERT_DIR");
        return FAILED;
    }
    dir= opendir(dirname);
    if (!dir) {
        error(cat(2, "can't open ALERT_DIR ", dirname));
        return FAILED;
    }
    readdir(dir); //.
    readdir(dir); //..
    if (!(ent = readdir(dir))) {
        ALERT_DIR = NULL;
        error("not alert music found!");
        return FAILED;
    }
    else ALERT_DIR=dirname;
    return SUCCESS;
}

void localalert () {
  FILE *pip;
  //dup(1);close(1);
  if (system(TWORK_PLAYER)<1) {
    char *err = readFile(pip);
    error(cat(5, "alarm_directory:", ALERT_DIR, "\nErr:", err,"\n"));
    free(err);
  }
  pclose(pip);
}

//TODO read mac from configuration file, 
char *hostname() {
  if (!IP) {
    /*resolve MAC, update IP from arp*/ 
  }
  return IP;
}

void lights_flag(int type) {
  int sockfd, err;
  char buffer[1];
  sprintf(&buffer[0], "%d", type);
  struct hostent *server;
  struct sockaddr_in *saddr;
  do {
    server = gethostbyname(hostname());
    saddr = malloc(sizeof(struct sockaddr_in));
    saddr->sin_family = AF_INET;
    saddr->sin_port = htons(port);
    memcpy((void*) &saddr->sin_addr.s_addr, (void*) server->h_addr, server->h_length);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      error("socked connection failed");
      return;
    }
  } while (connect(sockfd, (struct sockaddr*) saddr, sizeof(struct sockaddr_in)) < 0);
  write(sockfd, (const void*) &buffer, 1);
}

void mute_lights() {
  //TODO detect task, elongate, muting phase or lazy reacting tasks e.g sleep
  //wait(x);
  lights_flag(LIGHTS_OFF);
}

void networkalert () {
  lights_flag(LIGHT_LEVEL);
}

void signalalert (int sig) {
  networkalert();
  localalert();
}

void alert (String name, int secs, int level) {
  if (!ALERT_DIR) {
    error("wrong ALERT_DIR");
    return;
  }
  LIGHT_LEVEL = level;
  if (startst(name)) {
    signal(SIGALRM, &signalalert);
    alarm(secs);
  } else error("wrong state!");
}
