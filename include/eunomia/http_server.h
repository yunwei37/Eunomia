#ifndef HTTP_EUNOMIA_H
#define HTTP_EUNOMIA_H

#include "httplib.h"
#include "eunomia/eunomia_core.h"

class eunomia_server
{
private:
    // add a mutex to serialize the http request
    std::mutex seq_mutex;
    httplib::Server server;
    eunomia_core core;
    int port;

    void start_tracker(const httplib::Request &req, httplib::Response &res);
    void stop_tracker(const httplib::Request &req, httplib::Response &res);
    void list_trackers(const httplib::Request &req, httplib::Response &res);

public:
    eunomia_server(eunomia_config_data& config, int p);
    ~eunomia_server() = default;
    void serve(void);
};

#endif
