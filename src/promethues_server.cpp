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
#include "prometheus/registry.h"

int prometheus_server::start_prometheus_server()
{
  using namespace prometheus;

  // ask the exposer to scrape the registry on incoming HTTP requests
  exposer.RegisterCollectable(registry);
  return 0;
}
