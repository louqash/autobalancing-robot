################################################################################
#
# Motor Driver
#
################################################################################

TENSORFLOW_LITE_VERSION = v2.6.0
TENSORFLOW_LITE_SITE = https://github.com/tensorflow/tensorflow.git
TENSORFLOW_LITE_SUBDIR = tensorflow/lite
TENSORFLOW_LITE_SITE_METHOD = git
TENSORFLOW_LITE_INSTALL_TARGET = YES
TENSORFLOW_LITE_CONF_OPTS = -DCMAKE_C_FLAGS="-march=armv6 -mfpu=vfp -funsafe-math-optimizations" -DCMAKE_SYSTEM_PROCESSOR=armv6 -DCMAKE_SYSTEM_NAME=Linux -DTFLITE_ENABLE_XNNPACK=OFF
TENSORFLOW_LITE_SUPPORTS_IN_SOURCE_BUILD = NO

$(eval $(cmake-package))
