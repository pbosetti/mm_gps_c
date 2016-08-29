//
//  mm_gps_utils.c
//  mm_gps_c
//
//  Created by Paolo Bosetti on 25/08/16.
//  Copyright © 2016 Paolo Bosetti. All rights reserved.
//

#include "mm_gps_utils.h"

ushort CRC16(const void *buf, ushort length) {
  uchar *arr = (uchar *)buf;
  bytes_t crc;
  crc.w = 0xffff;
  while(length--) {
    char i;
    ushort odd;
    crc.b.lo ^= *arr++;
    for(i = 0; i< 8; i++) {
      odd = crc.w& 0x01;
      crc.w >>= 1;
      if(odd)
        crc.w ^= 0xa001;
    }
  }
  return (ushort)crc.w;
}

void hexprint(char * buf, size_t buflen) {
  size_t i;
  for (i = 0; i < buflen; i ++) {
    printf(" %02X", (uchar)buf[i]);
  }
  putchar('\n');
}

