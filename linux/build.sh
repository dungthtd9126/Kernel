#!/bin/sh
# print each command before executing it
set -e

IMAGE="bitinit/linux-2.6.26-env"

docker run --rm \
    -v "$PWD":/src \
    bitinit/linux-2.6.26-env \
    'cd /src/linux-2.6.26 &&
     make -C . M=/src/rootfs/home modules'

echo "success"

cd rootfs/home

./run
