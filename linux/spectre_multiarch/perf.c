/*
 * Adrian Brzezinski (2018) <adrbxx at gmail.com>
 * License: GPLv2+
 *
 */

#include "perf.h"

static int sys_perf_event_open( struct perf_event_attr *attr, pid_t pid,
  int cpu, int group_fd, unsigned long flags ) {

  return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

int perf_init( struct perf_event_attr *attr ) {
  int fd = -1;

  if ( (fd = sys_perf_event_open(attr, getpid(), -1, -1, 0)) < 0 || errno != 0 ) {
    perror("sys_perf_event_open");

    if ( fd >= 0 ) close(fd);
    return -1;
  }

return fd; 
}

inline void perf_start(int fd) {

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

}

inline void perf_stop(int fd) {

  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
}

uint64_t perf_read(int fd) {
  int ret;
  uint64_t counter;

  if ( (ret = read(fd, &counter, sizeof(counter))) < 0 || ret != sizeof(counter) ) {
    perror("perf_read");
  }

  assert(ret == sizeof(counter));

return counter;
}

uint64_t perf_min_deviation(int fd) {

  uint64_t result = -1, counter;
  int i;

  for (i=0; i < 4096 ; i++) {
    perf_start(fd);
    perf_stop(fd);

    counter = perf_read( fd );
    if ( counter < result ) result = counter;
  }

return result;
}

void flush_dcache(size_t cache_size) {

  uint8_t *cache_flush;
  int pgs = sysconf(_SC_PAGESIZE);
  size_t i;

  // align to page size
  cache_size = (cache_size + pgs - 1) & ~(pgs - 1);

  cache_flush = (uint8_t*)malloc(cache_size);
  assert( cache_flush != 0 );

  // read one byte per page
  for ( i=0 ; i < cache_size ; i += pgs ) {
//    irnd = i + (((65454729 * i) + 834039843) % (cache_size - i - 1));

    cache_flush[i] = 0xff;
  }

  free(cache_flush);
}

