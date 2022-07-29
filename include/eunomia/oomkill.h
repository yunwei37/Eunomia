/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef OOM_CMD_H
#define OOM_CMD_H

#include "eunomia/model/tracker_alone.h"

class oomkill_tracker: public tracker_alone_base {
  oomkill_tracker(config_data config);
  oomkill_tracker(tracker_alone_env env);

  static std::unique_ptr<oomkill_tracker> create_tracker_with_default_env(tracker_event_handler handler);
};

#endif
