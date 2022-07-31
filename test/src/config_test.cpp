#include "eunomia/config.h"

#include <json.hpp>
#include <toml.hpp>

using namespace nlohmann;

int main()
{
  auto config1 = eunomia_config_data::from_json_file("test/config.json");
  auto config2 = eunomia_config_data::from_toml_file("test/config.toml");

  std::cout << config1.prometheus_listening_address <<std::endl;
  std::cout << config1.enabled_trackers[0].name <<std::endl;
  std::cout << config2.prometheus_listening_address <<std::endl;
  std::cout << config1.fmt <<std::endl;
  std::cout << config2.fmt <<std::endl;

  return 0;
}
