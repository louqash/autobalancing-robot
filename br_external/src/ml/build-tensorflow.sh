#!/bin/bash

mkdir -p tensorflow_src/build
cd tensorflow_src/build

ARMCC_PREFIX=${HOME}/toolchains/arm-rpi-linux-gnueabihf/x64-gcc-6.5.0/arm-rpi-linux-gnueabihf/bin/arm-rpi-linux-gnueabihf-
ARMCC_FLAGS="-march=armv6 -mfpu=vfp -funsafe-math-optimizations"

if [[ ! -f "libtensorflow-lite.a" ]]; then
  cmake -DCMAKE_C_COMPILER=${ARMCC_PREFIX}gcc \
    -DCMAKE_CXX_COMPILER=${ARMCC_PREFIX}g++ \
    -DCMAKE_C_FLAGS="${ARMCC_FLAGS}" \
    -DCMAKE_CXX_FLAGS="${ARMCC_FLAGS}" \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DBUILD_ARMNN_TFLITE_DELEGATE=1 \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=armv6 \
    -DTFLITE_ENABLE_XNNPACK=OFF \
    ../tensorflow/lite/
  make
fi
