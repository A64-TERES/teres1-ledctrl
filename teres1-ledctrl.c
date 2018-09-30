/* teres1-ledctrl.c - quick and dirty capsl/numl led trigger

   Copyright (C) 2017 OLIMEX LTD.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2 of the License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>

#define LED_CAPS	71
#define LED_NUM		68

void export_gpio(int gpio)
{
  int fd;
  char buf[255];

  fd = open("/sys/class/gpio/export", O_WRONLY);
  sprintf(buf, "%d", gpio);
  write(fd, buf, strlen(buf));
  close(fd);

  sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
  fd = open(buf, O_WRONLY);
  write(fd, "out", 3);
  close(fd);
}

void set_gpio(int gpio, int value)
{
  char buf[255];
  sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
  int fd = open(buf, O_WRONLY);
  sprintf(buf, "%d", value);
  write(fd, buf, 1);
  close(fd);
}

void usage(void)
{
  extern char *program_invocation_short_name;
  printf("USAGE:\n");
  printf("   %s /dev/input/eventX\n", program_invocation_short_name);
  printf("\n");
}

int main (int argc, char **argv)
{
  const char *device = NULL;
  int fd;
  struct input_event ie;

  if (!argv[1])
  {
    usage();
    exit(0);
  }

  device = argv[1];

  if((fd = open(device, O_RDONLY)) == -1)
  {
    perror("Could not open input device");
    exit(255);
  }

  export_gpio(LED_NUM);
  export_gpio(LED_CAPS);


  while(read(fd, &ie, sizeof(struct input_event)))
  {
    switch (ie.type)
    {
    case 17: // EV_LED
      switch (ie.code) {
      case 0: // LED_NUML
        printf("type %d\tcode %d\tvalue %d\n", ie.type, ie.code, ie.value);
        set_gpio(LED_NUM, ie.value);
        break;
      case 1: // LED_CAPSL
        printf("type %d\tcode %d\tvalue %d\n", ie.type, ie.code, ie.value);
        set_gpio(LED_CAPS, ie.value);
        break;
      }
      break;
    default:
      if(errno == ENODEV){ // gpio device is gone
	exit(1);
      }
    }
    errno = 0;

  }

  return 0;
}
