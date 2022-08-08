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
  // TODO: add more levels?
};

enum class sec_rule_type
{
  syscall,
  tcp,
  process,
  files,
  mix,
  // TODO: add more types?
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

class sec_analyzer
{
private:
  // EVNETODO: use the mutex
  std::mutex mutex;
public:
  const std::vector<sec_rule_describe> rules;

  sec_analyzer(const std::vector<sec_rule_describe> &in_rules) : rules(in_rules)
  {
  }
  virtual ~sec_analyzer() = default;
  virtual void report_event(const rule_message &msg);
  void print_event(const rule_message &msg);

  static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_default_rules(void);
  static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_additional_rules(const std::vector<sec_rule_describe> &rules);
};

struct sec_analyzer_prometheus : sec_analyzer
{
  prometheus::Family<prometheus::Counter> &eunomia_sec_warn_counter;
  prometheus::Family<prometheus::Counter> &eunomia_sec_event_counter;
  prometheus::Family<prometheus::Counter> &eunomia_sec_alert_counter;

  void report_prometheus_event(const struct rule_message &msg);
  void report_event(const rule_message &msg);
  sec_analyzer_prometheus(prometheus_server &server, const std::vector<sec_rule_describe> &rules);

  static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_default_rules(prometheus_server &server);
  static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_additional_rules(const std::vector<sec_rule_describe> &rules, prometheus_server &server);
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
  rule_base(std::shared_ptr<sec_analyzer> analyzer_ptr) : analyzer(analyzer_ptr) {}
  virtual ~rule_base() = default;

  // return rule id if matched
  // return -1 if not matched
  virtual int check_rule(const tracker_event<EVNET> &e, rule_message &msg) = 0;
  void handle(tracker_event<EVNET> &e)
  {
    if (!analyzer)
    {
      std::cout << "analyzer is null" << std::endl;
    }
    struct rule_message msg;
    int res = check_rule(e, msg);
    if (res != -1)
    {
      analyzer->report_event(msg);
    }
  }
};

// files rule:
//
// for example, a read or write to specific file
struct files_rule_checker : rule_base<files_event>
{
  virtual ~files_rule_checker() = default;
  files_rule_checker(std::shared_ptr<sec_analyzer> analyzer_ptr) : rule_base(analyzer_ptr)
  {}
  int check_rule(const tracker_event<files_event> &e, rule_message &msg);
};

// process rule:
//
// for example, a specific process is running
struct process_rule_checker : rule_base<process_event>
{
  virtual ~process_rule_checker() = default;
  process_rule_checker(std::shared_ptr<sec_analyzer> analyzer_ptr) : rule_base(analyzer_ptr)
  {}
  int check_rule(const tracker_event<process_event> &e, rule_message &msg);
};

// syscall rule:
//
// for example, a process is using a syscall
struct syscall_rule_checker : rule_base<syscall_event>
{
  syscall_rule_checker(std::shared_ptr<sec_analyzer> analyzer_ptr) : rule_base(analyzer_ptr)
  {}
  int check_rule(const tracker_event<syscall_event> &e, rule_message &msg);
};

#endif