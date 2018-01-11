/*
 * Measure cached vs non-cached memory read
 * 
 * Adrian Brzezinski (2018) <adrbxx at gmail.com>
 * License: GPLv2+
 *
 * In some cases below line may help:
 * # echo 1 > /proc/sys/kernel/perf_event_paranoid
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "perf.h"


uint64_t measure_read(void *p, size_t cache_size, int flush) {

  volatile uint8_t *addr = p;
  uint64_t result = 0, counter, perf_d;
  int i, junk;

  struct perf_event_attr clock_attr = {
    .type       = PERF_TYPE_HARDWARE,
    .config     = PERF_COUNT_HW_CPU_CYCLES
  };
  int clock_fd = -1;

  clock_fd = perf_init(&clock_attr);
  assert(clock_fd != -1);

  perf_d = perf_min_deviation(clock_fd);
//  printf("%llu\n", perf_d);

  for (i=0; i < 16384 ; i++) {
    if ( flush ) flush_dcache(cache_size);

    perf_start(clock_fd);
    junk = *addr;

//    volatile int v;
//    for ( v=0 ; v < 100 ; v++ ) {}
    perf_stop(clock_fd);

    counter = perf_read( clock_fd );
    result += counter - perf_d;
  }

  close(clock_fd);

return (result >> 14);
}

int main(int argc, const char **argv) {

  int addr = 0xc0de;
  uint64_t cached_rt, noncached_rt;
  size_t cache_size = 0;

  if ( argc != 2 ) {
    printf("Usage: %s <flush table size>\n" \
        "  Increase table size until there will be a permanent difference\n" \
        "  between cached and non-cached read\n", argv[0] );

    return 1;
  }

  sscanf(argv[1], "%d", &cache_size);

  cached_rt = measure_read(&addr, cache_size, 0);
  printf("Cached average read time: %llu\n", cached_rt);

  noncached_rt = measure_read(&addr, cache_size, 1);
  printf("Non-cached average read time: %llu\n", noncached_rt);

  if ( cached_rt*3 >= noncached_rt ) {
    printf("No clear prediction!\n");
  } else
    printf("Suggested threshold: %llu\n", cached_rt*2 );

return 0;
}
