# Intergration

Describe how to intergrate with other apps.

# prometheus

sudo docker pull prom/prometheus
cp ./prometheus.yml /etc/prometheus/prometheus.yml

sudo docker run -d --user root -p 9090:9090 -v /etc/prometheus/prometheus.yml:/etc/prometheus/prometheus.yml -v /etc/prometheus/data:/data/prometheus prom/prometheus --config.file="/etc/prometheus/prometheus.yml" --web.listen-address="0.0.0.0:9090"

# grafana

sudo docker pull grafana/grafana:latest

## Kubernetes