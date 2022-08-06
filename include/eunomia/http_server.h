#ifndef HTTP_EUNOMIA_H
#define HTTP_EUNOMIA_H

#include "httplib.h"
#include "eunomia/eunomia_core.h"

class eunomia_server
{
private:
    httplib::Server server;
    int port;

public:
    eunomia_server();
    eunomia_server(int p);
    ~eunomia_server();
    void start_tracking();
    void end_trackting();
};
#endif