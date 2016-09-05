//
//  mm_gps_beacon.h
//  mm_gps_c
//
//  Created by Paolo Bosetti on 24/08/16.
//  Copyright © 2016 Paolo Bosetti. All rights reserved.
//

#ifndef mm_gps_beacon_h
#define mm_gps_beacon_h

#include <stdio.h>
#include <stdint.h>
#include "mm_gps_utils.h"

#define MMGPS_NUMBER_OF_BEACONS 5 // including the hedgehog!
#define MMGPS_SEPARATOR "\xFFG"
#define MMGPS_SEPLEN 2

// The largest packet is for code 2 with MMGPS_NUMBER_OF_BEACONS beacons, plus the next separator
// separator(2)+header(4)+N*8+CRC16(2)+next packet separator(2)
#define MMGPS_MAX_PACKET_LEN MMGPS_SEPLEN+4+MMGPS_NUMBER_OF_BEACONS*8+2+MMGPS_SEPLEN
typedef char (*mm_gps_char_getter)(void *data);

typedef enum {
  HEDGEHOG = 1,
  FROZEN
} mm_gps_packet_codes;

typedef union {
  char b[MMGPS_MAX_PACKET_LEN];
  
  struct __attribute__((packed)) {
    uint8_t  address;
    uint8_t  type;
    uint8_t  code;
    uint8_t  code_h; // code high byte, not used
    uint8_t  size;
    uint32_t time;
    int16_t  x;
    int16_t  y;
    int16_t  z;
    uint8_t  flag;
    char reserved[5];
    uint16_t crc16;
  } hedge;
  
  struct __attribute__((packed)) {
    uint8_t  address;
    uint8_t  type;
    uint8_t  code;
    uint8_t  code_h; // code high byte, not used
    uint8_t  size;
    uint8_t  n_beacons;
    struct __attribute__((packed)) {
      uint8_t address;
      uint16_t x;
      uint16_t y;
      uint16_t z;
      uint8_t  reserved;
    } beacon[MMGPS_NUMBER_OF_BEACONS];
  } beacons;
} mm_gps_packet;

typedef struct {
  struct __attribute__((packed)) {
    mm_gps_packet packet;
    char * head;
    ushort crc16;
  } buffer;
  void * userdata;
  mm_gps_char_getter get_next_char;
} mm_gps;


/*! 
 Initialize a newly allocated mm_gps object
 \param data Pointer to a struct containing info useful for the custom reader callout
 */
mm_gps * mm_gps_init(void * data);

/*!
 Free the mm_gps instance memory.
 \params gps The mm_gps object to be freed
 */
void mm_gps_free(mm_gps *gps);

/*!
 Set the data reader callback. The callback must read and return one character at a time. 
 The callback gets a pointer to an userdata structure, which can convey file/port handles, etc.
 \brief Set the data reader callback.
 \param gps Pointer to the mm_gps object
 \param f the data reader callback, must conform to mm_gps_char_getter
 */
void mm_gps_set_reader(mm_gps *gps, mm_gps_char_getter f);

/*!
 Read the next raw packet.
 \param gps Pointer to the mm_gps object
 \return the size of the new packet, or 0 if no data were available
 */
uint16_t mm_gps_next_raw_packet(mm_gps *gps);

/*!
 Return the timestamp of the last packet (obtained with a previous call to mm_gps_next_raw_packet).
 \param gps Pointer to the mm_gps object
 \return The hedgehog timestamp since the last wake up.
 */
double mm_gps_time(mm_gps *gps);

/*!
 Return the x position of the last packet (obtained with a previous call to mm_gps_next_raw_packet).
 \param gps Pointer to the mm_gps object
 \return The hedgehog x position
 */
double mm_gps_x(mm_gps *gps);

/*!
 Return the y position of the last packet (obtained with a previous call to mm_gps_next_raw_packet).
 \param gps Pointer to the mm_gps object
 \return The hedgehog y position
 */
double mm_gps_y(mm_gps *gps);

/*!
 Return the z position of the last packet (obtained with a previous call to mm_gps_next_raw_packet).
 \param gps Pointer to the mm_gps object
 \return The hedgehog z position
 */
double mm_gps_z(mm_gps *gps);

/*!
 Set the provided array to the three coordinates of the last packet (obtained with a previous call to mm_gps_next_raw_packet).
 \param coords Pointer to the vector of three double to be filled with the coordinates. It must be pre-allocated.
 */
void mm_gps_coords(mm_gps *gps, double * coords);

#endif /* mm_gps_beacon_h */
