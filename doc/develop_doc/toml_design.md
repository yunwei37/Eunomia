# toml design

- trackers
- exporter
- rules

```toml
[trackers]
enable = ["process", "tcp", "syscall"]

[process]
min_duration = 20
cgroup_id = 1

[exporter]
enable = ["prometheus", "json", "influxdb"]

[prometheus]
endpoint ="127.0.0.1"
port = 6789

[rules]
enable = ["rule1", "rule2"]

[rule1]
type = "syscall"
name = "bpf..."
error_message = "error: bpf exec"

[rule2]
type = "file"
name = "/proc/xxx

type = "syscall"
sycall = "xxxxx"

[seccomp]
allow = ["read","write", "connect"]

```
