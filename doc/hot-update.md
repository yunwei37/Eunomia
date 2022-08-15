# 使用 Eunomia 实现通过 http API 完成 ebpf 程序的极速热更新

目前 ebpf 程序的热更新还在测试阶段，我们还在探索更好的实现方式；

after compile, you will get a client an a server

```sh
./client > update.json
sudo ./update $(cat update.json)
```

This program seprated the libbpf load skeleton and exec code into 2 parts, and use json to pass the base64-encoded ebpf program between them.

the load skeleton part:

```c
obj = (struct update_bpf *)calloc(1, sizeof(*obj));
  if (!obj)
    return 1;
  if (update_bpf__create_skeleton(obj))
    goto err;

  data.name = obj->skeleton->name;
  data.data_sz = obj->skeleton->data_sz;
  base64_data = base64_encode((const unsigned char *)obj->skeleton->data, data.data_sz, &base64_len);
  data.data = (const char*)base64_data;
  for (int i = 0; i < obj->skeleton->map_cnt; i++)
  {
    data.maps_name.push_back(obj->skeleton->maps[i].name);
  }
  for (int i = 0; i < obj->skeleton->prog_cnt; i++)
  {
    data.progs_name.push_back(obj->skeleton->progs[i].name);
  }
  std::cout << data.to_json();
```

the exec ebpf code part:

```c
/* Load and verify BPF application */
	struct ebpf_update_data ebpf_data;
	ebpf_data.from_json_str(json_str);
	skel = update_bpf__open_from_json(ebpf_data);
	if (!skel) {
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	/* Load & verify BPF programs */
	err = update_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}
```

with the http API of eunomia, we can hot update the ebpf code using POST json to this program:

(hot update example is not enabled by default, so you may need to add it in config file.)

```sh
sudo ./eunomia server
./client 127.0.0.1:8527
```
