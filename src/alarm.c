#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#ifndef CLOCK
#include "alarm.h"
#include <signal.h>
#include "clock.h"
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

const char LIGHTS_LIGHT = 1;
const char LIGHTS_ON = 2;
const char LIGHTS_OFF=  0;
int LIGHT_LEVEL;

String ALERT_DIR;
String PLAYER = "vlc-wrapper";
int port = 4321;
String RUN_MUSIC;
int initalarm() {
    DIR *dir;
    struct dirent *ent;
    String dirname = getenv("TWORK_ALERT");

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

<<<<<<< HEAD
=======
    RUN_MUSIC = cat(4, PLAYER, " -ZL ", ALERT_DIR,  "/*");
>>>>>>> 9f01479... light alarm
    return SUCCESS;
}

void localalert () {
<<<<<<< HEAD
    String args[] = { PLAYER, "-ZL", ALERT_DIR, "/*", (char *)NULL};
=======
  //TODO use default if vlc isn't installed.
>>>>>>> 9f01479... light alarm
    int link[2];
    pid_t pid;

    if (pipe(link) == -1) {
        error("alarm: failed to pipe");
        return;
    }

    if ((pid = fork()) == -1) {
        error("alarm: failed to fork");
        return;
    }

    if (pid == 0) {
        if (dup2(link[1], STDOUT_FILENO) == -1) {
            error(cat(3, "dup2 pipe failed with error: ", strerror(errno), " \n"));
            return;
            }
        printf("from child!!\n");
        assert(close(link[0]) != -1);
        assert(close(link[1]) != -1);
    
        system("amixer -D pulse sset Master unmute; amixer -D pulse sset Master 150%");
        if (system(RUN_MUSIC) < 1) {
            error(cat(5, "failed to run track", ALERT_DIR,
                      "\nErr: ", strerror(errno) ,"\n"));
            return;
        }
    }
}

char *hostname() {
  // map hardcoded mac to it's ip addr.
  return "192.168.2.100";
}

void lights_flag(int type) {
  int sockfd, err;
  char buffer[1];
  sprintf(&buffer[0], "%d", type);
  struct hostent *server = gethostbyname(hostname());
  struct sockaddr_in *saddr = malloc(sizeof(struct sockaddr_in));
  saddr->sin_family = AF_INET;
  saddr->sin_port = htons(port);
  memcpy((void*) &saddr->sin_addr.s_addr,
         (void*) server->h_addr,
         server->h_length);
  
  
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    error("socked connection failed");
    return;
  }
  
  if ((err = connect(sockfd,
                     (struct sockaddr*) saddr,
                     sizeof(struct sockaddr_in))) < 0)  {
    printf("connection failed, errno %d, message %s, is ECONNREFUSED %d\n", errno, strerror(errno), errno==ECONNREFUSED);
  }
  
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
    localalert();
    networkalert();
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
    } else {
        error("wrong state!");
    }
}
