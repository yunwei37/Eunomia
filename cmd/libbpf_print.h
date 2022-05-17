#ifndef LIBBPF_PRINT_H
#define LIBBPF_PRINT_H

extern "C"
{
#include "process/process_tracker.h"
}

extern bool verbose;

int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    if (level == LIBBPF_DEBUG && !verbose)
        return 0;
    return vfprintf(stderr, format, args);
}

#endif