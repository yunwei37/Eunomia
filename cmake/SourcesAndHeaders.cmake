set(sources
    src/libbpf_print.cpp
    src/process.cpp
    src/container.cpp
    src/promethues_server.cpp
    src/files.cpp
    src/eunomia_core.cpp
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
    bpftools/tcp/tcp_tracker.h
)

set(skel_includes
    bpftools/ipc/.output
    bpftools/process/.output
    bpftools/tcp/.output
    bpftools/syscall/.output
    bpftools/container/.output
    bpftools/files/.output
)


set(test_sources
    src/process_test.cpp
    src/container_test.cpp
    src/prometheus_test.cpp
    src/files_test.cpp
)
