# Prometheus Metrics Description

## Process Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_bserved_process_start` | Counter | Number of observed process start |
| `eunomia_observed_process_end` | Counter | Number of observed process end |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `node` | worker-1 | Node name represented in Kubernetes cluster |
| `mount_namespace` | default | Namespace of the pod |
| `container_name` | Ubuntu | The name of the container |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `pid` | 12344 | The pid of the running process |
| `comm` | ps | The command of the running process |
| `filename` | /usr/bin/ps | The exec file name |
| `exit_code` | 0 | The exit code |
| `duration_ms` | 375 | The running time |


## files Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_files_read_count` | Counter | Number of observed files read count |
| `eunomia_observed_files_write_count` | Counter | Number of observed files write count |
| `eunomia_observed_files_write_bytes` | Counter | Number of observed files read bytes |
| `eunomia_observed_files_read_bytes` | Counter | Number of observed files write bytes |

## Tcp Connect Metrics

## Syscall Metrics

## Service Metrics

Service metrics are generated from the  eunomia server-side events, which are used to show the quality of service.

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_run_tracker_total` | Counter | Total number of running trackers |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `node` | worker-1 | Node name represented in Kubernetes cluster |
| `namespace` | default | Namespace of the pod |
| `container` | api-container | The name of the container |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `ip` | 10.1.11.23 | The IP address of the entity |
| `port` | 80 | The listening port of the entity |


### Notes
**Note 1**: xxxx

**Note 2**: xxxx

## PromQL Example

Here are some examples of how to use these metrics in Prometheus, which can help you understand them faster.

| **Describe** | **PromQL** |
| --- | --- |
| Request counts | `sum(increase(eunomia_observed_tcp_v4_count{}[1m])) by(task)` |
| read rate | `sum(rate(eunomia_observed_files_read_bytes{}[1m])) by(comm)` |
| write rate | `sum(rate(eunomia_observed_files_write_count{}[1m])) by(comm)` |