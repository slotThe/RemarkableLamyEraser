#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int fd;
  struct input_event ev;
  const size_t input_event_size = sizeof(struct input_event);

  char name[256] = "Unknown";

  if (argc != 3) {
      printf("Incorrect arguments.\n"
             "Please specify the input device location and the number of locations to record.\n");
      exit(EXIT_FAILURE);
    }


  char* devicePath = argv[1];
  char* end;
  int numLocations = strtol(argv[2], &end, 10);

  int locations[numLocations][2];
  int locationAvg[5][2];
  int locationX, locationY;




  fd = open(devicePath, O_RDONLY);
  if (fd == -1) {
      fprintf(stderr, "%s is not a vaild device\n", devicePath);
      exit(EXIT_FAILURE);
    }
  ioctl(fd, EVIOCGNAME(sizeof(name)), name);
  printf("Using device file = %s\n", devicePath);
  printf("With device name = %s\n", name);
  close(fd);


  setbuf(stdout, NULL);
  for(int i = 0; i < numLocations; i++) {
      printf("Tap Location %2d 5 times...  ", i+1);
      for (int j = 0; j < 5; j++) {
          fd = open(devicePath, O_RDONLY);
          locationX = 0;
          locationY = 0;
          while (locationX == 0 || locationY == 0) {
              read(fd, &ev, input_event_size);
              if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X) {
                   locationX = ev.value;
                }
              if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y) {
                  locationY = ev.value;
                 }
            }
          locationAvg[j][0] = locationX;
          locationAvg[j][1] = locationY;
          close(fd);
          printf("%d... ", j+1);
        }
      locations[i][0]= (locationAvg[0][0]+locationAvg[1][0]+locationAvg[2][0]+locationAvg[3][0]+locationAvg[4][0]) / 5 ;
      locations[i][1]= (locationAvg[0][1]+locationAvg[1][1]+locationAvg[2][1]+locationAvg[3][1]+locationAvg[4][1]) / 5 ;
      printf("Grabbed Location %d\n", i+1);
      sleep(1);

    }


  printf("\n\nLocations\n----------------\n");
  for(int i = 0; i < numLocations; i++) {
      printf("Location %2d: {%4d,%4d}\n", i+1, locations[i][0], locations[i][1]);
    }

  return 0;
}
