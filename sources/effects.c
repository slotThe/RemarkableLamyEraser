#include <linux/input.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "effects.h"
#include "effects_data.h"
#include "orientation.h"
#include "triggers.h"

// Cries in no API.
#define SLEEP      -1
#define LONG_SLEEP -2
#define HALF_SLEEP -3

void write_event(int fd, struct input_event event) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  event.time = tv;
  // printf("writing: seconds = %ld, usec= %ld, type = %d, code = %d, value = %d\n", event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);
  write(fd, &event, sizeof(struct input_event));
}

static int currentTrackingID;

void handle_current_tracking_ID(struct input_event *event) {
  if (event->type == EV_ABS && event->code == ABS_MT_TRACKING_ID) {
    if (event->value != -1) {
      currentTrackingID = event->value;
      // printf("Grabbing tracking ID: %d\n", currentTrackingID);
    }
  }
}

void write_tap_with_touch(int fd_touch, const int location[2]) {
  struct input_event event;

  event = (struct input_event){.type = EV_SYN, .code = SYN_REPORT, .value = 1};
  // printf("Writing SYN Report\n");
  write_event(fd_touch, event);

  // this is the minimum (probably) seqeunce of events that must be sent to tap
  // the screen in a location.

  // Use slot 15. Should be high enough, right?
  event = (struct input_event){.type = EV_ABS, .code = ABS_MT_SLOT, .value = 15};
  // printf("Writing ABS_MT_SLOT: %d\n", event.value);
  write_event(fd_touch, event);

  event = (struct input_event){.type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = ++currentTrackingID};
  // printf("Writing Tracking ID: %d\n", event.value);
  write_event(fd_touch, event);

  event = (struct input_event){.type = EV_ABS, .code = ABS_MT_POSITION_X, .value = location[0]};
  // printf("Writing Touch X: %d\n", event.value);
  write_event(fd_touch, event);

  event = (struct input_event){.type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = location[1]};
  // printf("Writing Touch Y: %d\n", event.value);
  write_event(fd_touch, event);

  event = (struct input_event){.type = EV_SYN, .code = SYN_REPORT, .value = 1};
  // printf("Writing SYN Report\n");
  write_event(fd_touch, event);

  event = (struct input_event){.type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = -1};
  // printf("Writing Tracking ID: -1\n");
  write_event(fd_touch, event);

  event = (struct input_event){.type = EV_SYN, .code = SYN_REPORT, .value = 1};
  // printf("Writing SYN Report\n");
  write_event(fd_touch, event);
}

void write_oriented_tap(int fd, toolbar_orientation orientation, int action) {
  int     actionLocation[2];
  if (action == SLEEP) {
    usleep(50000);
  } else if (action == HALF_SLEEP) {
    usleep(100000);
  } else if (action == LONG_SLEEP) {
    usleep(550000);
  } else {
    actionLocation[0] = LOCATION_LOOKUP_TOUCH[orientation.doc_type][action]
                                             [orientation.orientation][0];
    actionLocation[1] = LOCATION_LOOKUP_TOUCH[orientation.doc_type][action]
                                             [orientation.orientation][1];
    write_tap_with_touch(fd, actionLocation);
  }
}


int write_oriented_tap_sequence(int fd, int numLocations, ...) {
  int     action;
  va_list actionType;
  va_start(actionType, numLocations);
  toolbar_orientation orientation = get_toolbar_orientation();

  for (int i = 0; i < numLocations; i++) {
    action = va_arg(actionType, int);
    write_oriented_tap(fd, orientation, action);
  }

  va_end(actionType);
  return 0;
}

int write_oriented_tap_sequence_toolbar(int fd, int numLocations, ...) {
  toolbar_orientation orientation = get_toolbar_orientation();

  int action;
  va_list actionType;
  va_start(actionType, numLocations);

  for (int i = 0; i < numLocations; i++) {
    action = va_arg(actionType, int);
    write_oriented_tap(fd, orientation, action);
  }

  printf("%b\n", assumeTBOpen);
  if (!assumeTBOpen) {
    write_oriented_tap(fd, orientation, TOOLBAR);
    write_oriented_tap(fd, orientation, LONG_SLEEP);

    va_start(actionType, numLocations);

    for (int i = 0; i < numLocations; i++) {
      action = va_arg(actionType, int);
      write_oriented_tap(fd, orientation, action);
    }

    write_oriented_tap(fd, orientation, TOOLBAR);
  }

  va_end(actionType);
  return 0;
}

/*-----------------------------------------------------------------
 * Single shot actions: compatible with clicks
 * -----------------------------------------------------------------*/

void action_toolbar(int fd_touch) {
  write_oriented_tap_sequence(fd_touch, 1, TOOLBAR);
}

void action_writing1(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 1, WRITING1);
}

void action_writing2(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 1, WRITING2);
}

void action_text(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 1, TEXT);
}

void action_eraser_panel(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 1, ERASER_PANEL);
}

void action_undo(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 1, UNDO);
}

