# 安全规则设计

## 安全分析和告警

目前我们的安全风险等级主要分为三类（未来可能变化，我觉得这个名字不一定很直观）：

include\eunomia\sec_analyzer.h
```cpp
enum class sec_rule_level
{
  event,
  warnning,
  alert,
  // TODO: add more levels?
};
```

安全规则和上报主要由 sec_analyzer 模块负责：

```cpp

struct sec_analyzer
{
  // EVNETODO: use the mutex
  std::mutex mutex;
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
```

我们通过 sec_analyzer 类来保存所有安全规则以供查询，同时以它的子类 sec_analyzer_prometheus 完成安全事件的上报和告警。具体的告警信息发送，可以由 prometheus 的相关插件完成，我们只需要提供一个接口。由于 rules 是不可变的，因此它在多线程读条件下是线程安全的。

## 安全规则实现

我们的安全风险分析和安全告警规则基于对应的handler 实现，例如：

include\eunomia\sec_analyzer.h
```cpp

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
```

这个部分定义了一个简单的规则基类，它对应于某一个 ebpf 探针上报的事件进行过滤分析，以系统调用上报的事件为例：

```cpp
// syscall rule:
//
// for example, a process is using a dangerous syscall
struct syscall_rule_checker : rule_base<syscall_event>
{
  syscall_rule_checker(std::shared_ptr<sec_analyzer> analyzer_ptr) : rule_base(analyzer_ptr)
  {}
  int check_rule(const tracker_event<syscall_event> &e, rule_message &msg);
};
```

其中的 check_rule 函数实现了对事件进行过滤分析，如果事件匹配了规则，则返回规则的 id，否则返回 -1：关于 check_rule 的具体实现，请参考：src\sec_analyzer.cpp

除了通过单一的 ebpf 探针上报的事件进行分析之外，通过我们的 handler 机制，我们还可以综合多种探针的事件进行分析，或者通过时序数据库中的查询进行分析，来发现潜在的安全风险事件。

## 其他

除了通过规则来实现安全风险感知，我们还打算通过机器学习等方式进行进一步的安全风险分析和发现。