#include <stdio.h>
#include <stdlib.h>
#include "../../include/eunomia/myseccomp.h"
#include "../../include/eunomia/config.h"

seccomp_config config;

int main()
{
  config.len = 2;
  std::string str[] = {"execve","read"};
  config.ban_syscall[0] = str[0];
  config.ban_syscall[1] = str[1];
  /*
   * ...
   */
  enable_seccomp(config);
  int d;
  scanf("%d",&d);
  //execv("./process_test_Tests",NULL);
  printf("if you can get here,that means seccomp is wrong\n");

  return 0;
}