void action_redo(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 1, REDO);
}

static bool hlOn = false;

void action_fineliner(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 13, SELECT, SLEEP, SELECT, HALF_SLEEP, WRITING1, HALF_SLEEP, WRITING1, LONG_SLEEP, WRITING_FINELINER, SLEEP, WRITING_FINELINER, HALF_SLEEP, WRITING1);
  hlOn = false;
}

void action_calligraphy(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 9, SELECT, HALF_SLEEP, WRITING1, HALF_SLEEP, WRITING1, LONG_SLEEP, WRITING_CALLIGRAPHY, HALF_SLEEP, WRITING1);
  hlOn = false;
}

void action_black(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 9, SELECT, HALF_SLEEP, WRITING1, HALF_SLEEP, WRITING1, LONG_SLEEP, WRITING_BLACK, HALF_SLEEP, WRITING1);
}

void action_grey(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 9, SELECT, HALF_SLEEP, WRITING1, HALF_SLEEP, WRITING1, LONG_SLEEP, WRITING_GREY, HALF_SLEEP, WRITING1);
}

void action_hl(int fd_touch) {
  write_oriented_tap_sequence_toolbar(fd_touch, 13, SELECT, SLEEP, SELECT, HALF_SLEEP, WRITING1, HALF_SLEEP, WRITING1, LONG_SLEEP, WRITING_HL, SLEEP, WRITING_HL, HALF_SLEEP, WRITING1);
  hlOn = true;
}

/*-----------------------------------------------------------------
 * State-type / Continous actions
 * compatible with press and hold, and with clicks as toggles.
 * -----------------------------------------------------------------*/

static int ToolEraserRM2 = 0;

char * etype_to_string(enum effect_type etype){
  switch (etype) {
    case temp_on:  return "on";
    case toggle:   return "toggle";
    case temp_off: return "off";
  }
}

static int currTempEffect = NULL_EFFECT; // keeps track of any temporary effect to nullify when lifting the pen from the screen

void tool_eraser(enum effect_type etype, int fd_touch) {
  printf("writing eraser [%s]\n", etype_to_string(etype));
  switch (etype) {
    case temp_on:  currTempEffect = ERASER_ERASE; activate_tool_eraser(fd_touch); break;
    case toggle:   toggle_tool_eraser(fd_touch); break;
    case temp_off: currTempEffect = NULL_EFFECT; deactivate_tool_eraser(fd_touch); break;
  }
}

void activate_tool_eraser(int fd_pen) {
  printf("Activating ToolEraserRM2: writing eraser on\n");
  write_event(fd_pen, tool_rubber_off);
  write_event(fd_pen, tool_rubber_on);
  ToolEraserRM2 = 1;
}

void deactivate_tool_eraser(int fd_pen) {
  printf("Deactivating ToolEraserRM2: writing pen on\n");
  write_event(fd_pen, tool_pen_off);
  write_event(fd_pen, tool_pen_on);
  ToolEraserRM2 = 0;
}

void toggle_tool_eraser(int fd_pen) {
  if (ToolEraserRM2)
    deactivate_tool_eraser(fd_pen);
  else
    activate_tool_eraser(fd_pen);
}

void hl(enum effect_type etype, int fd_touch) {
  printf("highlighter [%s]\n", etype_to_string(etype));
  switch (etype) {
    case temp_on:  currTempEffect = WRITING_HL; action_hl(fd_touch); break;
    case toggle:   toggle_hl(fd_touch); break;
    case temp_off: currTempEffect = NULL_EFFECT; action_fineliner(fd_touch); break;
  }
}

void toggle_hl(int fd_pen) {
  if (hlOn) {
    action_fineliner(fd_pen);
  }
  else {
    action_hl(fd_pen);
  }
}

void action_tool_eraser(struct input_event *ev_pen, int fd_pen) {
  if (ToolEraserRM2) {
    if (ev_pen->code == BTN_TOOL_PEN &&
        ev_pen->value == 1) { // while activated, we write rubber_on or off to
                              // supercede any TOOL_PEN messages
      printf("From actionToolEraserRM2: writing eraser on\n");
      write_event(fd_pen, tool_rubber_off);
      write_event(fd_pen, tool_rubber_on);
    }
  }
}

static int toolEraseSelect = 0;

void tool_eraser_select(enum effect_type etype, int fd_touch) {
  printf("writing erase selection [%s]\n", etype_to_string(etype));
  switch (etype) {
    case temp_on:  currTempEffect = ERASER_SELECTION; activate_tool_eraser_select(fd_touch); break;
    case toggle:   toggle_tool_eraser_select(fd_touch); break;
    case temp_off: currTempEffect = NULL_EFFECT; deactivate_tool_eraser_select(fd_touch); break;
  }
}

void activate_tool_eraser_select(int fd_touch) {
  printf("activating eraser select...\n");
  write_oriented_tap_sequence_toolbar(fd_touch, 4, ERASER_PANEL, ERASER_PANEL, ERASER_SELECTION, ERASER_PANEL);
  toolEraseSelect = 1;
}

