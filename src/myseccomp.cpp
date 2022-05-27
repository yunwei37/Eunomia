#include "eunomia/myseccomp.h"

static int install_syscall_filter(uint32_t syscall_id[], int len)
{
    std::vector<sock_filter> filter_vec = {
  	/* Validate architecture. */
        BPF_STMT(BPF_LD+BPF_W+BPF_ABS,4),
        BPF_JUMP(BPF_JMP+BPF_JEQ,0xc000003e,0,2),
        /* Grab the system call number. */
        BPF_STMT(BPF_LD+BPF_W+BPF_ABS,0),
        /* syscalls. */
        BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_ALLOW)
    };

    /* add ban rules*/
    for (auto i = 0; i < len; i++) {
        // ALLOW SYSCALL only swap RET_KILL and RET_ALLOW
        filter_vec.insert(filter_vec.end()-1,BPF_JUMP(BPF_JMP+BPF_JEQ,syscall_id[i],0,1));
	printf("syscall_id : %d\n",syscall_id[i]);
        filter_vec.insert(filter_vec.end()-1,BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_KILL));
    }
    
    filter_vec.push_back(BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_ALLOW));

    sock_filter filter[filter_vec.size()];
    std::copy(filter_vec.begin(), filter_vec.end(), filter);

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
        .filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(NO_NEW_PRIVS)");
        goto failed;
    }    
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
        perror("prctl(SECCOMP)");
        goto failed;
    }
    return 0;

failed:
    if (errno == EINVAL)
        fprintf(stderr, "SECCOMP_FILTER is not available. :(n");
    return 1;
}

int get_syscall_id(std::string syscall_name) {
    for(int i=0; i<439; i++) {
        if (strcmp(syscall_names_x86_64[i],syscall_name.data()) == 0) return i;
    }   
    return -1;
}

// Enable Seccomp syscall
// param seccomp_config type is defined by include/eunomia/config.h
int enable_seccomp(seccomp_config config) {
    printf("enabled seccomp\n");
    std::vector <uint32_t>syscall_vec;
    for (int i=0; i<config.len; i++) {
	int id = get_syscall_id(config.ban_syscall[i]);
        printf("id : %d\n",id);
	if (id == -1) {
	   printf("%s error\n",config.ban_syscall[i]);    
	   continue;
	}
        syscall_vec.push_back(id);
    }
    uint32_t syscall_id[syscall_vec.size()];
    std::copy(syscall_vec.begin(), syscall_vec.end(), syscall_id);
    if (install_syscall_filter(syscall_id,syscall_vec.size()))
        return 1;

    return 0;
}
