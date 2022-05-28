#include <stdio.h>
#include <stdlib.h>
#include "../../include/eunomia/myseccomp.h"
#include "../../include/eunomia/config.h"

seccomp_config config;

int main()
{
  config.len = 20;
  std::string str[] = {
      "open",
      "close",
      "fstat",
      "execve",
      "mmap",
      "munmap",
      "uname",
      "arch_prctl",
      "brk",
      "access",
      "readlink",
      "sysinfo",
      "writev",
      "lseek",
      "mprotect",
      "exit_group",
      "read",
      "write",
      "exit",
      "rt_sigreturn"      
  };

  for (auto i = 0; i < config.len; i++) {
      config.allow_syscall[i] = str[i];
  }
  /*
   * ...
   */
//  enable_seccomp_white_list(config);
  int d;
  scanf("%d",&d);
  printf("%d\n if you get the print is equal to your input,congraduation! you are successful to enable seccomp, and don't care about next line print\n",d);
  //execv("./process_test_Tests",NULL);
  printf("if you can get here,that means seccomp is wrong\n");

  return 0;
}
