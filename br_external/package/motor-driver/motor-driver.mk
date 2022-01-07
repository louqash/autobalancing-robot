################################################################################
#
# Motor Driver
#
################################################################################

MOTOR_DRIVER_VERSION = 1.0
MOTOR_DRIVER_SITE = $(BR2_EXTERNAL)/src/motor-driver
MOTOR_DRIVER_SITE_METHOD = local
$(eval $(kernel-module))
$(eval $(generic-package))
