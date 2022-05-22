set(sources
    src/libbpf_print.cpp
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
  src/tmp_test.cpp
)
