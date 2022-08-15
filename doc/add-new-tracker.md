# 教程：添加新的自定义检查器

## 在 bpftools 中添加新的检查器代码

在 bpftools 中，每个文件夹都是一个基于 libbpf-bootstrap 的简单脚手架，包含了 ebpf 子程序所需的 ebpf 代码一些 C/C++ 辅助代码；

每个子文件夹都可以被编译为一个独立的 tool 二进制，也可以被 Eunomia 作为头文件包含，并被编译到 Eunomia 的二进制中。

以 tcpconnlat 为例，这是一个从 bcc/libbpf-tools 中移植过来并且修改过后的 ebpf 追踪器，在 `bpftools/tcpconnlat` 目录中。它的目录结构如下：

```
bpftools/tcpconnlat
├── bits.bpf.h
├── btf_helpers.c
├── btf_helpers.h
├── Makefile
├── maps.bpf.h
├── tcpconnlat.bpf.c
├── tcpconnlat.c
├── tcpconnlat.h
├── tcpconnlat_tracker.h
├── trace_helpers.c
├── trace_helpers.h
├── uprobe_helpers.c
└── uprobe_helpers.h
```

其中，helper 文件是从 libbpf-tools 中直接移植的 C 辅助函数，作为可选项，也可以不使用。 Makefile 是基于 libbpf-bootstrap 的 Makefile 模板，用于编译和生成 libbpf 头文件和 ebpf 代码。真正需要编写的只有四个文件：

- tcpconnlat.bpf.c
- tcpconnlat.c
- tcpconnlat.h
- tcpconnlat_tracker.h

其中 tcpconnlat.h 是包含有数据结构定义的头文件， ebpf 代码在 tcpconnlat.bpf.c 中。这也是 libbpf-bootstrap 和 bcc/libbpf-tools 的默认组织方式。需要注意的是，原先的在 .c 中的代码需要被移到 tcpconnlat_tracker.h 中，将函数和变量声明为 static，然后将 main 函数的名字改为 start_tcpconnlat 这样的形式：

```c
static int start_tcpconnlat(int argc, char **argv)
{
}
```

在 tcpconnlat.c 中，我们直接调用 tcpconnlat_tracker.h 中的 start_tcpconnlat 函数，它就可以类似 libbpf-bootstrap 一样被编译为单独的二进制工具使用，也能产生我们所需的 libbpf 头文件：

```c
#include "tcpconnlat_tracker.h"

int main(int argc, char **argv)
{
	return start_tcpconnlat(argc, argv);
}
```

这之前我们只是在我们的项目里面构建了一个 libbpf 的二进制程序，它可以单独运行，也可以用来测试我们的ebpf实现。接下来是如何将其移植到 Eunomia 中。

## 修改 include\eunomia\tracker_integrations.h

在 include\eunomia\tracker_integrations.h 中，添加新的检查器的类定义：

```cpp
struct tcpconnlat_tracker final : public tracker_alone_base
{
  tcpconnlat_tracker(config_data config) : tracker_alone_base(config)
  {
  }
  static std::unique_ptr<tcpconnlat_tracker> create_tracker_with_default_env(tracker_event_handler handler);
  static std::unique_ptr<tcpconnlat_tracker> create_tracker_with_args(
      tracker_event_handler handler,
      const std::vector<std::string> &args);
};
```

在 src\tracker_integrations 中，新建一个 tcpconnlat.cpp 文件：

```cpp
#include "eunomia/tracker_integrations.h"
#include "spdlog/spdlog.h"

extern "C"
{
#include "tcpconnlat/tcpconnlat_tracker.h"
}

std::unique_ptr<tcpconnlat_tracker> tcpconnlat_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "tcpconnlat";
  config.env = tracker_alone_env{ .main_func = start_tcpconnlat };
  return std::make_unique<tcpconnlat_tracker>(config);
}

std::unique_ptr<tcpconnlat_tracker> tcpconnlat_tracker::create_tracker_with_args(
    tracker_event_handler handler,
    const std::vector<std::string> &args)
{
  auto tracker = tcpconnlat_tracker::create_tracker_with_default_env(handler);
  if (tracker)
  {
    tracker->current_config.env.process_args = args;
  }
  return tracker;
}
```

这一块基本上是模板代码，之后也许可以改为利用宏等工具自动生成；

在 src\eunomia_core.cpp 中注册当前追踪器：

```cpp
    ...
  else if (config.name == "tcpconnlat")
  {
    return core_tracker_manager.start_tracker(create_default_tracker<tcpconnlat_tracker>(config), config.name);
  }
  ...
```

## 修改 cmake 进行编译

在 cmake\SourcesAndHeaders.cmake 中，添加：

```cmake
set(sources
...
    src/tracker_integrations/tcpconnlat.cpp
...
)

set(exe_sources
		src/main.cpp
		${sources}
)

set(headers
...
    bpftools/tcpconnlat/tcpconnlat_tracker.h
...
)

set(skel_includes
...
    bpftools/tcpconnlat/.output
...
)

```

即可完成添加新的 tracker。
