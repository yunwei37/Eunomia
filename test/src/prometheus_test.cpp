#include "eunomia/prometheus_server.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{ 
  auto server = prometheus_server("127.0.0.1:8528");
  server.start_prometheus_server();
  return 0;
}
