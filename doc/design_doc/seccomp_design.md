Seccomp(全称：secure computing mode)在2.6.12版本(2005年3月8日)中引入linux内核，将进程可用的系统调用限制为四种：read，write，_exit，sigreturn。最初的这种模式是白名单方式，在这种安全模式下，除了已打开的文件描述符和允许的四种系统调用，如果尝试其他系统调用，内核就会使用SIGKILL或SIGSYS终止该进程。Seccomp来源于Cpushare项目，Cpushare提出了一种出租空闲linux系统空闲CPU算力的想法，为了确保主机系统安全出租，引入seccomp补丁，但是由于限制太过于严格，当时被人们难以接受。

尽管seccomp保证了主机的安全，但由于限制太强实际作用并不大。在实际应用中需要更加精细的限制，为了解决此问题，引入了Seccomp – Berkley Packet Filter(Seccomp-BPF)。Seccomp-BPF是Seccomp和BPF规则的结合，它允许用户使用可配置的策略过滤系统调用，该策略使用Berkeley Packet Filter规则实现，它可以对任意系统调用及其参数（仅常数，无指针取消引用）进行过滤。Seccomp-BPF在3.5版（2012年7月21日）的Linux内核中（用于x86 / x86_64系统）和Linux内核3.10版（2013年6月30日）被引入Linux内核。

seccomp在过滤系统调用(调用号和参数)的时候，借助了BPF定义的过滤规则，以及处于内核的用BPF language写的mini-program。Seccomp-BPF在原来的基础上增加了过滤规则，大致流程如下：

<img src="./imgs/seccomp.png" weight=100% height=100%>
