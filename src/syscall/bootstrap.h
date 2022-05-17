/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __BOOTSTRAP_H
#define __BOOTSTRAP_H

#define TASK_COMM_LEN 16
#define MAX_FILENAME_LEN 127
#define CONTAINER_ID_LEN 127

struct event {
	int pid;
	int ppid;
	uint32_t syscall_id;
	uint64_t mntns;
	char comm[TASK_COMM_LEN];
};

#endif /* __BOOTSTRAP_H */
