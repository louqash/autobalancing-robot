################################################################################
#
# autobalancer_pid
#
################################################################################

AUTOBALANCER_PID_VERSION = 0.2
AUTOBALANCER_PID_SITE = $(BR2_EXTERNAL)/src/pid
AUTOBALANCER_PID_SITE_METHOD = local

$(eval $(cmake-package))
