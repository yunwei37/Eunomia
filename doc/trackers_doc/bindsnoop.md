## Bindsnoop 工具讲解

### 背景
Bindsnoop 会跟踪操作socket端口绑定的内核函数，并且在可能会影响端口绑定的系统调用发生之前，打印
现有的socket选项集

### 实现原理
Bindsnoop 通过kprobe实现。其主要挂载点为 inet_bind 和 inet6_bind。inet_bind 为处理 IPV4 类型
socket 端口绑定系统调用的接口，inet6_bind 为处理IPV6类型 socket 端口绑定系统调用的接口。
当系统试图进行socket端口绑定操作时会经过这两个挂载点，此时Bindsnoop会记录对应的tid和socket信息，
并将其存入map中。当用户停止该工具时，其用户态代码会读取存入的数据并按要求打印。

### Eunomia中使用方式
