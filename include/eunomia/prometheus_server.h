#ifndef EUNOMIA_PROMETHEUS_SERVER_H
#define EUNOMIA_PROMETHEUS_SERVER_H

#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include "prometheus/client_metric.h"
#include "prometheus/counter.h"
#include "prometheus/exposer.h"
#include "prometheus/family.h"
#include "prometheus/registry.h"

#include "model/event_handler.h"

struct prometheus_server
{   
    prometheus_server(std::string bind_address) : exposer(bind_address) {
        registry = std::make_shared<prometheus::Registry>();
    }
    prometheus::Exposer exposer;
    std::shared_ptr<prometheus::Registry> registry;
    int start_prometheus_server();
};


#endif