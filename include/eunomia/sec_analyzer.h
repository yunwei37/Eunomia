/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef EUNOMIA_SEC_ANALYZER_H
#define EUNOMIA_SEC_ANALYZER_H

#include "model/event_handler.h"
#include "process.h"
#include "files.h"

enum class sec_rule_level
{
  attention,
  warnning,
  // TODO: add more levels?
};

template <typename T>
struct rule_base : event_handler<T>
{
    sec_rule_level level;
    virtual ~rule_base() = default;
    virtual bool check_rule(const std::string&) = 0;
};

// files rule:
//
// for example, a read or write to specific file
struct files_rule_base : rule_base<files_event>
{
    virtual ~files_rule_base() = default;
    virtual bool check_rule(const std::string&) = 0;
};

// process rule:
//
// for example, a specific process is running
struct process_rule_base : rule_base<process_event>
{
    virtual ~process_rule_base() = default;
    virtual bool check_rule(const std::string&) = 0;
};

#endif