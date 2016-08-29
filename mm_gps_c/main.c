//
//  main.c
//  mm_gps_c
//
//  Created by Paolo Bosetti on 24/08/16.
//  Copyright © 2016 Paolo Bosetti. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mm_gps_beacon.h"

// userdata to be passed to the data reader callback. Put anything useful in it.
typedef struct {
  int argc;
  char ** argv;
  char * filename;
  FILE * handle;
} gps_userdata;

// Data reader callback. It must consume and return ON single char at a time. The userdata struct
// provides persistency between calls (eg. file handles, ports, counters, command line arguments, etc)
// The callback signature must conform to: char (mm_gps_char_getter*)(void * data)
char dumped_char(gps_userdata *data) {
  char result;
  result = fgetc(data->handle);
  return result;
}

int main(int argc, const char * argv[]) {
  size_t i, len = 0;
  // fill the userdata struct
  gps_userdata data;
  data.argc = argc;
  data.argv = (char **)argv;
  data.filename = (char *)argv[1]; // yes I know: this is redundant ;)
  data.handle = fopen(data.filename, "rb");
  
  // Initialize the mm_gps object
  mm_gps *gps = mm_gps_init(&data);
  // Set the data reader callback
  mm_gps_set_reader(gps, (mm_gps_char_getter)&dumped_char);

  // Loop on packages: every call to mm_gps_next_raw_packet reads a new packet.
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
        printf("#%03zu (%02zu bytes, %04X CRC16) [%8.2f %6.2f %6.2f %6.2f]:", i, len, gps->buffer.crc16, mm_gps_time(gps), mm_gps_x(gps), mm_gps_y(gps), mm_gps_z(gps));
      }
      // print the HEX description of the raw packet
      hexprint(gps->buffer.packet.b, len);
    }
  }
  
  // Close file handle
  fclose(data.handle);
  // Free the gps object
  mm_gps_free(gps);
  return 0;
}
