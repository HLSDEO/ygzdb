FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        wget unzip ca-certificates cmake make gcc g++ ninja-build python3 \
        libc6-dev libstdc++6 zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

ADD https://github.com/Lameguy64/mkpsxiso/releases/download/v2.30/mkpsxiso-2.30-Linux.deb /tmp/mkpsxiso.deb
RUN apt-get update \
    && apt-get install -y /tmp/mkpsxiso.deb \
    && rm -f /tmp/mkpsxiso.deb \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt \
    && cd /tmp \
    && wget -q https://github.com/Lameguy64/PSn00bSDK/releases/download/v0.24/gcc-mipsel-none-elf-12.3.0-linux.zip \
    && unzip -q gcc-mipsel-none-elf-12.3.0-linux.zip -d /opt \
    && rm gcc-mipsel-none-elf-12.3.0-linux.zip \
    && wget -q https://github.com/Lameguy64/PSn00bSDK/releases/download/v0.24/PSn00bSDK-0.24-Linux.zip \
    && unzip -q PSn00bSDK-0.24-Linux.zip -d /opt \
    && rm PSn00bSDK-0.24-Linux.zip

ENV PATH="/opt/mipsel-none-elf/bin:/opt/PSn00bSDK-0.24-Linux/mipsel-none-elf/bin:${PATH}"
ENV PSN00BSDK_LIBS="/opt/PSn00bSDK-0.24-Linux/lib/libpsn00b"

WORKDIR /work
