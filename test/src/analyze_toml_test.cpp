/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include <stdio.h>
#include "eunomia/config.h"

std::string config_file_path = "";

int main() {
  config config_toml;
  analyze_toml(config_file_path, config_toml);
  return 0;
}
