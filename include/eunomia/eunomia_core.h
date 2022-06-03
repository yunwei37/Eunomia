/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef EUNOMIA_TRACKER_FACTORY_H
#define EUNOMIA_TRACKER_FACTORY_H

#include "config.h"
#include "eunomia/config.h"
#include "eunomia/container.h"
#include "eunomia/files.h"
#include "eunomia/ipc.h"
#include "eunomia/myseccomp.h"
#include "eunomia/process.h"
#include "eunomia/prometheus_server.h"
#include "eunomia/sec_analyzer.h"
#include "eunomia/tcp.h"
#include "eunomia/tracker_manager.h"

// core for building tracker
// construct tracker with handlers
// and manage state
struct eunomia_core
{
 private:
  // eunomia config
  config core_config;
  // manager for all tracker
  tracker_manager core_tracker_manager;
  // manager for container events
  container_manager core_container_manager;
  prometheus_server core_prometheus_server;

  // sec analyzer
  std::shared_ptr<sec_analyzer> core_sec_analyzer;

  // create all event handlers for a tracker
  template<tracker_concept TRACKER>
  TRACKER::tracker_event_handler create_tracker_event_handler(const TRACKER* tracker_ptr);

  // create event handler for print to console
  template<tracker_concept TRACKER>
  TRACKER::tracker_event_handler create_print_event_handler(const TRACKER* tracker_ptr);

  // create a default tracker with default env
  template<tracker_concept TRACKER>
  std::unique_ptr<TRACKER> create_default_tracker(const tracker_data_base* base);

  // create a default tracker with other handlers
  template<tracker_concept TRACKER>
  std::unique_ptr<TRACKER> create_default_tracker_with_handler(
      const tracker_data_base* base,
      TRACKER::tracker_event_handler);

  // create a default tracker with sec_analyzer handlers
template<tracker_concept TRACKER, typename CHECKER>
  std::unique_ptr<TRACKER> create_default_tracker_with_sec_analyzer(const tracker_data_base* base);

  // start all trackers
  void start_trackers(void);
  // check and stop all trackers if needed
  void check_auto_exit(void);
  // start prometheus server
  void start_prometheus_server(void);
  // start container manager
  void start_container_manager(void);
  // start sec analyzer
  void start_sec_analyzer(void);
 public:
  eunomia_core(config& config);
  int start_eunomia(void);
};

#endif
