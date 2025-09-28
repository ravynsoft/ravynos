FROM docker.io/library/ubuntu:devel

RUN DEBIAN_FRONTEND=noninteractive TZ=America/Denver apt-get update && \
    DEBIAN_FRONTEND=noninteractive TZ=America/Denver apt-get install -y \
        build-essential curl dpkg-dev ed fakeroot file git libapparmor-dev \
	libaudit-dev libkrb5-dev libldap2-dev libpam0g-dev libpython3-dev \
	libsasl2-dev libselinux1-dev libsepol-dev libssl-dev libwolfssl-dev \
	lsb-release ncurses-term openssh-client pkg-config procps python3-dev \
	ssh zlib1g-dev
RUN useradd -ms /bin/bash build
