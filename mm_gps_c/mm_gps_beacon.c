//
//  mm_gps_beacon.c
//  mm_gps_c
//
//  Created by Paolo Bosetti on 24/08/16.
//  Copyright © 2016 Paolo Bosetti. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "mm_gps_beacon.h"

#define buflen(gps) gps->buffer.head - gps->buffer.packet.b
#if !(defined(BYTE_ORDER) && defined(BIG_ENDIAN))
#error "Cannot figure out byte order on this platform"
#endif
#if BYTE_ORDER == BIG_ENDIAN
#define SWAP16(num) ((num & 0xff) >> 8) | (num << 8)
#define SWAP32(num) ((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | (num << 24)
#else
#define SWAP16(num) num
#define SWAP32(num) num
#endif

mm_gps * mm_gps_init(void * data) {
  mm_gps *gps = (mm_gps*)malloc(sizeof(mm_gps));
  memset(gps->buffer.packet.b, 0, sizeof(gps->buffer.packet.b));
  gps->buffer.head = gps->buffer.packet.b;
  gps->userdata = data;
  return gps;
}

void mm_gps_free(mm_gps *gps) {
  free(gps);
}

void mm_gps_set_reader(mm_gps *gps, mm_gps_char_getter f) {
  gps->get_next_char = f;
}

uint16_t mm_gps_next_raw_packet(mm_gps *gps) {
  uint16_t i, result;
BEGIN:
  for (i=0; i<MMGPS_MAX_PACKET_LEN; i++) {
    *gps->buffer.head = (*gps->get_next_char)(gps->userdata);
    gps->buffer.head++;
    if (memcmp(gps->buffer.head-MMGPS_SEPLEN, MMGPS_SEPARATOR, MMGPS_SEPLEN) == 0) {
      if (gps->buffer.packet.hedge.code == HEDGEHOG && buflen(gps) < (MMGPS_SEPLEN+21+MMGPS_SEPLEN)) {
        // NO-OP: packet is still incomplete
      }
      else if (gps->buffer.packet.beacons.code == FROZEN && buflen(gps) < (MMGPS_SEPLEN+4+gps->buffer.packet.beacons.n_beacons*8+2+MMGPS_SEPLEN)) {
        // NO_OP: packet is still incomplete
      }
      else {
        break;
      }
    }
  }
  if (strncmp(gps->buffer.packet.b, MMGPS_SEPARATOR, MMGPS_SEPLEN) != 0) {
    // packet DES NOT start with sep: discarding all data til the next sep
    result = 0;
    gps->buffer.crc16 = -1;
    memset(gps->buffer.packet.b, 0, MMGPS_MAX_PACKET_LEN);
    strncpy(gps->buffer.packet.b, MMGPS_SEPARATOR, MMGPS_SEPLEN);
    gps->buffer.head = gps->buffer.packet.b + MMGPS_SEPLEN;
  }
  else {
    // Packet starts with sep: deal with it!
    if (gps->buffer.head <= gps->buffer.packet.b + MMGPS_SEPLEN)
      goto BEGIN; // Packet is too short: go back to accumulator
    memset(gps->buffer.head-MMGPS_SEPLEN, 0, MMGPS_SEPLEN);
    result = gps->buffer.head - gps->buffer.packet.b - MMGPS_SEPLEN;
    gps->buffer.head = gps->buffer.packet.b + MMGPS_SEPLEN;
    gps->buffer.crc16 = CRC16(gps->buffer.packet.b, result);
  }
  return result;
}

double mm_gps_time(mm_gps *gps) {
  if (gps->buffer.crc16 != 0 || gps->buffer.packet.hedge.code == FROZEN) {
    return 0;
  }
  return (double)(SWAP32(gps->buffer.packet.hedge.time)) / 64.0;
}

double mm_gps_x(mm_gps *gps) {
  if (gps->buffer.crc16 != 0 || gps->buffer.packet.hedge.code == FROZEN) {
    return 0;
  }
  return (double)(SWAP16(gps->buffer.packet.hedge.x)) / 100.0;
}

double mm_gps_y(mm_gps *gps) {
  if (gps->buffer.crc16 != 0 || gps->buffer.packet.hedge.code == FROZEN) {
    return 0;
  }
  return (double)(SWAP16(gps->buffer.packet.hedge.y)) / 100.0;
}

double mm_gps_z(mm_gps *gps) {
  if (gps->buffer.crc16 != 0 || gps->buffer.packet.hedge.code == FROZEN) {
    return 0;
  }
  return (double)(SWAP16(gps->buffer.packet.hedge.z)) / 100.0;
}

void mm_gps_coords(mm_gps *gps, double * coords) {
  if (gps->buffer.crc16 == 0 && gps->buffer.packet.hedge.code != FROZEN) {
    coords[0] = mm_gps_x(gps);
    coords[1] = mm_gps_y(gps);
    coords[2] = mm_gps_z(gps);
  }
}

#undef SWAP16
#undef SWAP32

