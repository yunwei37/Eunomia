/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef EUNOMIA_TRACKER_FACTORY_H
#define EUNOMIA_TRACKER_FACTORY_H

#include "config.h"
#include "eunomia/process.h"
#include "eunomia/files.h"
#include "eunomia/tcp.h"
#include "eunomia/ipc.h"
#include "eunomia/container.h"
#include "eunomia/prometheus_server.h"
#include "eunomia/tracker_manager.h"
#include "eunomia/config.h"
#include "eunomia/myseccomp.h"

// core for building tracker
// construct tracker with handlers
// and manage state
struct eunomia_core {
private:
    config core_config;
    tracker_manager core_tracker_manager;
    container_manager core_container_manager;
    prometheus_server core_prometheus_server;
    
    // create all event handlers for a tracker
    template <tracker_concept TRACKER>
    TRACKER::tracker_event_handler
    create_tracker_event_handler(void);

    // create event handler for print to console
    template <tracker_concept TRACKER>
    TRACKER::tracker_event_handler
    create_print_event_handler(void);

    // create a default tracker with default env
    template <tracker_concept TRACKER>
    std::unique_ptr<TRACKER>
    create_default_tracker(const tracker_data_base* base);

    // start all trackers
    void start_trackers(void);
public:
    eunomia_core(config &config);
    int start_eunomia(void); 
};

#endif
