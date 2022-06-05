# Intergration

Describe how to intergrate with other apps.

# prometheus

```sh
sudo docker pull prom/prometheus
cp ./prometheus.yml /etc/prometheus/prometheus.yml

sudo docker run -d --user root -p 9090:9090 -v /etc/prometheus/prometheus.yml:/etc/prometheus/prometheus.yml -v /etc/prometheus/data:/data/prometheus prom/prometheus --config.file="/etc/prometheus/prometheus.yml" --web.listen-address="0.0.0.0:9090"
```

or run locally:

```sh
/prometheus --config.file=prometheus.yml --web.listen-address="0.0.0.0:9090"
```

# grafana

```sh
sudo docker pull grafana/grafana:latest
```

or podman

```sh
sudo podman run --user root -d -p 3000:3000 grafana/grafana:latest
```

see: https://grafana.com/grafana/download

see: https://prometheus.io/docs/visualization/grafana/

or on Ubuntu:

```sh
sudo apt-get install -y adduser libfontconfig1
wget https://dl.grafana.com/enterprise/release/grafana-enterprise_8.5.4_amd64.deb
sudo dpkg -i grafana-enterprise_8.5.4_amd64.deb

sudo /bin/systemctl start grafana-server
```

## Kubernetes