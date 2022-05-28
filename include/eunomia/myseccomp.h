#ifndef SECCOMP_H
#define SECCOMP_H

#define _GNU_SOURCE 1
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/prctl.h>
#include <seccomp.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <vector>
#include <string>

#include "seccomp-bpf.h"
#include "syscall_helper.h"
#include "config.h"

bool is_not_exist(uint32_t syscall_id[], int len, int id);

static int install_syscall_filter(uint32_t syscall_id[], int len);

int get_syscall_id(std::string syscall_name);

// Enable Seccomp syscall
// param seccomp_config type is defined by include/eunomia/config.h
int enable_seccomp_white_list(seccomp_config config);

#endif
