## Funclatency工具讲解

### 背景
Funclatency可以获取函数的执行时长

### 实现原理
Funclatency定义了kprobe和kretprobe，分别作用于函数被执行前和函数退出后。当调用了函数时，Funclatency会
进行打点计时操作，将pid和时间点数据存入map中。在函数返回时，Funclatency会再进行一次打点计时操作，根据pid
从map中找到对应的进入时间，通过计算差值得到函数的执行时长。

### Eunomia中使用方式
