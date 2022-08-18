#ifndef HOT_UPDATE_H
#define HOT_UPDATE_H

#include <fstream>
#include <iostream>
#include <string>

#include "../../include/json.hpp"

using json = nlohmann::json;

struct ebpf_update_meta_data
{
  /// meta data
  std::string name;
  std::vector<std::string> maps_name;
  std::vector<std::string> progs_name;
  int data_sz;
  std::string data;
  /// buffer to base 64 decode
  std::vector<char> base64_decode_buffer;

  std::string to_json()
  {
    json j;
    j["name"] = name;
    j["maps"] = maps_name;
    j["progs"] = progs_name;
    j["data_sz"] = data_sz;
    j["data"] = data;
    return j.dump();
  }
  void from_json_str(const std::string& j_str)
  {
    json jj = json::parse(j_str);
    name = jj["name"];
    maps_name = jj["maps"];
    progs_name = jj["progs"];
    data_sz = jj["data_sz"];
    data = jj["data"];
  }
};

#endif