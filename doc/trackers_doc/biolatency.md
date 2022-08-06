## Funclatency工具讲解

### 工具目的
Biolatency 可以统计在该工具运行后系统中发生的IO事件个数，并且计算IO事件在不同时间段内的分布情况，以
直方图的形式展现给用户。

### 实现原理
Biolatency 主要通过 tracepoint 实现，其在 block_rq_insert, block_rq_issue, 
block_rq_complete 挂载点下设置了处理函数。在 block_rq_insert 和 block_rq_issue 挂载点下，
Biolatency 会将IO操作发生时的rq和时间计入map中。在block_rq_complete 挂载点下，Biolatency 会根据
rq 从map中读取上一次操作发生的时间，然后计算与当前时间的差值来判断其在直方图中存在的区域，将该区域内的IO操作
计数加一。当用户中止程序时，用户态程序会读取直方图map中的数据，并打印呈现。

### Eunomia中使用方式
