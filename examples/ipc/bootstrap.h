/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/* Copyright (c) 2020 Facebook */
#ifndef __BOOTSTRAP_H
#define __BOOTSTRAP_H

struct event {
    unsigned int pid;
    unsigned int uid;
    unsigned int gid;
    unsigned int cuid;
    unsigned int cgid;
};

#endif /* __BOOTSTRAP_H */
