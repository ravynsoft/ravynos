FROM docker.io/library/fedora:latest
ENV TZ=America/Denver

RUN dnf -y install audit-libs-devel cyrus-sasl-devel glibc-devel krb5-devel \
    libasan libubsan libselinux-devel libsepol-devel make openldap-devel \
    openssl-devel pam-devel python3-devel rpm-build zlib-devel binutils \
    ed gcc gdb git openssh pkg-config procps which
RUN useradd -ms /bin/bash build
