## json fields

process

```json
{{"type", "process"},
{"time", get_current_time()},
{"pid", e.common.pid},
{"ppid", e.common.ppid},
{"cgroup_id", e.common.cgroup_id},
{"user_namespace_id", e.common.user_namespace_id},
{"pid_namespace_id", e.common.pid_namespace_id},
{"mount_namespace_id", e.common.mount_namespace_id},
{"exit_code", e.exit_code},
{"duration_ns", e.duration_ns},
{"comm", e.comm},
{"filename", e.filename},
{"exit_event", e.exit_event}};
```

syscall

```json
{{"type", "syscall"},
{"time", get_current_time()},
{"pid", e.pid},
{"ppid", e.ppid},
{"mount_namespace_id", e.mntns},
{"syscall_id", e.syscall_id},
{"comm", e.comm},
{"occur_times", e.occur_times}};
```