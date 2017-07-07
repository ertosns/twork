#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#ifndef CLOCK
#include "alarm.h"
#include <signal.h>
#include "clock.h"
#endif

String ALERT_DIR;
String PLAYER = "vlc-wrapper";

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
    readdir(dir);
    readdir(dir);
    if (!(ent = readdir(dir)))
        ALERT_DIR = NULL;
    else ALERT_DIR=dirname;

    return SUCCESS;
}

void localalert () {
    String args[] = { PLAYER, "-ZL", ALERT_DIR, "/*", (char *)NULL};
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
        if (execvp(PLAYER, args) == -1) {
            error(cat(5, "failed to run track", ALERT_DIR,
                      "\nErr: ", strerror(errno) ,"\n"));
            return;
        }
    }
}

void networkalert () {}

void signalalert (int sig) {
    localalert();
    networkalert();
}

void alert (String name, int secs) {
    if (!ALERT_DIR) {
        error("wrong ALERT_DIR");
        return;
    }

    if (startst(name)) {
        signal(SIGALRM, &signalalert);
        alarm(secs);
    } else {
        error("wrong state!");
    }
}
