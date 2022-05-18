/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __BOOTSTRAP_H
#define __BOOTSTRAP_H

#define TASK_COMM_LEN 16
#define MAX_FILENAME_LEN 127
#define CONTAINER_ID_LEN 127

// if pid is not set, this message is the first time a syscall happends in a process;
// if target_pid or cgroups is set, this message is all syscalls
struct syscall_event
{
	int pid;
	int ppid;
	uint32_t syscall_id;
	uint64_t mntns;
	char comm[TASK_COMM_LEN];

	long unsigned int args[6];
	int occur_times;
};

#endif /* __BOOTSTRAP_H */
