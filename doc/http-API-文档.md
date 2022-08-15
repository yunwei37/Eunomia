# eunomia http API Documentation

<!-- TOC -->

- [start ebpf tracker](#start-ebpf-tracker)
- [stop ebpf tracker](#stop-ebpf-tracker)
- [list all running ebpf tracker](#list-all-running-ebpf-tracker)

<!-- /TOC -->

## start ebpf tracker

启动一个 ebpf 跟踪器：

- name：指定的跟踪器名称
- export_handlers：需要的处理流水线名称和参数
- args：跟踪器附带的参数，以字符串形式存储；

Eunomia core 会根据 name 来自动选择需要启动的跟踪器。

返回结果：一个 id 标识符，用来标明当前的跟踪器；

example:
```json
POST /start

        {
            "name": "files",
            "export_handlers": [
                {
                    "name": "stdout",
                    "args": [
                        "arg1",
                        "arg2"
                    ]
                }
            ],
            "args": [
                "arg1",
                "arg2"
            ]
        }
```

result
    
```json
{
"status": "ok",
"id": 1
}
```

## stop ebpf tracker

停止一个 ebpf 跟踪器：

- id：指定的跟踪器 id

返回结果：状态

example:

```json
POST /stop
{"id": 1}
```

result:

```json
{
"status": "ok",
}
```

## list all running ebpf tracker

列出当前在运行的所有跟踪器信息

返回结果包含一个数组：

- id: 跟踪器 id
- name：跟踪器名字；

example:
```
GET /list
```

result:

```json
{
"status": "ok",
"list": [
    {
        "id": 1,
        "name": "files",
    }
]
}
```