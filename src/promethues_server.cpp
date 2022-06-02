/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include "eunomia/prometheus_server.h"
#include "prometheus/client_metric.h"
#include "prometheus/counter.h"
#include "prometheus/exposer.h"
#include "prometheus/family.h"

prometheus_server::prometheus_server(std::string bind_address) : exposer(bind_address)
{
  registry = std::make_shared<prometheus::Registry>();
}

int prometheus_server::start_prometheus_server()
{
  using namespace prometheus;

  // ask the exposer to scrape the registry on incoming HTTP requests
  exposer.RegisterCollectable(registry);
  return 0;
}
