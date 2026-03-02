# The image is also used for libxslt, that's why we need git and
# libgcrypt-dev.

FROM ubuntu:22.10
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
        curl git ca-certificates \
        autoconf automake libtool pkg-config \
        make gcc clang llvm \
        zlib1g-dev liblzma-dev libgcrypt-dev \
        python2-dev python3-dev \
        cmake
WORKDIR /tests
RUN curl https://www.w3.org/XML/Test/xmlts20080827.tar.gz |tar xz
