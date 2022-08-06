# build run env
FROM ubuntu:22.04

ENV TZ = Asia/Shanghai

WORKDIR /usr/local/src

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install -y net-tools
RUN apt-get install -y libelf-dev
RUN apt-get install -y curl libssl-dev libcurl4-openssl-dev libcurl4

EXPOSE 80

COPY ./ /usr/local/src

ENTRYPOINT ["/bin/bash","-l","-c"]
# ./eunomia server --prometheus --config test.toml && ./prometheus --config.file=./prometheus.yml && ./grafana-server --config=./defaults.ini
CMD ["./eunomia server --prometheus --config test.toml"]
