## Biopattern工具讲解

### 工具目的
Biopattern 可以统计随机/顺序磁盘IO次数的比例

### 实现原理
Biopattern 的ebpf代码在 tracepoint/block/block_rq_complete 挂载点下实现。在磁盘完成IO请求
后，程序会经过此挂载点。Biopattern 内部存有一张以设备号为主键的哈希表，当程序经过挂载点时, Biopattern
会获得操作信息，根据哈希表中该设备的上一次操作记录来判断本次操作是随机IO还是顺序IO，并更新操作计数。
当用户停止Biopatter后，用户态程序会读取获得的计数信息，并将其输出给用户。

### Eunomia中使用方式
