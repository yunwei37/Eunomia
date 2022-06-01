# Usage

## command line usage example

### files


You can simply use eunomia to run a single ebpf tracker, for example:

```
./eunomia run files
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


## config toml example