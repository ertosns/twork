#include <X11/X.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include "atrack.h"
#ifndef CRUD
#include "crud.h"
#endif
#ifndef LINKABLES
#include "linker.h"
#endif
#include <math.h>

int old_x_root, old_y_root;

const String SESSIONS = "SESSIONS";
//! note that end date is the default date column
const String START_DATE = "START_DATE";
const String LINKABLE_TASK = "LINKABLE_TASK";
const String KEYING_FREQ = "K_FREQ";
const String MOUSING_FREQ = "M_FREQ";
const String WINDOW = "WINDOW";
const String WINDOW_NAME = "WINDOW_NAME";

void initatrack() {
    cursession = malloc(sizeof(session));
    cursession->date = NULL;
    track();
    //get current window, x_root, y_rootP
}

String getwindow_name (Display *disp, Window win) {
    Atom prop = XInternAtom(disp,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *windowName;
    if (XGetWindowProperty(disp,win,prop,0,1024,False,AnyPropertyType,&type,&form,&len,&remain,&windowName) != Success) {
        return NULL;
    }
    return (String)windowName;
}


/*retrieve current window, update WINDOW table*/
String add_window(String window) {
    if (notexist(WINDOW)) {
        Val cols[] = { makeval(WINDOW_NAME, sdt_type) };
        int rc = sqlCreate(WINDOW, cols, 1);
        if (!rc) {
            error("window creation failed!");
            assert(rc);
        }
    }
    Val vals[] = { makeval(window, sdt_type) };
    String cols[] = { WINDOW, WINDOW_NAME };
    int rc = sqlInsert(cols, vals,1 );
    if (!rc)
        error("failed to insert current window");

    return window;
}

//! TODO make sure both tasks&windows are linked to linkable, window tables
/* called upon setting task */
void push_session() {
    if (!cursession->date)
        return;

    if (notexist(SESSIONS)) {
        Val cols[] = { makeval(START_DATE, sdt_date),
                       makeval(LINKABLE_TASK, sdt_type),
                       makeval(WINDOW, sdt_type),
                       makeval(KEYING_FREQ, sdt_number),
                       makeval(MOUSING_FREQ, sdt_number) };
        int rc = sqlCreate(SESSIONS, cols, 5);
        if (!rc) {
            error("session creation failed!");
            return;
        }
    }

    Val vals[] = { makeval(cursession->date, sdt_date),
                   makeval(cursession->task, sdt_type),
                   makeval(cursession->window, sdt_type),
                   makeval(itos(cursession->keying), sdt_number),
                   makeval(itos(cursession->mousing), sdt_number) };
    String cols[] = { SESSIONS, START_DATE, LINKABLE_TASK, WINDOW, KEYING_FREQ, MOUSING_FREQ };
    sqlInsert(cols, vals, 5);
}

/*changing interactive window is terminating event,
  new sessions starts*/
void set_window(String window, String task) {
    add_window(window);
    push_session();

    cursession->date = getDateTime();
    cursession->task = task;
    cursession->window = window;
    cursession->keying = 0;
    cursession->mousing = 0;
}

void inckeying() {
    cursession->keying++;
}

void incmousing(int distance) {
    cursession->mousing+=distance;
}

// other application block events!
int track() {
  pid_t pid = fork();
  String task, window_name;
  XEvent *xevent = malloc(sizeof(XEvent));
  XMotionEvent *xmotion;
  long mask = KeyPressMask|
    ButtonPressMask|
    EnterWindowMask|
    PointerMotionMask;
  Display *display = XOpenDisplay(NULL);
  Window window;
  int distance;
  if (!pid) {
    read_cur_task();
    for(;;) {
      //if (!CUR_TASK)
      //  continue;
      //else task = CUR_TASK;
      XMaskEvent(display, mask, xevent);
      switch(xevent->type) {
      case EnterNotify: {
        window = ((XEnterWindowEvent*)xevent)->window;
        window_name = getwindow_name(display, window);
        if(!window_name) {
          error("nameless window caught!");
          continue;
        }
        set_window(window_name, task);
        break;
      }
      case MotionNotify: {
        xmotion = (XMotionEvent*)xevent;
        distance = (int) sqrt(pow(abs(old_x_root - xmotion->x_root), 2) +
                              pow(abs(old_y_root - xmotion->y_root), 2));
        incmousing(distance);
        break;
      }
      case KeyPress:
      case ButtonPress: {
        inckeying();
        break;
      }
      default: {
        error("undefined event mask!");
      }
      }
    }
  }
  return pid != -1;
}
