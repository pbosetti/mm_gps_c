//
//  main.c
//  mm_gps_c
//
//  Created by Paolo Bosetti on 24/08/16.
//  Copyright © 2016 Paolo Bosetti. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "mm_gps_beacon.h"

// userdata to be passed to the data reader callback. Put anything useful in it.
typedef struct {
  int argc;
  char ** argv;
  char * filename;
  FILE * handle;
  int    port;
  uint32_t baudrate;
} gps_userdata;

// Stream opening: a serialport if filename begins with /dev,
// a standard dump file otherwise
int open_stream(gps_userdata *data) {
  if (strncmp(data->filename, "/dev", 4) == 0) {

    struct termios tty_ctrl;
    data->port = open(data->filename, O_RDWR| O_NONBLOCK | O_NDELAY);
    if (data->port < 0) {
      return -1;
    }

    memset(&tty_ctrl, 0, sizeof tty_ctrl);
    if (tcgetattr(data->port, &tty_ctrl ) != 0) {
      return -2;
    }
    
    cfsetospeed(&tty_ctrl, data->baudrate);
    cfsetispeed(&tty_ctrl, data->baudrate);
    // 8N1, no flow control
#ifndef POKY_LINUX
    tty_ctrl.c_cflag     &=  ~(PARENB|CSTOPB|CSIZE|CRTSCTS);
#else
    tty_ctrl.c_cflag     &=  ~(PARENB|CSTOPB|CSIZE);
#endif
    tty_ctrl.c_cflag     |=  CS8;
    // no signaling chars, no echo, no canonical processing
    tty_ctrl.c_lflag     =   0;
    tty_ctrl.c_oflag     =   0;  // no remapping, no delays
    tty_ctrl.c_cc[VMIN]  =   0;  // read doesn't block
    tty_ctrl.c_cc[VTIME] =   30; // 3 seconds read timeout
    tty_ctrl.c_cflag     |=  CREAD | CLOCAL; // turn on READ & ignore ctrl lines
    tty_ctrl.c_iflag     &=  ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
    tty_ctrl.c_lflag     &=  ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    tty_ctrl.c_oflag     &=  ~OPOST; // make raw
    tcflush(data->port, TCIFLUSH );  // Flush port
    if (tcsetattr (data->port, TCSANOW, &tty_ctrl) != 0) {
      return -3;
    }
    data->handle = NULL;
  }
  else {
    if ((data->handle = fopen(data->filename, "rb")) == NULL) {
      return -4;
    }
    data->port = -1;
  }
  return 0;
}

// Data reader callback. It must consume and return ONE single char at a time. The userdata struct
// provides persistency between calls (eg. file handles, ports, counters, command line arguments, etc)
// The callback signature must conform to: char (mm_gps_char_getter*)(void * data)
char dumped_char(gps_userdata *data) {
  char result;
  if (data->handle == NULL) {
    char buf[1];
    while (read(data->port, buf, 1) != 1) {} //NOOP
    result = *buf;
  }
  else {
    result = fgetc(data->handle);
  }
  return result;
}


int main(int argc, const char * argv[]) {
  size_t i, len = 0;
  int code = 0;
  double coords[3];
  // fill the userdata struct
  gps_userdata data = {.argc = argc, .argv = (char **)argv, .baudrate = 115200};
  data.filename = (char *)argv[1]; // yes I know: this is redundant ;)

  if ((code = open_stream(&data)) != 0) {
    printf("Code %d, could not open port %s\n", code, data.filename);
    perror("Error was");
    return -1;
  }
  
  // Initialize the mm_gps object
  mm_gps *gps = mm_gps_init(&data);
  
  // Set the data reader callback
  mm_gps_set_reader(gps, (mm_gps_char_getter)&dumped_char);

  // Loop on packets: every call to mm_gps_next_raw_packet reads a new packet.
  // Parsed contents are available into gps->buffer.packet.hedge (if it is a hedgehog data packet, code 1)
  // ot into gps->buffer.packet.beacons (if the system is frozen, code 2).
  for (i=0; i<100; i++) {
    if (!(len = mm_gps_next_raw_packet(gps))) {
      i--; // if len is zero, we are dealing with a fragment packet: discard it!
    }
    else {
      if (gps->buffer.packet.hedge.code == 2) {
        printf("#%03zu (%02zu bytes, %04X CRC16) [--------FROZEN packet--------]:", i, len, gps->buffer.crc16);
      }
      else if (gps->buffer.crc16 != 0) {
        printf("#%03zu (%02zu bytes, %04X CRC16) [---------CRC16 error---------]:", i, len, gps->buffer.crc16);
      }
      else {
        mm_gps_coords(gps, coords);
        printf("#%03zu (%02zu bytes, %04X CRC16) [%8.2f %6.2f %6.2f %6.2f]:", i, len, gps->buffer.crc16, mm_gps_time(gps), coords[0], coords[1], coords[2]);
      }
      // print the HEX description of the raw packet
      hexprint(gps->buffer.packet.b, len);
    }
  }
  
  // Close file handle
  if (data.handle) {
    fclose(data.handle);
  }
  else {
    close(data.port);
  }
  // Free the gps object
  mm_gps_free(gps);
  return 0;
}
