// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
#ifndef CONTAINER_TRACKER_H
#define CONTAINER_TRACKER_H

#include <argp.h>
#include <bpf/libbpf.h>
#include <unistd.h>
#include <bpf/bpf.h>
#include "container.h"
#include "container.skel.h"

static const char hex_dec_arr[] = {'0','1','2','3','4','5','6','7','8',
                                    '9','a','b','c','d','e','f'};

struct container_env
{
    bool verbose;
    volatile bool *exiting;
};

static int long_hex_to_str(unsigned long num, char *arr, int len) {
    int i = 0;
    while(num > 0) {
        int res = num % 16;
        arr[i++] = hex_dec_arr[res];
        num /= 16;
        if(i >= len) {
            i = num == 0 ? i : -i;
            break;
        }
    }
    arr[len] = '\0';
    return i;
}

static void init_container_map(struct container_bpf *skel, int processes_fd) {
	char *cmd = "./library/container/namespace.sh > ./library/container/container.txt";
	system(cmd);

	/* read from the output */
    FILE* fp = fopen("./library/container/container.txt","r");
	if(fp == NULL) {
        fprintf(stderr, "Fail to open container.txt\n");
        exit(0);
    }
	char content_line[150];
    /* ignore the first two line*/
    fgets(content_line, 150, fp);
    fgets(content_line, 150, fp);

	unsigned long container_id, cgroup_id, ipc_id, mnt_id, net_id, pid_id, user_id, uts;
    char name[50];
    pid_t pid;

	/* read from txt */
    while(fscanf(fp, "%lx %s %d %*s %lu %lu %lu %lu %lu %lu %lu\n", 
                &container_id, name, &pid, 
                &cgroup_id, &ipc_id, &mnt_id, 
                &net_id, &pid_id, &user_id, &uts) == 10) {
        //printf("%lx %lu %lu\n", container_id, cgroup_id, mnt_id);
        char container_id_ch[16];
        int len;
        if((len = long_hex_to_str(container_id, container_id_ch, 16)) < 0) {
            fprintf(stderr, "id is to long\n");
            exit(0);
        }
        /* generate cmd */
        char cmd[100] = "docker top ";
        char *assit = " > ./library/container/top.txt";
        int i = strlen(cmd);
        while(len >= 0) {
            cmd[i++] = container_id_ch[--len];
        }
        cmd[i] = '\0';
        strcat(cmd, assit);
        // printf("%s\n", cmd);
        system(cmd);
        

        FILE* f = fopen("./library/container/top.txt","r");
        if(f == NULL) {
            exit(0);
        }
        /* read the process detail from the assist.txt and ignore the first line */
        fgets(content_line, 150, f); 
        char uid[10];
        pid_t c_pid, ppid;
        while(fscanf(f,"%s %d %d %*[^\n]\n", uid, &c_pid, &ppid) == 3) {
            // printf("%d, %d\n", c_pid, ppid);
			struct container_event data = {
				.container_id = container_id,
				.pid = c_pid,
				.ppid = ppid,
			};
			bpf_map_update_elem(processes_fd, &c_pid, &data, BPF_ANY);
        }

    }
    

}

static int start_container_tracker(ring_buffer_sample_fn handle_event, libbpf_print_fn_t libbpf_print_fn, struct container_env env)
{
	struct ring_buffer *rb = NULL;
	struct container_bpf *skel;
    int err;
    int processes_fd;
    int to_lookup, next_key;
    struct container_event container_value;

	/* Parse command line arguments */
	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);
	/* Load and verify BPF application */
	
    skel = container_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}
    
    /* Load & verify BPF programs */
	err = container_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}
    
    /* execute shell to generate container data */
	processes_fd = bpf_map__fd(skel->maps.processes);
	init_container_map(skel, processes_fd);

	/* Set up ring buffer polling */
	rb = ring_buffer__new(bpf_map__fd(skel->maps.events), handle_event, NULL, NULL);
	if (!rb) {
		err = -1;
		fprintf(stderr, "Failed to create ring buffer\n");
		goto cleanup;
	}

    /* Container events */
	printf("%-10s %-15s %s\n",
	    "PID", "PARENT_PID", "CONTAINER_ID");
	to_lookup = 0;
	next_key = 0;
	while (!bpf_map_get_next_key(processes_fd, &to_lookup, &next_key))
	{
		bpf_map_lookup_elem(processes_fd, &next_key, &container_value);
		printf("%-10u %-15u %lu \n", container_value.pid, container_value.ppid, container_value.container_id);
		to_lookup = next_key;
	}
	
    while (!*env.exiting) {
		if (err == -EINTR) {
			err = 0;
			break;
		}
		if (err < 0) {
			printf("Error polling perf buffer: %d\n", err);
			break;
		}
	}

cleanup:
	/* Clean up */
	ring_buffer__free(rb);
	container_bpf__destroy(skel);
	return err < 0 ? -err : 0;
}


#endif