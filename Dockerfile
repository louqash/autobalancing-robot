FROM ubuntu:21.04

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
  curl git

# downloading toolchain for tensorflow lite according to:
# https://www.tensorflow.org/lite/guide/build_cmake_arm#build_for_raspberry_pi_zero_armv6
ARG TOOLCHAIN_VERSION=eb68350c5c8ec1663b7fe52c742ac4271e3217c5
RUN curl -L https://github.com/rvagg/rpi-newer-crosstools/archive/$TOOLCHAIN_VERSION.tar.gz -o rpi-toolchain.tar.gz
RUN mkdir /opt/toolchains && tar xzf rpi-toolchain.tar.gz -C /opt/toolchains
RUN mv /opt/toolchains/rpi-newer-crosstools-$TOOLCHAIN_VERSION /opt/toolchains/arm-rpi-linux-gnueabihf
