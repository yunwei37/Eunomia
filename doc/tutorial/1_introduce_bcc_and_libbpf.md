## 如何使用eBPF编程
&ensp;&ensp;&ensp;&ensp;eBPF在Linux各个版本的内核中均有支持。
在Linux的源码包的`samples/bpf/`目录下，有大量Linux提供的原生eBPF
样例代码。一个典型的原生eBPF程序具有`*_kern.c`和`*_user.c`两个文件，
`*_kern.c`中书写在内核中的挂载点以及处理函数，`*_user.c`中书写用户态代码，
完成内核态代码注入以及与用户交互的各种任务。更为详细的教程可以参考[该视频](https://www.bilibili.com/video/BV1f54y1h74r?spm_id_from=333.999.0.0)
由于原生的eBPF程序书写起来较为复杂，并且存在诸多不便，目前我们的eBPF开发
大多基于一些工具，比如：
- BCC
- BPFtrace
- libbpf-bootstrap
等等，接下来我们将介绍其中较为典型的两种工具BCC和libbpf-bootstrap。
### BCC
&ensp;&ensp;&ensp;&ensp;BCC全称为BPF Compiler Collection，该项目包含了完整的编写、编译、和
加载BPF程序的工具链，以及用于调试和诊断性能问题的工具。自2015年发布以来，BCC经过上百位贡献者地不断完善后，
目前已经包含了大量随时可用的跟踪工具。[其官方项目库](https://github.com/iovisor/bcc/blob/master/docs/tutorial.md)
提供了一个方便上手的教程，用户可以快速地根据教程完成BCC入门。
&ensp;&ensp;&ensp;&ensp;BCC使用Python和Lua语言作为入口进行编程。

### libbpf