// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
// Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
// All rights reserved.

#ifndef __CONTAINER_H
#define __CONTAINER_H

#include "../process/process.h"

struct container_event {
	struct process_event process;
	unsigned long container_id;
	char container_name[50];
};

#endif