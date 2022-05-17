/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#include "syscall_tracker.h"

static struct syscall_env syscall_env;

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level == LIBBPF_DEBUG && !syscall_env.verbose)
		return 0;
	return vfprintf(stderr, format, args);
}

static volatile bool exiting = false;

static void sig_handler(int sig)
{
	exiting = true;
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
	const struct syscall_event *e = data;
	struct tm *tm;
	char ts[32];
	time_t t;

	time(&t);
	tm = localtime(&t);
	strftime(ts, sizeof(ts), "%H:%M:%S", tm);

	printf("%-8s %-16s %-7d %-7d [%lu] %u\n",
		   ts, e->comm, e->pid, e->ppid, e->mntns, e->syscall_id);

	return 0;
}

int main(int argc, char **argv)
{
	/* Cleaner handling of Ctrl-C */
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	/* Process events */
	printf("%-8s %-5s %-16s %-7s %-7s %s\n",
		   "TIME", "EVENT", "COMM", "PID", "PPID", "SYSCALL_ID");

	syscall_env.exiting = &exiting;
	return start_syscall_tracker(handle_event, libbpf_print_fn, syscall_env);
}
