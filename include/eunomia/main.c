#define _GNU_SOURCE 1
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/prctl.h>
#include <seccomp.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <vector>
#include <typeinfo>

#include "seccomp-bpf.h"
//#include "syscall_helper.h"

static int install_syscall_filter(uint32_t syscall_id[], int len)
{
 //  struct sock_filter filter[] = {
        /* Validate architecture. */
 //       VALIDATE_ARCHITECTURE,
        /* Grab the system call number. */
 //       EXAMINE_SYSCALL,
        /* List allowed syscalls. */
 //       KILL_PROCESS,
 //   };
    
    std::vector<sock_filter> filter_1 = {
        /* Validate architecture. */
        BPF_STMT(BPF_LD+BPF_W+BPF_ABS,4),
	BPF_JUMP(BPF_JMP+BPF_JEQ,0xc000003e,0,2),
        /* Grab the system call number. */
        BPF_STMT(BPF_LD+BPF_W+BPF_ABS,0),
        /* syscalls. */
	BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_ALLOW)
    };

    /* add rules*/
    for (auto i = 0; i < len; i++) {
	// add BAN_SYSCALL() 
	// ALLOW SYSCALL only swap RET_KILL and RET_ALLOW
	filter_1.insert(filter_1.end()-1,BPF_JUMP(BPF_JMP+BPF_JEQ,syscall_id[i],0,1));
        filter_1.insert(filter_1.end()-1,BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_KILL));
    }
    filter_1.push_back(BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_ALLOW));

    sock_filter filter[filter_1.size()];
    std::copy(filter_1.begin(), filter_1.end(), filter);

    printf("%ld",(sizeof(filter)/sizeof(filter[0])));

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

int main()
{
    printf("hi~\n");
    uint32_t syscall_id[439];
    for (int i=0; i<439; i++) {
        syscall_id[i] = i;
    }
    if (install_syscall_filter(syscall_id,59))
        return 1;

    return 0;
}

