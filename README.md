# mm_gps_c
Plain C driver for [MarvelMind indoor GPS hedgehog](http://www.marvelmind.com).

## Notes
There are other drivers around, but this driver aims primarily at computational efficiency, using C unions and structures for parsing the binary packets in place rather than recursively, one-byte-at-a-time as in other solutions.

## Platforms
Tested and working on OS X, Debian, and Raspbian (Raspberry PI).

## Compiling
The repo contains a Xcode project. On other platforms, you can use Cmake and create the build target according to your preferred development environment (including Xcode!):

    $ cd build; cmake ..
    $ make
    
This generates both two libraries (static and dynamic) and a test executable, that is configured to read data from a binary dump of hedgehog data (as example, those in `dump.hex`).

By default, it uses GCC on linux, clang on OS X. If you prefer clang on linux, just type:

    $ cd build; CC=/usr/bin/clang cmake ..
    $ make

## Usage
Look at the example executable in `main.c`: you have to implement the readout callback, with a signature matching `char dumped_char(void *data)`. The callback must return one single byte (the next available on the serialport, for example) and can use the data argument (a C struct) for passing connection data between calls:

```C
  gps_userdata data;
  data.handle = fopen(filename, "rb");
  
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

```

# Note on cross-compilation
I am finding it useful to cross-compile with Docker. For example, you can easily cross-compile this project for Raspberry Pi by following these steps:

1. Get [Docker](http://docker.com)
2. Get the [thewtex/cross-compiler-linux-armv6](https://hub.docker.com/r/thewtex/cross-compiler-linux-armv6/) image
3. In the project root, on a docker console, do: `docker run --rm thewtex/cross-compiler-linux-armv6 > dockross`
4. On the same console, do: `./dockross cmake -Bbuild -H.`. This will run cmake in the target platform image
5. On the same console, do: `./dockross make -Cbuild`. This will make all your targets.
6. Copy the compiled binaries on your Raspberry Pi.

Docker can also be used for cross-compiling to Intel Edison:

1. get the [p4010/edison-build](https://hub.docker.com/r/p4010/edison-build) image;
2. from the project root, type: `docker run -ti --rm -v$PWD:/root/build p4010/edison-build`; this will put you in a shell, where `/root/build` is your project root dir;
3. build the project: `rm -rf build/*; cmake -Bbuild -H.; make -Cbuild`;
4. copy the compiled binaries on your Edison stick.
 