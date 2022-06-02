# Usage

## command line usage example


### files

You can simply use eunomia to run a single ebpf tracker, for example:

```
sudo eunomia run files
```

will trace all files read or write in the system at a defaut interval of 3s, and print the result:

```sh
[2022-05-28 11:23:10.699] [info] start eunomia...
[2022-05-28 11:23:10.699] [info] start ebpf tracker...
[2022-05-28 11:23:10.699] [info] start prometheus server...
[2022-05-28 11:23:10.699] [info] press 'Ctrl C' key to exit...
[2022-05-28 11:23:13.785] [info] pid    read_bytes      read count      write_bytes     write count     comm    type    tid     filename
[2022-05-28 11:23:13.785] [info] 34802  2048            1               0               0               ps      R       34802   status
[2022-05-28 11:23:13.785] [info] 34802  2048            1               0               0               ps      R       34802   stat
[2022-05-28 11:23:13.785] [info] 34802  2048            1               0               0               ps      R       34802   status
....
```

For json format, you can use:

```
sudo eunomia run files --fmt json
```

and get:

```
...
{"comm":"ps","filename":"cmdline","pid":161093,"read_bytes":262103,"reads":2,"tid":161093,"type":82,"write_bytes":0,"writes":0},{"comm":"ps","filename":"cmdline","pid":161093,"read_bytes":262051,"reads":2,"tid":161093,"type":82,"write_bytes":0,"writes":0},{"comm":"ps","filename":"stat","pid":161111,"read_bytes":2048,"reads":1,"tid":161111,"type":82,"write_bytes":0,"writes":0},
...
```

we also have csv format:

```
sudo eunomia run files --fmt csv
```

for results:

```
pid,read_bytes,read_count,write_bytes,write count,comm,type,tid,filename
161711,2048,1,0,0,ps,R,161711,status
161711,2048,1,0,0,ps,R,161711,stat
161711,131072,1,0,0,ps,R,161711,cmdline
161711,2048,1,0,0,ps,R,161711,stat
161711,131072,1,0,0,ps,R,161711,cmdline
816,1024,1,0,0,vmtoolsd,R,816,uptime
161711,2048,1,0,0,ps,R,161711,status
161711,262123,2,0,0,ps,R,161711,cmdline
161711,2048,1,0,0,ps,R,161711,stat
....
```

### process

Print plant process events.

command:
```
sudo eunomia run process
```

result:
```
[2022-06-02 03:21:34.652] [info] pid    ppid    cgroup_id       user_namespace_id       pid_namespace_id        mount_namespace_idexit_code/comm  duration_ns/filename
[2022-06-02 03:21:34.652] [info] 173002 62485   10911           4026531837              4026531836              4026531840       sh               /bin/sh
[2022-06-02 03:21:34.655] [info] 173003 173002  10911           4026531837              4026531836              4026531840       which            /usr/bin/which
[2022-06-02 03:21:34.659] [info] 173003 173002  10911           4026531837              4026531836              4026531840       03838767
[2022-06-02 03:21:34.660] [info] 173002 62485   10911           4026531837              4026531836              4026531840       08189018
[2022-06-02 03:21:34.667] [info] 173004 62485   10911           4026531837              4026531836              4026531840       sh               /bin/sh
[2022-06-02 03:21:34.670] [info] 173005 173004  10911           4026531837              4026531836              4026531840       ps               /usr/bin/ps
[2022-06-02 03:21:34.707] [info] 173006 9428    2991            4026531837              4026531836              4026531840       git              /usr/bin/git
[2022-06-02 03:21:34.749] [info] 173007 173006  2991            4026531837              4026531836              4026531840       git              /usr/lib/git-core/git
```

### tcp

test:

```
sudo bin/Debug/eunomia run tcpconnect
```

result:

```
[2022-06-02 07:48:24.527] [info] uid    pid    task             af src                  dst                  dport 
[2022-06-02 07:48:24.527] [info]   1000   5943 code              4 192.168.187.130      13.69.239.74            443
[2022-06-02 07:48:27.565] [info]   1000  51677 prometheus        4 127.0.0.1            127.0.0.1              8080
[2022-06-02 07:48:30.367] [info]   1000  51677 prometheus        4 127.0.0.1            127.0.0.1              8528
[2022-06-02 07:48:33.522] [info]   1000  51677 prometheus        4 127.0.0.1            127.0.0.1              9091
[2022-06-02 07:48:42.563] [info]   1000  51677 prometheus        4 127.0.0.1            127.0.0.1              8080
[2022-06-02 07:48:43.746] [info]   1000  33953 Socket Thread     4 127.0.0.1            127.0.0.1              3000
[2022-06-02 07:48:43.758] [info]   1000  33953 Socket Thread     4 127.0.0.1            127.0.0.1              3000
[2022-06-02 07:48:45.367] [info]   1000  51677 prometheus        4 127.0.0.1            127.0.0.1              8528
[2022-06-02 07:48:46.516] [info]    130   5453 grafana-server    4 127.0.0.1            127.0.0.1              9090
```

### syscall

```
sudo bin/Debug/eunomia run syscall
```

```
[2022-06-02 07:52:11.917] [info]  65871  65866 arch_prctl sleep                1
[2022-06-02 07:52:11.917] [info]  65871  65866 access     sleep                1
[2022-06-02 07:52:11.917] [info]  65871  65866 openat     sleep                1
[2022-06-02 07:52:11.918] [info]  65871  65866 fstat      sleep                1
[2022-06-02 07:52:11.918] [info]  65871  65866 mmap       sleep                1
[2022-06-02 07:52:11.918] [info]  65871  65866 read       sleep                1
[2022-06-02 07:52:11.918] [info]  65871  65866 pread64    sleep                1
[2022-06-02 07:52:11.919] [info]  65871  65866 mprotect   sleep                1
[2022-06-02 07:52:11.919] [info]  65871  65866 munmap     sleep                1
```

## config toml example

```

[trackers]
Enable = ["process", "tcp", "syscall"]
container_id = 7895212
process_id = 100
run_time = 50
fmt = "json"

[exporter]
Enable = ["prometheus"]

[prometheus]
endpoint ="127.0.0.1"
port = 6789

[rules]
Enable = ["bpf_rule", "debug"]

[bpf_rule]
type = "syscall"
name = "Insert-BPF"
syscall = "bpf"
error_message = "BPF program loaded"

[debug]
type = "syscall"
name = "Anti-Debugging"
error_message = "Process uses anti-debugging technique to block debugger"

[seccomp]
allow = ["read","write", "connect"]
```