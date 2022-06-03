/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef EUNOMIA_PROMETHEUS_SERVER_H
#define EUNOMIA_PROMETHEUS_SERVER_H

#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include "prometheus/exposer.h"
#include "prometheus/registry.h"

#include "model/event_handler.h"

struct prometheus_server
{   
    prometheus_server(std::string bind_address);
    prometheus::Exposer exposer;
    std::shared_ptr<prometheus::Registry> registry;
    int start_prometheus_server();
};


#endif
