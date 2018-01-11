/*
 * Measure cached vs non-cached memory read on ARM
 * 
 * Adrian Brzezinski (2018) <adrbxx at gmail.com>
 * License: GPLv2+
 *
 */

#ifndef __PERF_H__
#define __PERF_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <syscall.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>


int perf_init( struct perf_event_attr *attr );

void perf_start(int fd);
void perf_stop(int fd);
uint64_t perf_read(int fd);

uint64_t perf_min_deviation(int fd);

// not strictly related to perf, but it's just convenient
void flush_dcache();

#endif /* __PERF_H__ */
