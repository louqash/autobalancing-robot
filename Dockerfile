FROM ubuntu:21.04

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
    curl git make gcc g++ file wget cpio unzip rsync bc bzip2 libncurses-dev

WORKDIR /workdir

# downloading toolchain for tensorflow lite according to:
# https://www.tensorflow.org/lite/guide/build_cmake_arm#build_for_raspberry_pi_zero_armv6
ARG TOOLCHAIN_VERSION=eb68350c5c8ec1663b7fe52c742ac4271e3217c5
RUN curl -L https://github.com/rvagg/rpi-newer-crosstools/archive/$TOOLCHAIN_VERSION.tar.gz -o rpi-toolchain.tar.gz
RUN mkdir /opt/toolchains && \
    tar xzf rpi-toolchain.tar.gz -C /opt/toolchains && \
    rm rpi-toolchain.tar.gz
RUN mv /opt/toolchains/rpi-newer-crosstools-$TOOLCHAIN_VERSION /opt/toolchains/arm-rpi-linux-gnueabihf

COPY docker_makefile Makefile

RUN git clone https://github.com/buildroot/buildroot && cd buildroot && git checkout 0f42d06ecf350aaa2d20bf716bf549d55b95982e
RUN ln -sf ../artifacts buildroot/output
