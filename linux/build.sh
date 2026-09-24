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

cd rootfs

find . | cpio -H newc -o | gzip > ../initramfs.gz

cd ..

qemu-system-i386 \
    -kernel bzImage \
    -initrd initramfs.gz \
    -append "console=ttyS0" \
    -device e1000,netdev=n1 \
    -netdev tap,id=n1,ifname=tap0,script=no,downscript=no \
    -nographic # \
    # -s -S

  #  -netdev user,id=n1,hostfwd=tcp::3636-:36 \

