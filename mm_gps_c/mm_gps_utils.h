//
//  mm_gps_utils.h
//  mm_gps_c
//
//  Created by Paolo Bosetti on 25/08/16.
//  Copyright © 2016 Paolo Bosetti. All rights reserved.
//

#ifndef mm_gps_utils_h
#define mm_gps_utils_h

#include <stdio.h>
#include <sys/types.h>

#ifndef uchar
typedef unsigned char uchar;
#endif

typedef union {
  ushort w;
  struct{
    uchar lo;
    uchar hi;
  } b;
  uchar bs[2];
} bytes_t;

ushort CRC16(const void *buf, ushort length);
void hexprint(char * buf, size_t buflen);

#endif /* mm_gps_utils_h */
