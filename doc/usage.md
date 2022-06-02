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

### 

## config toml example