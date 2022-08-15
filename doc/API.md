# eunomia http API Documentation

## start ebpf tracker

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
"id": "1"
}
```

## stop ebpf tracker

example:
```json
POST /stop
```

result:

```json
{
"status": "ok",
}
```

## list all running ebpf tracker

example:
```
GET /list
```

result:

```json
{
"status": "ok",
"list": []
}
```