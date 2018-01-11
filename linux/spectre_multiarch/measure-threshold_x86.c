/*
 * Adrian Brzezinski (2018) <adrbxx at gmail.com>
 * License: GPLv2+
 *
 * https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6
 *
 * $ gcc spectre-measure-threshold.c -o m
 * $ ./m
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <x86intrin.h>

uint64_t measure_read(void *p, int flush) {

  uint64_t rt = 0, rt_start = 0;
  volatile uint8_t *addr = p;
  volatile int v;
  int i, junk;

  for (i=0; i < 16384 ; i++) {
    if ( flush ) {

      _mm_clflush(p);
      for ( v=0 ; v < 400 ; v++ ) {}
    }

    rt_start = __rdtscp(&junk);
    junk = *addr;
    rt += __rdtscp(&junk) - rt_start;
  }

return (rt >> 14);
}


int main() {

  int addr = 0xc0de;
  uint64_t cached_rt, noncached_rt;

  cached_rt = measure_read(&addr, 0);
  noncached_rt = measure_read(&addr, 1);

  printf("Cached average read time: %d\n", cached_rt);
  printf("Non-cached average read time: %d\n", noncached_rt);

  if ( cached_rt*3 >= noncached_rt ) {
    printf("No clear prediction!\n");
  } else
    printf("Suggested threshold: %d\n", cached_rt*2 );

return 0;
}

