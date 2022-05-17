#ifndef OUTPUT_H
#define OUTPUT_H

#include <time.h>

static void
print_table_header(const char *custom_headers[], bool is_csv)
{
    if (is_csv)
    {
        printf("time,pid,ppi,cgroup_id,user_namespace_id,pid_namespace_id,mount_namespace_id");
    }
    else
    {
        printf("%s\t\t%s\t%s\t%s\t%s\t%s\t%s",
               "time", "pid", "ppid", "cgroup_id", "user_namespace_id", "pid_namespace_id", "mount_namespace_id");
    }
    while (*custom_headers)
    {
        if (is_csv)
        {
            printf(",%s", *custom_headers);
        }
        else
        {
            printf("\t%s", *custom_headers);
        }
        custom_headers++;
    }
    printf("\n");
}

static void print_basic_info(const struct process_event *e, bool is_csv)
{
    struct tm *tm;
    char ts[32];
    time_t t;

    if (!e)
    {
        return;
    }
    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);
    /* format: [time] [pid] [ppid] [cgroup_id] [user_namespace_id] [pid_namespace_id] [mount_namespace_id] */
    if (is_csv)
    {
        printf("%s,%d,%d,%lu,%u,%u,%u",
               ts, e->common.pid, e->common.ppid, e->common.cgroup_id, e->common.user_namespace_id, e->common.pid_namespace_id, e->common.mount_namespace_id);
    }
    else
    {
        printf("%-8s\t%-7d\t%-7d\t%lu\t\t%u\t\t%u\t\t%u\t\t",
               ts, e->common.pid, e->common.ppid, e->common.cgroup_id, e->common.user_namespace_id, e->common.pid_namespace_id, e->common.mount_namespace_id);
    }
}

#endif