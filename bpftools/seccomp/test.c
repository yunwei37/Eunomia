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

int main() {
    struct sock_filter filter[] = {
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS,4),
    BPF_JUMP(BPF_JMP+BPF_JEQ,0xc000003e,0,2),
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS,0),
//    BPF_JUMP(BPF_JMP+BPF_JEQ,59,0,1),
//    BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_KILL),
    BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_ALLOW),
    };
    struct sock_fprog prog = {
    .len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
    .filter = filter,
    };
    prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0);
    prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&prog);
    printf("start!n");
    system("id");
    return 0;
}
