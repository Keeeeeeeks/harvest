# Ubuntu 10.04 (lucid) with the compiler that built the Linux 1.18 release:
# "GCC: (Ubuntu 4.4.3-4ubuntu5) 4.4.3".
#
# Docker Hub's ubuntu:10.04 is a schema 1 manifest that current Docker no longer pulls, so the
# rootfs is bootstrapped from old-releases. Only the release pocket is enabled, so apt cannot pull
# the later lucid-updates build (4.4.3-4ubuntu5.1).
ARG MIRROR=http://old-releases.ubuntu.com/ubuntu

FROM ubuntu:24.04 AS bootstrap
ARG MIRROR
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends debootstrap ubuntu-keyring \
 && rm -rf /var/lib/apt/lists/*
# Unpack the pinned release packages before configuring them in the Lucid rootfs.
RUN debootstrap --foreign --variant=minbase --arch=amd64 --include=gpgv \
      --keyring=/usr/share/keyrings/ubuntu-archive-removed-keys.gpg \
      lucid /rootfs "$MIRROR"

# Lucid's time functions can fault on the legacy vsyscall page under QEMU.
# Build a dependency-free syscall bridge with the bootstrap compiler, use it only
# during package installation, and discard it from the final compiler environment.
FROM bootstrap AS configured
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends gcc \
 && rm -rf /var/lib/apt/lists/*
COPY legacy-time.c /tmp/legacy-time.c
RUN gcc -shared -fPIC -nostdlib -o /rootfs/legacy-time.so /tmp/legacy-time.c \
 && LD_PRELOAD=/legacy-time.so chroot /rootfs /debootstrap/debootstrap --second-stage

FROM scratch
ARG MIRROR
COPY --from=configured /rootfs /
COPY --from=configured /usr/share/keyrings/ubuntu-archive-removed-keys.gpg /etc/apt/trusted.gpg
RUN rm -rf /var/cache/apt/archives/*.deb
RUN LD_PRELOAD=/legacy-time.so \
      sh -c 'echo "deb $1 lucid main universe" > /etc/apt/sources.list \
 && apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      g++-4.4=4.4.3-4ubuntu5 \
      gcc-4.4=4.4.3-4ubuntu5 \
      cpp-4.4=4.4.3-4ubuntu5 \
      libstdc++6-4.4-dev=4.4.3-4ubuntu5 \
      g++-4.4-multilib=4.4.3-4ubuntu5 \
      binutils=2.20.1-3ubuntu5 \
      libc6-dev=2.11.1-0ubuntu7 \
      make \
      libjpeg62-dev \
      zlib1g-dev \
      libogg-dev \
      libvorbis-dev \
      libopenal-dev \
      libalut-dev \
      libgtk2.0-dev \
      libgl1-mesa-dev \
      libglu1-mesa-dev \
      libx11-dev \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/* \
 && ln -s /usr/bin/gcc-4.4 /usr/local/bin/gcc \
 && ln -s /usr/bin/g++-4.4 /usr/local/bin/g++' sh "$MIRROR" \
 && rm /legacy-time.so

ENV PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
WORKDIR /work
