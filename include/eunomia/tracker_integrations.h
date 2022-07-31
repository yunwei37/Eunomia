/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef TRACKER_INTEGRATIONS_CMD_H
#define TRACKER_INTEGRATIONS_CMD_H

#include "eunomia/model/tracker_alone.h"

struct oomkill_tracker final: public tracker_alone_base {
  oomkill_tracker(config_data config) : tracker_alone_base(config) {}

  static std::unique_ptr<oomkill_tracker> create_tracker_with_default_env(tracker_event_handler handler);
};

struct tcpconnlat_tracker final: public tracker_alone_base {
  tcpconnlat_tracker(config_data config) : tracker_alone_base(config) {}

  static std::unique_ptr<tcpconnlat_tracker> create_tracker_with_default_env(tracker_event_handler handler);
};

#endif
