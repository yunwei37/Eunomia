#ifndef __BOOTSTRAP_H
#define __BOOTSTRAP_H

#define TASK_COMM_LEN 16
#define MAX_FILENAME_LEN 127
#define CONTAINER_ID_LEN 127

struct event {
    // container this process belongs to
	unsigned long container_id; // container_id
	// pid in host
	int hpid;   
	// pid in container
	int cpid;
};

/**
 * container_t - Represents per-container metadata
 * @container_id: id of this container
 * @mnt_ns_id: ns id of the container's mount namespace
 * @pid_ns_id: ns id of the container's pid namespace
 * @user_ns_id: ns id of the container's user namespace
 * @refcount: number of processes that are running under this container
 */
typedef struct {
    // bpfcontain's version of a container id,
    // also used as a key into the map of containers
    unsigned long long container_id;
    // the mount namespace id of this container
    unsigned int mnt_ns_id;
    // the pid namespace id of this container
    unsigned int pid_ns_id;
    // the user namespace id of this container
    unsigned int user_ns_id;
    // reference count of the container (how many tasks are running inside it)
    // this should only be incremented and decremented atomically
    unsigned int refcount; 
    // Is the container in privileged mode?
    unsigned char privileged : 1;
} container_t;



#endif /* __BOOTSTRAP_H */
