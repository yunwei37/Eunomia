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

// core for building tracker
// construct tracker with handlers
// and manage state
struct eunomia_core {
private:
    config core_config;
    tracker_manager core_tracker_manager;
    container_manager core_container_manager;
    prometheus_server core_prometheus_server;
    
    // create a 
    template <tracker_concept TRACKER>
    TRACKER::tracker_event_handler
    create_tracker_event_handler(void);

    template <tracker_concept TRACKER>
    std::unique_ptr<TRACKER>
    create_default_tracker(const tracker_data_base* base);

    void start_files_tracker(const tracker_data_base* base);

    void start_trackers(void);
public:
    eunomia_core(config &config);
    int start_eunomia(void); 
};

#endif