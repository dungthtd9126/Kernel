FROM ubuntu:14.04.1

RUN apt-get update \
        && apt install -y vim gcc make libncurses5-dev automake wget
        # && wget https://busybox.net/downloads/busybox-1.14.4.tar.bz2 \
        # && tar xf busybox-1.14.4.tar.bz2

WORKDIR /src
ENTRYPOINT ["bash", "-c"]
