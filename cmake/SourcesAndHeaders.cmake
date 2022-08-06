set(sources
    src/libbpf_print.cpp
    src/process.cpp
    src/container.cpp
    src/promethues_server.cpp
    src/files.cpp
    src/ipc.cpp
    src/tcp.cpp
    src/syscall.cpp
    src/eunomia_core.cpp
    src/sec_analyzer.cpp
    src/config.cpp
    src/myseccomp.cpp
    src/tracker_alone.cpp
    src/http_server.cpp
    src/tracker_integrations/oomkill.cpp
    src/tracker_integrations/tcpconnlat.cpp
    src/tracker_integrations/capable.cpp
    src/tracker_integrations/memleak.cpp
    src/tracker_integrations/mountsnoop.cpp
    src/tracker_integrations/sigsnoop.cpp
    src/tracker_integrations/opensnoop.cpp
    src/tracker_integrations/bindsnoop.cpp
    # src/tracker_integrations/syscount.cpp
    # src/tracker_integrations/funclatency.cpp
    src/btf_helpers.c
    src/trace_helpers.c
    src/uprobe_helpers.c
)

set(exe_sources
		src/main.cpp
		${sources}
)

set(headers
    bpftools/container/container_tracker.h
    bpftools/process/process_tracker.h
    bpftools/ipc/ipc_tracker.h
    bpftools/syscall/syscall_tracker.h
    bpftools/tcpconnect/tcp_tracker.h
    bpftools/oomkill/oom_tracker.h
    bpftools/tcpconnlat/tcpconnlat_tracker.h
    bpftools/capable/capable_tracker.h
    bpftools/memleak/memleak_tracker.h
    bpftools/memleak/mountsnoop_tracker.h
    bpftools/memleak/sigsnoop_tracker.h
    bpftools/memleak/opensnoop_tracker.h
    bpftools/memleak/bindsnoop_tracker.h
    # bpftools/memleak/syscount_tracker.h
    # bpftools/memleak/funclatency_tracker.h
    include/eunomia/myseccomp.h
)

set(skel_includes
    bpftools/ipc/.output
    bpftools/process/.output
    bpftools/tcpconnect/.output
    bpftools/syscall/.output
    bpftools/container/.output
    bpftools/files/.output
    bpftools/oomkill/.output
    bpftools/tcpconnlat/.output
    bpftools/capable/.output
    bpftools/memleak/.output
    bpftools/mountsnoop/.output
    bpftools/sigsnoop/.output
    bpftools/opensnoop/.output
    bpftools/bindsnoop/.output
    # bpftools/syscount/.output
    # bpftools/funclatency/.output
)

set(test_sources
    src/process_test.cpp
    src/container_test.cpp
    src/prometheus_test.cpp
    src/files_test.cpp
    src/seccomp_test.cpp
    src/oom_test.cpp
    src/logger_test.cpp
    src/sec_analyzer_test.cpp
    src/config_test.cpp
)
