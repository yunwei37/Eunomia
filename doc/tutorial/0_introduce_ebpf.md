# eBPF开发教程

## 什么是eBPF
&ensp;&ensp;&ensp;&ensp;Linux 内核一直是实现监控/可观测性、网络和安全功能的理想地方。
不过很多情况下这并非易事，
因为这些工作需要修改内核源码或加载内核模块，最终实现形式是在已有的层层抽象之上叠加新的
抽象。而eBPF是一项革命性技术，它能在内核中运行沙箱程序（sandbox programs），而无
需修改内核源码或者加载内核模块。

### 起源
&ensp;&ensp;&ensp;&ensp;eBPF的雏形是BPF(Berkeley Packet Filter, 伯克利包过滤器)。BPF于
1992年被提出，其初衷是提供一种通用数据过滤包的方法。早期的BPF应用是由用户空间注入到内核的一段简单
字节码组成。这段字节码会被附着到一个套接字上，在接收到的每个包上运行，对其进行检验 以避免包中的数据导致
内核崩溃或者是其他安全问题。传统的BPF是32位架构，主要针对包解析场景设计。数年后，它被移植到了Linux
上并在上层的`libcap`和`tcpdump`等应用中得到了使用是一个性能卓越的工具。
&ensp;&ensp;&ensp;&ensp;2013年，Alexei Starovoitov对BPF进行彻底地改造，改造后的BPF被命名为
eBPF(extended BPF)。eBPF相较于BPF更为灵活且具有更大的可编程性，这直接带来了一些新的使用场景，
比如比如tracing，KCM等。同时，eBPF相较于BPF，其JIT编译器也得到了升级，解释器也被替换，这直接使得其
具有达到平台原生的执行性能的能力。

### 执行逻辑
&ensp;&ensp;&ensp;&ensp;eBPF可以认为是一个基于寄存器的微型"虚拟机"，使用自定义的64位RISC指令集，它可以以
在Linux内核中，以一种安全可控的方式运行本机编译的eBPF程序并且访问内核函数和内存的子集。
的方式加载小型程序。eBPF程序是由事件驱动的，我们在程序中需要提前确定程序地执行点。我们将写好的eBPF程序经过编译后
注入内核中，当系统发生了我们在程序指定的事件时，注入的程序就会被触发，拦截该事件并进行按照我们既定的方式去处理。
为了确保我们写入的程序 本身不会对内核产生较大影响，编译好的字节码在注入内核之前会被eBPF校验器严格地检查。
其整体执行逻辑由图1所示。
![](../imgs/exec_logic.png)

### 架构
#### 寄存器设计
&ensp;&ensp;&ensp;&ensp;eBPF有11个寄存器，分别是R0~R10，每个寄存器均是64位大小，有相应的32位
子寄存器，其指令集是固定的64位宽。
#### 指令编码格式
&ensp;&ensp;&ensp;&ensp;eBPF指令编码格式为：
- 8 bit: 存放真实指令码
- 4 bit: 存放指令用到的目标寄存器号
- 4 bit: 存放指令用到的源寄存器号
- 16 bit: 存放偏移量，具体作用取决于指令类型
- 32 bit: 存放立即数

更为详细的架构信息可以前往
进行深一步阅读

参考文章
[A thorough introduction to eBPF](https://lwn.net/Articles/740157/)
[bpf简介](https://www.collabora.com/news-and-blog/blog/2019/04/05/an-ebpf-overview-part-1-introduction/)
[bpf架构知识](https://www.collabora.com/news-and-blog/blog/2019/04/15/an-ebpf-overview-part-2-machine-and-bytecode/)


