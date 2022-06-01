# Prometheus Metrics Description
## Service Metrics
Service metrics are generated from the server-side events, which are used to show the quality of service. 
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

## Process Metrics


## Tcp Connect Metrics

## Syscall Metrics


## files Metrics


## PromQL Example

Here are some examples of how to use these metrics in Prometheus, which can help you understand them faster.

| **Describe** | **PromQL** |
| --- | --- |
| Request counts | `sum(increase(eunomia_run_tracker_total{namespace="$namespace",workload_name="$workload"}[5m])) by(namespace, workload_name)` |