void deactivate_tool_eraser_select(int fd_touch) {
  printf("Deactivating ToolEraseSelect: writing writing_tool on\n");
  write_oriented_tap_sequence_toolbar(fd_touch, 3, SLEEP, WRITING1, SLEEP);
  toolEraseSelect = 0;
}

void toggle_tool_eraser_select(int fd_touch) {
  if (toolEraseSelect)
    deactivate_tool_eraser_select(fd_touch);
  else
    activate_tool_eraser_select(fd_touch);
}

static int toolSelect = 0;

void tool_select(enum effect_type etype, int fd_touch) {
  printf("writing select [%s]\n", etype_to_string(etype));
  switch (etype) {
    case temp_on:  currTempEffect = SELECT; activate_tool_select(fd_touch); break;
    case toggle:   toggle_tool_select(fd_touch); break;
    case temp_off: currTempEffect = NULL_EFFECT; deactivate_tool_select(fd_touch); break;
  }
}

void activate_tool_select(int fd_touch) {
  printf("Activating ToolSelect: writing select tool on\n");
  write_oriented_tap_sequence(fd_touch, 5, SELECT, TOOLBAR, SELECT,
                              LONG_SLEEP, TOOLBAR);
  toolSelect = 1;
}

void deactivate_tool_select(int fd_touch) {
  printf("Deactivating ToolSelect: writing writing tool on\n");
  write_oriented_tap_sequence(fd_touch, 5, WRITING1, TOOLBAR, WRITING1,
                              LONG_SLEEP, TOOLBAR);
  toolSelect = 0;
}

void toggle_tool_select(int fd_touch) {
  if (toolSelect)
    deactivate_tool_select(fd_touch);
  else
    activate_tool_select(fd_touch);
}

enum effect_type one_off() {
  if (currTempEffect == NULL_EFFECT) return temp_on;
  else return temp_off;
}

int temp_effect_end() {
  switch (currTempEffect) {
    case ERASER_SELECTION: case WRITING_FINELINER: // UI need some extra wait time
      write_oriented_tap_sequence(0, 4, LONG_SLEEP, LONG_SLEEP, LONG_SLEEP, LONG_SLEEP);
      break;
  }
  return currTempEffect;
}

void setAssumeTBOpen(bool new) {
  assumeTBOpen = new;
}

// Test stored locations in effects_data.h
void test_locations(int fd) {
  const char *str[4];
  str[0] = "Right Hand Portrait";
  str[1] = "Right Hand Landscape";
  str[2] = "Left Hand Portrait";
  str[3] = "Left Hand Landscape";

  for (int i = 0; i < 4; i++) {
    printf("Please set notebook to %s orientation. Close the tool bar and draw "
           "a large circle.\n"
           "When ready, press enter\n",
           str[i]);
    getchar(); // pauses until user presses enter

    printf("opening Toolbar...\n");
    write_oriented_tap_sequence(fd, 1, TOOLBAR);
    getchar();

    printf("tapping writing utensil 2...\n");
    write_oriented_tap_sequence(fd, 1, WRITING2);
    getchar();

    printf("tapping writing utensil 1 twice...\n");
    write_oriented_tap_sequence(fd, 1, WRITING1);
    write_oriented_tap_sequence(fd, 1, WRITING1);
    getchar();

    printf("tapping fineliner...\n");
    write_oriented_tap_sequence(fd, 1, WRITING_FINELINER);
    getchar();

    printf("tapping calligraphy pen...\n");
    write_oriented_tap_sequence(fd, 1, WRITING_CALLIGRAPHY);
    getchar();

    printf("tapping black...\n");
    write_oriented_tap_sequence(fd, 1, WRITING_BLACK);
    getchar();

    printf("tapping grey...\n");
    write_oriented_tap_sequence(fd, 1, WRITING_GREY);
    getchar();

    printf("tapping hl...\n");
    write_oriented_tap_sequence(fd, 1, WRITING_HL);
    getchar();

    printf("tapping eraser panel...\n");
    write_oriented_tap_sequence(fd, 1, ERASER_PANEL);
    getchar();

    printf("tapping eraser panel...\n");
    write_oriented_tap_sequence(fd, 1, ERASER_PANEL);
    getchar();

    printf("tapping erase selection...\n");
    write_oriented_tap_sequence(fd, 1, ERASER_SELECTION);
    getchar();

    printf("tapping eraser...\n");
    write_oriented_tap_sequence(fd, 1, ERASER_ERASE);
    getchar();

    printf("tapping select tool...\n");
    write_oriented_tap_sequence(fd, 1, SELECT);
    getchar();

    printf("tapping undo...\n");
    write_oriented_tap_sequence(fd, 1, UNDO);
    getchar();

    printf("tapping redo...\n");
    write_oriented_tap_sequence(fd, 1, REDO);

    printf("Completed %s.\n", str[i]);
  }
  printf("Completed all orientations.\n");
  exit(EXIT_SUCCESS);
}
