#include "eunomia/prometheus_server.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{ 
  auto server = prometheus_server("127.0.0.1:8528");


  // add a new counter family to the registry (families combine values with the
  // same name, but distinct label dimensions)
  //
  // @note please follow the metric-naming best-practices:
  // https://prometheus.io/docs/practices/naming/
  auto& packet_counter =
      prometheus::BuildCounter().Name("observed_packets_total").Help("Number of observed packets").Register(*server.registry);

  // add and remember dimensional data, incrementing those is very cheap
  auto& tcp_rx_counter = packet_counter.Add({ { "protocol", "tcp" }, { "direction", "rx" } });
  auto& tcp_tx_counter = packet_counter.Add({ { "protocol", "tcp" }, { "direction", "tx" } });
  auto& udp_rx_counter = packet_counter.Add({ { "protocol", "udp" }, { "direction", "rx" } });
  auto& udp_tx_counter = packet_counter.Add({ { "protocol", "udp" }, { "direction", "tx" } });

  // add a counter whose dimensional data is not known at compile time
  // nevertheless dimensional values should only occur in low cardinality:
  // https://prometheus.io/docs/practices/naming/#labels
  auto& http_requests_counter =
      prometheus::BuildCounter().Name("http_requests_total").Help("Number of HTTP requests").Register(*server.registry);

  server.start_prometheus_server();

  
  for (int i = 0; i< 100; i++)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto random_value = std::rand();

    if (random_value & 1)
      tcp_rx_counter.Increment();
    if (random_value & 2)
      tcp_tx_counter.Increment();
    if (random_value & 4)
      udp_rx_counter.Increment();
    if (random_value & 8)
      udp_tx_counter.Increment();

    const std::array<std::string, 4> methods = { "GET", "PUT", "POST", "HEAD" };
    auto method = methods.at(random_value % methods.size());
    // dynamically calling Family<T>.Add() works but is slow and should be
    // avoided
    http_requests_counter.Add({ { "method", method } }).Increment();
  }

  return 0;
}
