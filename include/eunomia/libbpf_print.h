/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef LIBBPF_PRINT_H
#define LIBBPF_PRINT_H

extern "C" {
#include <process/process_tracker.h>
}

#include <string>

extern bool verbose;

static int libbpf_print_fn(enum libbpf_print_level level, const char *format,
                           va_list args) {
  if (level == LIBBPF_DEBUG && !verbose)
    return 0;
  return vfprintf(stderr, format, args);
}

static std::string get_current_time(void) {
  struct tm *tm;
  char ts[32];
  time_t t;

  time(&t);
  tm = localtime(&t);
  strftime(ts, sizeof(ts), "%H:%M:%S", tm);
  return std::string(ts);
}

#endif