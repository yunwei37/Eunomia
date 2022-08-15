extern "C"
{
// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <argp.h>
#include <bpf/libbpf.h>
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

#include "base64.h"
#include "update.h"
#include "update.skel.h"
}
#include <fstream>

#include "base64.h"
#include "hot_update.h"
#include "httplib.h"

int main(int argc, char **argv)
{
  struct update_bpf *obj = NULL;
  const unsigned char *base64_data = NULL;
  size_t base64_len = 0;
  struct ebpf_update_data data;
  json j;

  obj = (struct update_bpf *)calloc(1, sizeof(*obj));
  if (!obj)
    return 1;
  if (update_bpf__create_skeleton(obj))
  {
    return 1;
  }

  data.name = obj->skeleton->name;
  data.data_sz = obj->skeleton->data_sz;
  base64_data = base64_encode((const unsigned char *)obj->skeleton->data, data.data_sz, &base64_len);
  data.data = (const char *)base64_data;
  for (int i = 0; i < obj->skeleton->map_cnt; i++)
  {
    data.maps_name.push_back(obj->skeleton->maps[i].name);
  }
  for (int i = 0; i < obj->skeleton->prog_cnt; i++)
  {
    data.progs_name.push_back(obj->skeleton->progs[i].name);
  }
  if (argc < 2)
  {
    std::cout << data.to_json();
    return 0;
  }
  std::string harg = data.to_json();
  json http_data = json::parse("{\
            \"name\": \"hotupdate\",\
            \"export_handlers\": [\
                {\
                    \"name\": \"plain_text\",\
                    \"args\": []\
                }\
            ],\
            \"args\": [\
            ]\
        }");
	http_data["args"].push_back(harg);
	std::cout << http_data.dump();
  httplib::Client cli(argv[1]);
  cli.Post("/start", http_data.dump(), "text/plain");

  return 0;
}
