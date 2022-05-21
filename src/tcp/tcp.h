// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2020 Anton Protopopov
#ifndef __TCPCONNECT_H
#define __TCPCONNECT_H

#include <stdint.h>

/* The maximum number of items in maps */
#define MAX_ENTRIES 8192

/* The maximum number of ports to filter */
#define MAX_PORTS 64

#define TASK_COMM_LEN 16

struct ipv4_flow_key {
  uint32_t saddr;
  uint32_t daddr;
  uint16_t dport;
};

struct ipv6_flow_key {
  uint8_t saddr[16];
  uint8_t daddr[16];
  uint16_t dport;
};

struct tcp_event {
  union {
    uint32_t saddr_v4;
    uint8_t saddr_v6[16];
  };
  union {
    uint32_t daddr_v4;
    uint8_t daddr_v6[16];
  };
  char task[TASK_COMM_LEN];
  uint64_t ts_us;
  uint32_t af; // AF_INET or AF_INET6
  uint32_t pid;
  uint32_t uid;
  uint16_t dport;
};

#endif /* __TCPCONNECT_H */
