/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */
#ifndef __BOOTSTRAP_H

#define TASK_COMM_LEN 16
#define MAX_FILENAME_LEN 127
#define CONTAINER_ID_LEN 127

struct common_event {
	int pid;
	int ppid;
	uint64_t cgroup_id;
	uint32_t user_namespace_id;
	uint32_t pid_namespace_id;
	uint32_t mount_namespace_id;
};

#endif