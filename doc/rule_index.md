# 安全告警规则

目前安全告警部分还未完善，只有一个框架和 demo，我们需要对更多的安全相关规则，以及常见的容器安全风险情境进行调研和完善，然后再添加更多的安全分析。

## 设计思路

请参考：[design_doc\sec-analyzer.md](design_doc\sec-analyzer.md)

## 规则

> 目前我们还没来得及整理更多有价值的规则，因此这里只有一点 demo 例子。

| Name | Description | Tags
| --- | --- | --- |
BPF program loaded | load bpf program in the container | "linux", "container"
