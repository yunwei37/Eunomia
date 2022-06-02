/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef EUNOMIA_SEC_ANALYZER_H
#define EUNOMIA_SEC_ANALYZER_H

#include "files.h"
#include "model/event_handler.h"
#include "process.h"
#include "prometheus_server.h"
#include "syscall.h"

enum class sec_rule_level
{
  event,
  warnning,
  alert,
  // EVNETODO: add more levels?
};

enum class sec_rule_type
{
  syscall,
  tcp,
  process,
  files,
  mix,
  // EVNETODO: add more types?
};

// message for sec_rule
struct rule_message
{
  sec_rule_level level;
  std::string name;
  std::string message;
  int pid;

  std::string container_id;
  std::string container_name;
};

// describe a sec_rule
// signature: the signature of the rule, for example, process name, syscall, etc.
struct sec_rule_describe
{
  sec_rule_level level;
  sec_rule_type type;
  std::string name;
  std::string message;

  std::string signature;
};

struct sec_analyzer
{
  // EVNETODO: use the mutex
  std::mutex mutex;
  const std::vector<sec_rule_describe> rules;

  sec_analyzer(const std::vector<sec_rule_describe> &rules) : rules(rules)
  {
  }
  virtual void report_event(const rule_message &msg);
  void print_event(const rule_message &msg);
};

struct sec_analyzer_prometheus : sec_analyzer
{
  prometheus::Family<prometheus::Counter> &eunomia_sec_warn_counter;
  prometheus::Family<prometheus::Counter> &eunomia_sec_event_counter;
  prometheus::Family<prometheus::Counter> &eunomia_sec_alert_counter;

  void report_prometheus_event(const struct rule_message &msg);
  void report_event(const rule_message &msg);
  sec_analyzer_prometheus(prometheus_server &server, const std::vector<sec_rule_describe> &rules);
};

// all events will need pid
template<typename EVNET>
concept event_concept = requires
{
  typename EVNET::pid;
};

// base class for securiy rules
template<typename EVNET>
struct rule_base : event_handler<EVNET>
{
  std::shared_ptr<sec_analyzer> analyzer;
  sec_rule_level level;
  virtual ~rule_base() = default;

  // return rule id if matched
  // return -1 if not matched
  virtual int check_rule(tracker_event<EVNET> &e) = 0;
  void handle(tracker_event<EVNET> &e)
  {
    if (!analyzer) {
        std::cout << "analyzer is null" << std::endl;
    }
    int res = check_rule(e);
    if (res != -1)
    {
      struct rule_message msg;
      msg.level = level;
      msg.name = analyzer->rules[res].name;
      msg.message = analyzer->rules[res].message;
      msg.pid = e.pid;
      // EVNETODO: fix get container id
      msg.container_id = "36fca8c5eec1";
      msg.container_name = "Ubuntu";
      analyzer->report_event(msg);
    }
  }
};

// files rule:
//
// for example, a read or write to specific file
struct files_rule_base : rule_base<files_event>
{
  virtual ~files_rule_base() = default;
  int check_rule(const files_event &);
};

// process rule:
//
// for example, a specific process is running
struct process_rule_base : rule_base<process_event>
{
  virtual ~process_rule_base() = default;
  int check_rule(const process_event &);
};

// syscall rule:
//
// for example, a process is using a syscall
struct syscall_rule : rule_base<syscall_event>
{
  int check_rule(const syscall_event &);
};

#endif