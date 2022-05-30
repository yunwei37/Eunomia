#include <stdio.h>
#include "eunomia/config.h"

std::string config_file_path = "";

int main() {
  config config_toml;
  analyze_toml(config_file_path, config_toml);
  return 0;
}
